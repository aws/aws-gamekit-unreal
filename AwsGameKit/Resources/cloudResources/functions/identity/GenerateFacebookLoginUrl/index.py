# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

"""
Purpose

This function generates the Facebook login url.
"""

from base64 import b64encode
from datetime import datetime, timezone, timedelta
from gamekithelpers import crypto, handler_request, handler_response
import os
import json


def lambda_handler(event, context):
    """
    This is the lambda function handler.
    """
    handler_request.log_event(event)

    login_url = os.environ.get('COGNITO_BASE_URL') + \
                "/authorize?response_type=code&client_id=" + \
                os.environ.get('APP_CLIENT_ID') + \
                "&redirect_uri=" + \
                os.environ.get('REDIRECT_URI') + \
                "&scope=openid+com.aws.gamekit/gamekit.identity&identity_provider=Facebook&state="

    body = handler_request.get_body_as_json(event)
    request_id = body.get('request_id')

    # Check if request_id is a valid UUIDv4
    if request_id is None or not handler_request.is_valid_uuidv4(request_id):
        return handler_response.invalid_request()

    # Get source IP from request context
    source_ip = handler_request.get_request_context_param(event, 'identity').get('sourceIp')

    # get gamekit cmk
    cmk_id, cmk_arn = crypto.get_gamekit_key()

    expiration = int((datetime.now(tz=timezone.utc) + timedelta(minutes=1)).timestamp())
    state = json.dumps({
        'request_id': request_id,
        'expiration': expiration,
        'source_ip': source_ip
    })
    encryption_success, encrypted_blob = crypto.encrypt_text(cmk_id, state)
    if encryption_success:
        login_url += b64encode(encrypted_blob).decode('utf-8')

    return handler_response.return_response(200, login_url)
