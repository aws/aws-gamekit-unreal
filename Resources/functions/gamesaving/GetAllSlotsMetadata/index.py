# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

from distutils.util import strtobool
import os
from typing import Dict, Optional

from gamekithelpers import ddb
from gamekithelpers.handler_request import get_player_id, get_query_string_param, log_event
from gamekithelpers.handler_response import response_envelope
from gamekithelpers.pagination import validate_pagination_token
from gamekithelpers.sanitizer import sanitize


DEFAULT_PAGE_SIZE = '100'
DEFAULT_START_KEY = ''
DEFAULT_CONSISTENT_READ = 'True'
EMPTY_RESPONSE = {}


def lambda_handler(event, context):
    """
    Retrieve metadata for all of the player's save slots, or an empty list if no metadata is found.

    Returns a maximum of "page_size" save slots or 1 MB of data, whichever is reached first. If the Lambda response body
    includes "next_start_key", then the player has more save slots than could be returned from a single invocation of
    this Lambda function. To get the next set of save slots, call the Lambda function again, this time including the
    query string parameter "start_key" with the value of "next_start_key".

    Parameters:

    Request Context:
        custom:gk_user_id: str
            The player_id to fetch the save slot metadata for. This value comes from the Cognito Authorizer that
            validates the API Gateway request.

    Query String Parameters:
        page_size: int
            The maximum number of save slots to return metadata for.
            [Optional, defaults to 100 (DEFAULT_PAGE_SIZE).]

        start_key: str
            The value "slot_name" from the "next_start_key" dictionary that was returned in the previous invocation of
            this Lambda function.
            [Optional, defaults to None.]

        consistent_read: bool
            Whether to use "Consistent Read" when querying DynamoDB.
            [Optional, defaults to True (DEFAULT_CONSISTENT_READ).]

    Errors:
        401 Unauthorized - Returned when the 'custom:gk_user_id' parameter is missing from the request context.
    """
    log_event(event)

    # Get player_id from requestContext:
    player_id = get_player_id(event)
    if player_id is None:
        return response_envelope(status_code=401, response_obj=EMPTY_RESPONSE)

    # Get query param inputs:
    page_size = int(get_query_string_param(event, 'page_size', DEFAULT_PAGE_SIZE))
    start_key = sanitize(get_query_string_param(event, 'start_key', DEFAULT_START_KEY))
    consistent_read = bool(strtobool(get_query_string_param(event, 'consistent_read', DEFAULT_CONSISTENT_READ)))

    if start_key is not None and len(start_key) > 0:
        paging_token = get_query_string_param(event, 'paging_token')
        # validate this pagination token
        if not validate_pagination_token(player_id, {"slot_name": start_key}, paging_token):
           return response_envelope(403, "Invalid paging token")

    # Get all metadata from DynamoDB:
    all_metadata, next_start_key = get_all_slots_metadata(player_id, page_size, consistent_read, start_key)

    # Construct response object:
    return response_envelope(
        status_code=200,
        response_obj={
            'slots_metadata': all_metadata
        },
        next_start_key=next_start_key,
        player_id=player_id
    )


def get_all_slots_metadata(player_id: str, page_size: int, consistent_read: bool, start_key: Optional[str]):
    """Get metadata for all save slots, or an empty list if no metadata is found."""
    gamesaves_table = ddb.get_table(table_name=os.environ.get('GAMESAVES_TABLE_NAME'))
    query_params = ddb.query_request_param(
        key_id='player_id',
        key_value=player_id,
        response_limit=page_size,
        use_consistent_read=consistent_read,
        start_key=create_exclusive_start_key(player_id, start_key)
    )
    response = gamesaves_table.query(**query_params)

    return response['Items'], create_next_start_key(response.get('LastEvaluatedKey'))


def create_exclusive_start_key(player_id: str, start_key: Optional[str]) -> Optional[Dict[str, str]]:
    """
    Create the 'ExclusiveStartKey' parameter for the DynamoDB query, based on the user-provided 'start_key' parameter to
    this Lambda function.
    """
    if start_key:
        return {
            'player_id': player_id,
            'slot_name': start_key,
        }
    else:
        return None


def create_next_start_key(last_evaluated_key: Optional[dict]) -> Optional[dict]:
    """
    Create the 'next_start_key' that will be returned in the response body of this Lambda function.

    Is identical to the 'LastEvaluatedKey' returned from DynamoDB, except the 'player_id' is removed for security.
    If 'player_id' was included as a user-provided parameter to the Lambda function, then a malicious player could fetch
    another player's slot metadata.
    """
    if last_evaluated_key is None:
        return None

    return {
        'slot_name': last_evaluated_key['slot_name']
    }
