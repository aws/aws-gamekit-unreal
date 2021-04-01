# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

"""
!!!IMPORTANT!!!
This Lambda function should only be invoked by Cognito through the User Pool where this is configured
as the Post Confirmation trigger. The CloudFormation key `CognitoPostConfirmationLambdaPermission` sets this.

Purpose

This function is triggered by Amazon Cognito after a new user is confirmed.
See: https://docs.aws.amazon.com/cognito/latest/developerguide/user-pool-lambda-post-confirmation.html

For users who signed up with email/password, this function will update their record in the identities table
if the hash of their gk_user_id matches what's passed in the request. The updated record will contain the
ID issued by Cognito for cross reference.
"""

from base64 import b64encode
import boto3
from boto3.dynamodb.conditions import Key
from gamekithelpers import handler_request, ddb
from hashlib import sha256
import os

dynamodb_client = boto3.client('dynamodb')


def _update_item_params(event, gk_user_id, gk_user_id_hash, user_attributes):
    params = {
        'Key': {
            'gk_user_id': gk_user_id,
        },
        'ReturnValues': 'ALL_NEW',
        'ConditionExpression': Key('gk_user_id_hash').eq(gk_user_id_hash),
        'ExpressionAttributeNames': {
            '#user_name': 'user_name',
            '#email_ref_id': 'email_ref_id',
            '#updated_at': 'updated_at'
        },
        'ExpressionAttributeValues': {
            ':user_name': event.get('userName'),
            ':email_ref_id': user_attributes.get('sub'),
            ':updated_at': ddb.timestamp()
        },
        'UpdateExpression': 'SET #user_name = :user_name, #email_ref_id = :email_ref_id, #updated_at = :updated_at'
    }

    return params


def lambda_handler(event, context):
    """
    This is the lambda function handler.
    """

    user_attributes = handler_request.get_cognito_user_attributes_from_request(event)

    # If the request was from one of the Federated Identity Providers ('cognito:user_status' == 'EXTERNAL_PROVIDER'),
    # return immediately. The extenal provider-specific callback handler will process the request.
    if user_attributes.get('cognito:user_status') == 'EXTERNAL_PROVIDER':
        return event

    ddb_table = ddb.get_table(os.environ.get('IDENTITY_TABLE_NAME'))

    # If the user signed up with email and password, custom:gk_user_id and custom:gk_user_hash_key
    # is created during sign_up. These are stored as custom attributes in Cognito. This handler will receive it in
    # event.request.userAttributes.
    gk_user_id = user_attributes.get('custom:gk_user_id')
    gk_user_hash_key = user_attributes.get('custom:gk_user_hash_key')

    # SHA256 hash (salted with gk_user_hash_key) of gk_user_id to store in gk_user_id_hash.
    id_hash = sha256((gk_user_hash_key + gk_user_id).encode('utf-8'))
    computed_gk_user_id_hash = b64encode(id_hash.digest()).decode('utf-8')

    # Update the record for gk_user_id if the hashes match
    try:
        params = _update_item_params(event, gk_user_id, computed_gk_user_id_hash, user_attributes)
        ddb_table.update_item(**params)
    except dynamodb_client.exceptions.ConditionalCheckFailedException as err:
        print(f"Hash mismatch. Error: {err}.")
        raise err

    return event
