# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

"""
Purpose

Retrieves all achievements.

This is a non-player facing Lambda function and used from the GameKit plugin.
"""

import distutils.core
import json
import os

import botocore
from gamekithelpers import handler_request, handler_response, ddb

ddb_table = ddb.get_table(os.environ['ACHIEVEMENTS_TABLE_NAME'])

def _get_achievements(response_limit, use_consistent_read, start_key):
    response = ddb_table.scan(**ddb.scan_request_param(response_limit, use_consistent_read, start_key))
    return ddb.get_response_items(response)


def lambda_handler(event, context):
    """
    This is the lambda function handler.
    """
    handler_request.log_event(event)

    start_key = handler_request.get_query_string_param(event, 'start_key')
    if start_key is not None and len(start_key) > 0:
        start_key = json.loads(start_key)
    wait_for_all_pages = bool(distutils.util.strtobool(handler_request.get_query_string_param(event, 'wait_for_all_pages', 'false')))
    use_consistent_read = bool(distutils.util.strtobool(handler_request.get_query_string_param(event, 'use_consistent_read', 'false')))
    limit = handler_request.get_query_string_param(event, 'limit', '100')

    response_limit = int(limit)
    if response_limit > 100 or response_limit <= 0:
        response_limit = 100

    try:
        all_achievements = []
        achievements, next_start_key = _get_achievements(response_limit, use_consistent_read, start_key)
        all_achievements.extend(achievements)
        while wait_for_all_pages and next_start_key:
            achievements, next_start_key = _get_achievements(response_limit, use_consistent_read, next_start_key)
            all_achievements.extend(achievements)

    except botocore.exceptions.ClientError as err:
        print(f"Error retrieving items. Error: {err}")
        raise err

    return handler_response.response_envelope(200, None, {'achievements': all_achievements}, next_start_key)
