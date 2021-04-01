# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

"""
Purpose

This is the lambda function invoked by the callback URL configured in Facebook.
In addition to issuing an OAuth token, this function will also insert a record in
the identities table with a unique ID and hash key for the user along with the
user's Facebook userid.
"""

from base64 import b64encode
import botocore
import boto3
from boto3.dynamodb.conditions import Key
from datetime import timezone, datetime
from gamekithelpers import crypto, handler_request, handler_response, ddb
from hashlib import sha256
import os
import json
from urllib.parse import urlencode
import urllib.request
import uuid

s3_client = boto3.client('s3')
cidp_client = boto3.client('cognito-idp')
dynamodb_client = boto3.client('dynamodb')


def _update_item_params(gk_user_id, computed_gk_user_id_hash, user_attributes):
    params = {
        'Key': {
            'gk_user_id': gk_user_id
        },
        'ReturnValues': 'ALL_NEW',
        'ConditionExpression': Key('gk_user_id_hash').eq(computed_gk_user_id_hash),
        'ExpressionAttributeNames': {
            '#facebook_ref_id': 'facebook_ref_id',
            '#facebook_external_id': 'facebook_external_id',
            '#updated_at': 'updated_at'
        },
        'ExpressionAttributeValues': {
            ':facebook_ref_id': user_attributes.get('sub'),
            ':facebook_external_id': user_attributes.get('user_id'),
            ':updated_at': ddb.timestamp()
        },
        'UpdateExpression': 'SET #facebook_ref_id = :facebook_ref_id, #facebook_external_id = :facebook_external_id, '
                            '#updated_at = :updated_at '
    }

    return params


def _callback_completion_path(request_id):
    cb_completion_path = 'cb_completions/' + request_id
    return cb_completion_path


def _write_object_to_s3(bucket, key, contents):
    s3_client.put_object(Bucket=bucket, Key=key, Body=contents)


def _check_object_exists_in_s3(bucket, key):
    try:
        s3_client.head_object(Bucket=bucket, Key=key)
        return True
    except botocore.exceptions.ClientError:
        return False


def _get_user_identy_id(userinfo_dict):
    """
    Get user identity from Cognito
    """
    try:
        user_info = cidp_client.admin_get_user(UserPoolId=os.environ['USER_POOL_ID'], Username=userinfo_dict['username'])
    except botocore.exceptions.ClientError as err:
        print(f"Unable to lookup user. Error: {err}")
        raise err
    for attr in user_info.get('UserAttributes'):
        if attr['Name'] == 'identities':
            identities = json.loads(attr['Value'])
            break
    user_attributes = {
        'sub': userinfo_dict['sub'],
        'user_id': identities[0]['userId']
    }

    return user_attributes


def _get_user_info(access_token):
    req = urllib.request.Request(os.environ['USER_POOL_DOMAIN'] + '/oauth2/userInfo')
    req.add_header('Authorization', 'Bearer ' + access_token)
    with urllib.request.urlopen(req) as response:
        userinfo_resp = response.read().decode('utf-8')
        userinfo_dict = json.loads(userinfo_resp)
    return userinfo_dict


def _exchange_code_for_access_token(code, request_source_ip):
    data = urlencode({
        'grant_type': 'authorization_code',
        'client_id': os.environ['APP_CLIENT_ID'],
        'redirect_uri': os.environ['REDIRECT_URI'],
        'code': code
    })
    data = data.encode('utf-8')
    req = urllib.request.Request(os.environ['USER_POOL_DOMAIN'] + '/oauth2/token', data)
    with urllib.request.urlopen(req) as response:
        token_resp_dict = json.loads(response.read().decode('utf-8'))
        token_resp_dict['source_ip'] = request_source_ip
        token_resp = json.dumps(token_resp_dict)
    return token_resp, token_resp_dict


def _success_html():
    return '<head><link type="text/css" rel="stylesheet" ' \
           'href="https://static.xx.fbcdn.net/rsrc.php/v3/yo/l/0,' \
           'cross/ksqWwthNfybqQTEbWveJZE.css?_nc_x=2vxG7aV-Q4T"><title>Facebook</title><body><div ' \
           'style="margin: auto;width: 50%;border: 3px solid gray;padding: 10px;"><div style="margin: ' \
           'auto;width: 50%;"><img src="https://static.xx.fbcdn.net/rsrc.php/v3/yg/r/dHwG4qwIHQS.png" ' \
           'width="80" height="64"><div><p><b>Success!</b></p></div><div><div><p>You are now ' \
           'logged in. You can close this browser window and go back to the ' \
           'game.</p></div></div></div></div></body></head> '


def lambda_handler(event, context):
    """
    This is the lambda function handler.
    """
    handler_request.log_event(event)

    code = handler_request.get_query_string_param(event, 'code')
    state = handler_request.get_query_string_param(event, 'state')

    # Exit early if state is None, an empty string, or whitespace:
    if not state or state.isspace():
        return handler_response.return_response(400, json.dumps({'error': 'Missing state'}))

    state = state.replace(" ", "+")
    is_account_linking_request = False

    # Get source IP from request context
    request_context = event.get('requestContext')
    request_source_ip = request_context.get('identity').get('sourceIp')

    # Immediately, obtain access token from Cognito's oauth2/token endpoint
    # so the code doesn't get re-used.
    try:
        token_resp, token_resp_dict = _exchange_code_for_access_token(code, request_source_ip)

    except urllib.error.HTTPError as e:
        raise Exception(f"Error on oauth2/token {e.code}: {e.read()}")

    # get gamekit cmk
    cmk_id, cmk_arn = crypto.get_gamekit_key()

    # Decrypt state parameter
    _, decrypted_state = crypto.decrypt_blob(cmk_id, state)

    # This parameter is passed in the redirect_uri.
    # It is a base64 encoded value of JSON with keys:
    # gk_user_id, gk_user_id_hash, request_id, and expiration.
    decoded_state = json.loads(decrypted_state.decode('utf-8'))
    request_id = decoded_state.get('request_id')
    if request_id is None or not handler_request.is_valid_uuidv4(request_id):
        return handler_response.return_response(400, json.dumps({'error': 'Invalid state'}))

    request_expiration = decoded_state.get('expiration')
    if request_expiration is None:
        return handler_response.return_response(400, json.dumps({'error': 'Invalid state'}))

    # Ensure that the IP address that generated the URL matches the IP address where
    # this login flow is being completed
    source_ip = decoded_state.get('source_ip')
    if source_ip is None or source_ip != request_source_ip:
        return handler_response.return_response(400, json.dumps({'error': 'Invalid state'}))

    current_time = int(datetime.now(tz=timezone.utc).timestamp())
    if current_time > request_expiration:
        return handler_response.return_response(403, json.dumps({'error': 'Request expired'}))

    if _check_object_exists_in_s3(os.environ.get('BOOTSTRAP_BUCKET'), _callback_completion_path(request_id)):
        # The completion callback file already exists. Skip processing.
        return handler_response.return_response(200, _success_html(), 'text/html')

    ddb_table = ddb.get_table(os.environ.get('IDENTITY_TABLE_NAME'))
    if decoded_state.get('gk_user_id') is not None and decoded_state.get('gk_user_id_hash') is not None:
        # If the state parameters 'gk_user_id' and 'gk_user_id_hash' are present, the FB account is being linked to
        # an existing user.
        is_account_linking_request = True
        gk_user_id = decoded_state.get('gk_user_id')
        gk_user_id_hash = decoded_state.get('gk_user_id_hash')

        # Lookup user in identities table using gk_user_id
        try:
            response = ddb_table.get_item(**ddb.get_item_request_param({'gk_user_id': gk_user_id}, True))
        except botocore.exceptions.ClientError as err:
            print(f"Error getting item: {gk_user_id}. Error: {err}")
            raise err

        user_identity = ddb.get_response_item(response)
        if user_identity is None:
            # The passed gk_user_id doesn't exist. Do not proceed with issuing a token.
            raise Exception(f"Invalid user: {gk_user_id}")
        else:
            # Get gk_user_hash_key from the record and compute the hash.
            # This is a SHA256 hash (salted with gk_user_hash_key) of gk_user_id to
            # be compared with what's passed in gk_user_id_hash.

            gk_user_hash_key = user_identity['gk_user_hash_key']
            id_hash = sha256((gk_user_hash_key + gk_user_id).encode('utf-8'))
            computed_gk_user_id_hash = b64encode(id_hash.digest()).decode('utf-8')

            if gk_user_id_hash != computed_gk_user_id_hash:
                raise Exception(f"Hash mismatch. Invalid user: {gk_user_id}")
    else:
        # # If the state parameters 'gk_user_id' and 'gk_user_id_hash' are NOT present, which means this is not an
        # account linking request. If the facebook_external_id doesn't exist, this function will create a new
        # gk_user_id and gk_user_hash_key for the user.
        gk_user_id = str(uuid.uuid4())
        key = str(uuid.uuid4())
        key_hash = sha256(key.encode('utf-8'))
        gk_user_hash_key = b64encode(key_hash.digest()).decode('utf-8')
        id_hash = sha256((gk_user_hash_key + gk_user_id).encode('utf-8'))
        computed_gk_user_id_hash = b64encode(id_hash.digest()).decode('utf-8')

    # Obtain user identifier from Cognito's oauth2/userInfo endpoint using the issued access_token.
    try:
        userinfo_dict = _get_user_info(token_resp_dict['access_token'])

    except urllib.error.HTTPError as e:
        raise Exception(f"Error on oauth2/userInfo {e.code}: {e.read()}")

    # Get the user identity_id for the username
    user_attributes = _get_user_identy_id(userinfo_dict)

    if is_account_linking_request:
        # Update the record for gk_user_id hashes match if the hashes match
        try:
            params = _update_item_params(gk_user_id, computed_gk_user_id_hash, user_attributes)
            response = ddb_table.update_item(**params)
        except dynamodb_client.exceptions.ConditionalCheckFailedException as err:
            print(f"Hash mismatch. Error: {err}.")
            raise err

        return handler_response.return_response(200, _success_html(), 'text/html')

    else:
        # Check if facebook_external_id is in GSI gidx_facebook_external_id
        try:
            response = ddb_table.query(**ddb.query_request_param(
                'facebook_external_id',
                user_attributes.get('user_id'),
                'gidx_facebook_external_id'))
        except botocore.exceptions.ClientError as err:
            print(f"Error querying gidx_facebook_external_id for: {user_attributes.get('user_id')}. Error: {err}.")
            raise err

        if response['Count'] == 0:
            # Insert new record for the Facebook user
            try:
                now = ddb.timestamp()
                ddb_table.put_item(**ddb.put_item_request_param({
                    'gk_user_id': gk_user_id,
                    'gk_user_hash_key': gk_user_hash_key,
                    'gk_user_id_hash': computed_gk_user_id_hash,
                    'facebook_ref_id': user_attributes.get('sub'),
                    'facebook_external_id': user_attributes.get('user_id'),
                    'created_at': now,
                    'updated_at': now
                }))
            except botocore.exceptions.ClientError as err:
                print(f"Error inserting item for: {gk_user_id}. Error: {err}")
                raise err

        # get gamekit cmk
        cmk_id, cmk_arn = crypto.get_gamekit_key()

        # encrypt tokens
        encryption_success, encrypted_blob = crypto.encrypt_text(cmk_id, token_resp)

        # write encrypted tokens to callback token path including the IP address
        # where the login flow was completed
        cb_token_path = 'cb_tokens/' + str(uuid.uuid4())
        completion_data = json.dumps({
            'token_path':  cb_token_path,
            'source_ip': request_source_ip
        })
        if encryption_success:
            _write_object_to_s3(os.environ.get('BOOTSTRAP_BUCKET'), cb_token_path, b64encode(encrypted_blob))

        # encrypt location of callback token path
        encryption_success, encrypted_blob = crypto.encrypt_text(cmk_id, completion_data)

        # write encrypted location to S3 completion object
        if encryption_success:
            _write_object_to_s3(os.environ.get('BOOTSTRAP_BUCKET'), _callback_completion_path(request_id), b64encode(encrypted_blob))

        return handler_response.return_response(200, _success_html(), 'text/html')
