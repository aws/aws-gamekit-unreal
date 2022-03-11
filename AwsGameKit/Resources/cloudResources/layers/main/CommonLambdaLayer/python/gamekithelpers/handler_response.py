# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

"""
Helper functions for creating Response objects.
"""

import json
from decimal import Decimal
from http.client import responses

from gamekithelpers.pagination import generate_pagination_token
from gamekithelpers.types import JsonObject

ENVELOPE_KEY_META = "meta"
ENVELOPE_KEY_META_CODE = "code"
ENVELOPE_KEY_META_MESSAGE = "message"
ENVELOPE_KEY_DATA = "data"
ENVELOPE_KEY_PAGING = "paging"
ENVELOPE_KEY_PAGING_NEXT_START_KEY = "next_start_key"
ENVELOPE_KEY_PAGING_TOKEN = "paging_token"
ENVELOPE_KEY_PAGING_VERSION_KEY = "version"
ENVELOPE_KEY_PAGING_VERSION = "1.0.0"

class DecimalEncoder(json.JSONEncoder):
  def default(self, obj):
    if isinstance(obj, Decimal):
      return str(obj)
    return json.JSONEncoder.default(self, obj)


def invalid_request() -> dict:
    """
    Returns a generic bad request response back.
    Used when headers, query string parameters, or path parameters aren't correct.
    """
    return return_response(400, responses[400])


def forbidden_request() -> dict:
    """
    Returns a generic forbidden request response back.
    Used when the token given is not authorized to act on resources.
    """
    return return_response(403, responses[403])


def response_envelope(status_code: int,
                      status_message: str = None,
                      response_obj: JsonObject = None,
                      next_start_key: dict = None,
                      player_id: str = None) -> dict:
    """
    Build an HTTP response following the schema expected by GameKit CPP SDK.

    Status codes should match to the following situations:
    200: Success with body for GET requests and optionally for PUT, POST, and DELETE requests.
    204: No Content, Use return_response(204)
    400: See/Use invalid_request()
    401: Unauthorized, there is no player id for the player facing method.
    403: See/Use forbidden_request()
    404: Not found, when a resource for a GET request doesn't exist, or using POST to update a resource that doesn't exist.
    """

    envelope = {
        ENVELOPE_KEY_META: {
            ENVELOPE_KEY_META_CODE: status_code,
            ENVELOPE_KEY_META_MESSAGE: responses[status_code]
        },
        ENVELOPE_KEY_DATA: response_obj
    }

    if status_message is not None:
        envelope[ENVELOPE_KEY_META][ENVELOPE_KEY_META_MESSAGE] = status_message

    if next_start_key is not None:
        envelope[ENVELOPE_KEY_PAGING] = {
            ENVELOPE_KEY_PAGING_NEXT_START_KEY: next_start_key
        }
        # if we have a player_uuid, encrypt a token with our response
        if player_id is not None:
          paging_token = generate_pagination_token(player_id, next_start_key)
          envelope[ENVELOPE_KEY_PAGING][ENVELOPE_KEY_PAGING_TOKEN] = paging_token
          envelope[ENVELOPE_KEY_PAGING][ENVELOPE_KEY_PAGING_VERSION_KEY] = ENVELOPE_KEY_PAGING_VERSION

    return return_response(status_code, json.dumps(envelope, cls=DecimalEncoder))


def return_response(status_code: int, body: str = None, content_type: str = 'application/json') -> dict:
    """
    Build a simple response object with a status code and optional body.

    The "body" parameter should be a JSON encoded string. For example: body=json.dumps(my_dict}

    Body can be None, e.g. 204 no content response. Use when deleting items even if they don't exist.
    """
    return {
        'statusCode': status_code,
        'headers': {
            'Content-Type': content_type,
        },
        'body': body
    }
