# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

import os
import logging
from distutils.util import strtobool
from typing import Dict, Any

from gamekithelpers import ddb
from gamekithelpers.handler_request import get_player_id, get_path_param, get_query_string_param, log_event
from gamekithelpers.handler_response import response_envelope
from gamekithelpers.validation import is_valid_primary_identifier

DEFAULT_CONSISTENT_READ = 'True'
EMPTY_RESPONSE = {}

logger = logging.getLogger()
logger.setLevel(logging.INFO)


def lambda_handler(event, context):
    """
    Retrieve metadata for the player's specified save slot, or an empty dictionary if no metadata is found.

    Parameters:

    Request Context:
        custom:gk_user_id: str
            The player_id to get the slot metadata for. This value comes from the Cognito Authorizer that validates the
            API Gateway request.

    Path Parameters:
        slot_name: str
            The save slot to get the slot metadata for.

            Limited to 512 characters long, using alphanumeric characters, dashes (-), underscores (_), and periods (.).
            This lambda will return an error if a malformed slot name is provided.

    Query String Parameters:
        consistent_read: bool
            Whether to use "Consistent Read" when querying DynamoDB.
            [Optional, defaults to True (DEFAULT_CONSISTENT_READ).]

    Errors:
        400 Bad Request  - Returned when a malformed 'slot_name' path parameter is provided.
        401 Unauthorized - Returned when the 'custom:gk_user_id' parameter is missing from the request context.
    """
    log_event(event)

    # Get player_id from requestContext:
    player_id = get_player_id(event)
    if player_id is None:
        return response_envelope(status_code=401)

    # Get path param inputs:
    slot_name = get_path_param(event, 'slot_name')
    if not is_valid_primary_identifier(slot_name):
        logger.error(f'Malformed slot_name: {slot_name} provided for player_id: {player_id}')
        return response_envelope(status_code=400)

    # Get query param inputs:
    consistent_read = bool(strtobool(get_query_string_param(event, 'consistent_read', DEFAULT_CONSISTENT_READ)))

    # Get metadata from DynamoDB:
    metadata = get_slot_metadata(player_id, slot_name, consistent_read)

    # Construct response object:
    return response_envelope(
        status_code=200,
        response_obj=metadata
    )


def get_slot_metadata(player_id: str, slot_name: str, consistent_read: bool) -> Dict[str, Any]:
    """Get metadata for the named save slot, or an empty dictionary if no metadata is found."""
    gamesaves_table = ddb.get_table(table_name=os.environ.get('GAMESAVES_TABLE_NAME'))
    response = gamesaves_table.get_item(
        Key={
            'player_id': player_id,
            'slot_name': slot_name,
        },
        ConsistentRead=consistent_read,
    )

    return response.get('Item', EMPTY_RESPONSE)
