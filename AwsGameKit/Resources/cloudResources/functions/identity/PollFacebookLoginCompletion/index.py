# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

"""
Purpose

This function polls the login completion object.
"""

import botocore
import boto3
from gamekithelpers import handler_request, handler_response
import os

s3_client = boto3.client('s3')


def lambda_handler(event, context):
    """
    This is the lambda function handler.
    """
    handler_request.log_event(event)
    body = handler_request.get_body_as_json(event)
    if body is None:
        return handler_response.invalid_request()

    request_id = body.get('request_id')
    if request_id is None or not handler_request.is_valid_uuidv4(request_id):
        return handler_response.invalid_request()

    try:
        # Get completion object
        response = s3_client.get_object(Bucket=os.environ.get('BOOTSTRAP_BUCKET'), Key='cb_completions/' + request_id)
        encrypted_key_location = response['Body'].read().decode('utf-8')

        # Rewrite completion object to indicate it's been retrieved.
        # - Any subsequent requests to get it will return an invalid payload.
        # - Any repeated requests to the Facebook callback handler will be ignored.
        # - Completion object will be deleted based on bucket policy.
        s3_client.put_object(Bucket=os.environ.get('BOOTSTRAP_BUCKET'), Key='cb_completions/' + request_id, Body='Retrieved')

        # Return encrypted location of tokens
        return handler_response.return_response(200, encrypted_key_location)
    except botocore.exceptions.ClientError as err:
        # completion object not found
        err_properties = err.__dict__['response']
        err_code = err_properties['ResponseMetadata']['HTTPStatusCode']
        return handler_response.return_response(err_code, err_properties['Error']['Message'])
