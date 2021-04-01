# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

"""
Helper functions for retrieving, validating and logging arguments from a Lambda event object.
"""

from distutils.util import strtobool
import json
import logging
import os
from typing import Optional, List, Dict
import re

from gamekithelpers.types import JsonObject


logger = logging.getLogger()
logger.setLevel(logging.INFO)


def log_event(event: dict) -> None:
    """
    Log details from the event for security and debugging purposes.
    """
    # Do not log if DETAILED_LOGGING_DISABLED setting is turned ON
    if bool(strtobool(os.environ.get('DETAILED_LOGGING_DISABLED', 'false'))):
        return

    log_items = {
        'resource': f"{event.get('httpMethod')} {event.get('resource')}",
        'source_ip': None,
        'has_amzn_security_token': False,
        'did_try_cognito_auth': False,
        'cognito_auth_success': False,
        'gk_user_id': None,
        'iam_caller': None
    }

    if event.get('headers') and event.get('headers').get('X-Forwarded-For'):
        log_items['source_ip'] = event.get('headers').get('X-Forwarded-For')

    if event.get('headers') and event.get('headers').get('x-amz-security-token') and len(str(event.get('headers').get('x-amz-security-token'))) > 0:
        log_items['has_amzn_security_token'] = True

    request_context = event.get('requestContext')
    if request_context and request_context.get('authorizer'):
        log_items['did_try_cognito_auth'] = True
        if request_context.get('authorizer').get('claims').get('custom:gk_user_id'):
            log_items['cognito_auth_success'] = True
            log_items['gk_user_id'] = request_context.get('authorizer').get('claims').get('custom:gk_user_id')
    if request_context and request_context.get('identity') and request_context.get('identity').get('caller'):
        log_items['iam_caller'] = request_context.get('identity').get('caller')

    log_message = (
        "Event Access Log:: "
        f"Resource:: {log_items['resource']} ; "
        f"Source IP:: {log_items['source_ip']} ; "
        f"IAM Auth Identifiers:: IAM auth used: {log_items['has_amzn_security_token']} ; IAM Caller: {log_items['iam_caller']} ; "
        f"Cognito Auth Identifiers:: Cognito Auth Attempted: {log_items['did_try_cognito_auth']} ; Cognito Auth Success: {log_items['cognito_auth_success']} ; Opaque Gamekit User ID: {log_items['gk_user_id']}"
    )
    logger.info(log_message)


def get_player_id(event: dict) -> Optional[str]:
    """
    Get the gk_user_id from the event's requestContext.
    """
    player_id = None
    try:
        player_id = event['requestContext']['authorizer']['claims'].get('custom:gk_user_id')
    except (KeyError, TypeError):
        logger.error('Did not find custom:gk_user_id in event.')

    return player_id


def get_path_param(event: dict, param_name: str) -> Optional[str]:
    """
    Get a path parameter from the event.
    """
    value = None
    try:
        value = event['pathParameters'][param_name]
    except (KeyError, TypeError):
        logger.error(f'Did not find path parameter: {param_name}')

    return value


def get_request_context_param(event: dict, param_name: str) -> Optional[str]:
    """
    Get a request_context parameter from the event.
    """
    value = None
    try:
        value = event['requestContext'][param_name]
    except (KeyError, TypeError):
        logger.error(f'Did not find context parameter: {param_name}')

    return value


def get_query_string_param(event: dict, param_name: str, default: str = None) -> Optional[str]:
    """
    Get a query string parameter from the event.
    """
    value = default
    try:
        value = event['queryStringParameters'][param_name]
    except (KeyError, TypeError):
        logger.warning(f'Did not find query string parameter: {param_name}')

    return value


def get_query_string_param_as_list(event: dict, param_name: str, delimiter: str = ',') -> List[str]:
    """
    Retrieve a query string parameter from the event and convert it into a list.
    """
    value = []
    try:
        value = event['queryStringParameters'][param_name].split(delimiter)
    except (KeyError, TypeError):
        logger.warning(f'Did not find query string parameter: {param_name}')

    return value


def get_header_param(event: dict, param_name: str, default: str = None) -> Optional[str]:
    """
    Get a header parameter from the event.
    """
    value = default
    try:
        value = event['headers'][param_name]
    except (KeyError, TypeError):
        logger.warning(f'Did not find header parameter: {param_name}')

    return value


def get_cognito_user_attributes_from_request(event: dict) -> Dict[str, str]:
    """
    Retrieve Cognito userAttributes from request
    """
    value = {}
    try:
        return event['request']['userAttributes']
    except (KeyError, TypeError):
        logger.error('Did not find userAttributes in event. Invalid Cognito request.')

    return value


def get_body_as_json(event: dict) -> JsonObject:
    """
    Retrieve the body of the Event as a JSON object.
    """
    body_obj = None
    try:
        body = event['body']
        body_obj = json.loads(body)
    except (KeyError, TypeError, json.JSONDecodeError):
        logger.exception('Did not find event body, or could not load event body from JSON.')

    return body_obj


def is_valid_uuidv4(uuid) -> bool:
    # Regex checker for UUIDv4
    uuid4hex = re.compile('[a-f0-9]{8}-?[a-f0-9]{4}-?4[a-f0-9]{3}-?[89ab][a-f0-9]{3}-?[a-f0-9]{12}', re.I)
    return bool(uuid4hex.match(uuid))
