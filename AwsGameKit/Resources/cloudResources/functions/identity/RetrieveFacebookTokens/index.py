# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

"""
Purpose

This lambda function retrieves the Facebook tokens.
"""

import boto3
from gamekithelpers import crypto, handler_request, handler_response
import json
import os

s3_client = boto3.client('s3')


def lambda_handler(event, context):
    """
    This is the lambda function handler.
    """
    handler_request.log_event(event)
    body = event.get('body')
    if body is None:
        return handler_response.invalid_request()

    # Get source IP from request context
    request_source_ip = handler_request.get_request_context_param(event, 'identity').get('sourceIp')

    # get gamekit cmk
    cmk_id, cmk_arn = crypto.get_gamekit_key()

    # Decrypt body which contains the token location
    _, decrypted_body = crypto.decrypt_blob(cmk_id, body)
    token_location_data = json.loads(decrypted_body.decode('utf-8'))

    # Ensure that only the IP addresses that completed the login flow can retrieve the tokens
    source_ip = token_location_data.get('source_ip')
    if source_ip is None or source_ip != request_source_ip:
        return handler_response.invalid_request()

    # Get encrypted tokens
    token_path = token_location_data['token_path']
    response = s3_client.get_object(Bucket=os.environ.get('BOOTSTRAP_BUCKET'), Key=token_path)
    encrypted_tokens = response['Body'].read().decode('utf-8')
    if encrypted_tokens == 'Retrieved':
        return handler_response.return_response(403, encrypted_tokens)

    # Decrypt tokens
    _, decrypted_tokens = crypto.decrypt_blob(cmk_id, encrypted_tokens)
    decrypted_token_data = json.loads(decrypted_tokens)

    # Ensure that only the IP addresses that completed the login flow can retrieve the tokens
    source_ip = decrypted_token_data.get('source_ip')
    if source_ip is None or source_ip != request_source_ip:
        return handler_response.invalid_request()

    # Rewrite token object to indicate it's been retrieved.
    # - Any subsequent requests to get it will return an invalid payload.
    # - token object will be deleted based on bucket policy.
    s3_client.put_object(Bucket=os.environ.get('BOOTSTRAP_BUCKET'), Key=token_path, Body='Retrieved')

    return handler_response.return_response(200, decrypted_tokens.decode('utf-8'))
