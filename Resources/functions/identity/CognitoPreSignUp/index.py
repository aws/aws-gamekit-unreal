# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

"""
!!!IMPORTANT!!!
This Lambda function should only be invoked by Cognito through the User Pool where this is configured
as the Pre-Sign-Up trigger. The CloudFormation key `CognitoPreSignUpLambdaPermission` sets this.

Purpose

This function is triggered just before Amazon Cognito signs up a new user.
See: https://docs.aws.amazon.com/cognito/latest/developerguide/user-pool-lambda-pre-sign-up.html

For users signing up with email and password, this function will create a record in the identities table with
a unique ID and hash key for the user. Also, this function checks if email already exists since the Cognito UserPool
is not configured to have the email address as the primary key.
"""

from base64 import b64encode
import botocore
import boto3
from gamekithelpers import handler_request, ddb
from hashlib import sha256
import os

cidp_client = boto3.client('cognito-idp')

def lambda_handler(event, context):
    """
    This is the lambda function handler.
    """

    # If the request was from one of the Federated Identity Providers (triggerSource == 'PreSignUp_ExternalProvider'),
    # return immediately and let the PostConfirmation lambda trigger process the request
    if event.get('triggerSource') == 'PreSignUp_ExternalProvider':
        return event

    user_attributes = handler_request.get_cognito_user_attributes_from_request(event)

    # custom:gk_user_id and custom:gk_user_hash_key are created by GameKit and passed during sign_up.
    # They are stored as custom attributes in Cognito. This handler will receive it in
    # event.request.userAttributes and custom:gk_user_id needs to be a valid UUIDv4 string.
    gk_user_id = user_attributes.get('custom:gk_user_id')
    gk_user_hash_key = user_attributes.get('custom:gk_user_hash_key')

    # Email from request
    email = user_attributes.get('email')

    # SHA256 hash (salted with gk_user_hash_key) of gk_user_id to store in gk_user_id_hash.
    id_hash = sha256((gk_user_hash_key + gk_user_id).encode('utf-8'))
    computed_gk_user_id_hash = b64encode(id_hash.digest()).decode('utf-8')

    # Do not auto-confirm the user
    event['response']['autoConfirmUser'] = False

    # Check if gk_user_id from userAttributes is a valid UUIDv4
    if not handler_request.is_valid_uuidv4(gk_user_id):
        event['response']['autoVerifyEmail'] = False
        raise ValueError('invalid gk_user_id, it must be a valid UUIDv4')

    # Cognito UserPool is not configured to have the email address as the primary key.
    # We need to check if email already exists in Cognito
    user_list = cidp_client.list_users(UserPoolId=os.environ.get('USER_POOL_ID'),
                                Limit=1,
                                AttributesToGet=['email'],
                                Filter='email="%s"' % email)

    if len(user_list.get('Users')) > 0:
        event['response']['autoVerifyEmail'] = False
        raise ValueError('%s already exists' % email)

    # Lookup user in identities table using gk_user_id
    ddb_table = ddb.get_table(os.environ.get('IDENTITY_TABLE_NAME'))
    try:
        response = ddb_table.get_item(**ddb.get_item_request_param({'gk_user_id': gk_user_id}))
    except botocore.exceptions.ClientError as err:
        print(f"Error getting item: {gk_user_id}. Error: {err}")
        raise err

    # If there is an existing user for gk_user_id, do nothing and let
    # the PostConfirmation lambda trigger process the request
    if response.get('Item') is not None:
        return event

    # Parameters are valid. Create a record in DDB.
    try:
        now = ddb.timestamp()
        ddb_table.put_item(**ddb.put_item_request_param({
            'gk_user_id': gk_user_id,
            'gk_user_hash_key': gk_user_hash_key,
            'gk_user_id_hash': computed_gk_user_id_hash,
            'created_at': now,
            'updated_at': now
        }))
    except botocore.exceptions.ClientError as err:
        print(f"Error inserting item for: {gk_user_id}. Error: {err}.")
        event['response']['autoVerifyEmail'] = False
        raise err

    return event
