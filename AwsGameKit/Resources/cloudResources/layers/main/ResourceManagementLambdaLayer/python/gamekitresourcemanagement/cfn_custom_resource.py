# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

"""
Helper functions for CloudFormation custom resources.
"""

import hashlib
import json
import logging
import re
import urllib.request
from enum import Enum


class ResponseStatus(str, Enum):
    SUCCESS = 'SUCCESS'
    FAILED = 'FAILED'


class RequestType(str, Enum):
    CREATE = 'Create'
    DELETE = 'Delete'
    UPDATE = 'Update'


logger = logging.getLogger()
logger.setLevel(logging.INFO)

MAX_CUSTOM_RESOURCE_TYPE_CHARS = 32
MIN_CUSTOM_RESOURCE_TYPE_CHARS = 1
CUSTOM_RESOURCE_TYPE_REGEX = re.compile('^([a-zA-Z0-9-_]+)$')


def send_success_response(event, custom_resource_type: str):
    """
    Send a success response to CloudFormation for the given custom resource.

    PhysicalResourceId is derived from the custom_resource_type. Must be 32 characters or less.
    """
    _send_response(event, custom_resource_type, ResponseStatus.SUCCESS, '')


def send_failure_response(event, custom_resource_type: str, reason: str):
    """
    Send a failure response to CloudFormation for the given custom resource and reason.

    PhysicalResourceId is derived from the custom_resource_type. Must be 32 characters or less.
    """
    _send_response(event, custom_resource_type, ResponseStatus.FAILED, reason)


def _validate_custom_resource_type(custom_resource_type: str):
    if len(custom_resource_type) < MIN_CUSTOM_RESOURCE_TYPE_CHARS or len(
            custom_resource_type) > MAX_CUSTOM_RESOURCE_TYPE_CHARS:
        raise ValueError(
            f'Custom resource type must be between {MIN_CUSTOM_RESOURCE_TYPE_CHARS} and {MAX_CUSTOM_RESOURCE_TYPE_CHARS} characters long')
    if CUSTOM_RESOURCE_TYPE_REGEX.fullmatch(custom_resource_type) is None:
        raise ValueError(
            f'Custom resource type may only include alphanumeric characters, numbers, dashes, and underscores')


def _get_physical_resource_id(stack_id: str, custom_resource_type: str, logical_resource_id: str) -> str:
    """
    Returns a PhysicalResourceId that uniquely identifies the custom CloudFormation resource.

    PhysicalResourceId must be stable - we'll base it on the StackId and LogicalResourceId of the resource.
    As PhysicalResourceId has a limit of 64 characters, we'll take the MD5 hash of these to avoid surpassing the
    resource limit while retaining a unique id per resource.
    """
    _validate_custom_resource_type(custom_resource_type)
    # Hex encoded MD5 hash is 32 characters
    physical_resource_id_hash = hashlib.md5(f'{stack_id}-{logical_resource_id}'.encode()).hexdigest()
    return f'{custom_resource_type}-{physical_resource_id_hash}'


def _send_response(event, custom_resource_type: str, response_status: ResponseStatus, reason: str):
    physical_resource_id = _get_physical_resource_id(event['StackId'], custom_resource_type, event['LogicalResourceId'])
    response_body = {
        'Status': response_status,
        'Reason': reason,
        'PhysicalResourceId': physical_resource_id,
        'StackId': event['StackId'],
        'RequestId': event['RequestId'],
        'LogicalResourceId': event['LogicalResourceId'],
        'Data': json.loads('{}')
    }

    request = urllib.request.Request(event['ResponseURL'],
                                     method='PUT',
                                     data=json.dumps(response_body).encode('utf-8'),
                                     headers={'Content-Type': '""'})
    # If there is an issue when responding to CloudFormation (malformed content, etc.),
    # we can pull this request from the logs and manually send a success response.
    logger.info(f'Sending {response_status} notification request to CloudFormation at {event["ResponseURL"]}')

    with urllib.request.urlopen(request) as response:
        logger.info(f'Received {response.getcode()} response from CloudFormation: {response.reason}')
