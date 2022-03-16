# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

import botocore
from gamekithelpers import handler_request, handler_response, ddb
import os

ddb_table = ddb.get_table(os.environ.get('IDENTITY_TABLE_NAME'))


def lambda_handler(event, context):
    handler_request.log_event(event)
    gk_user_id = handler_request.get_player_id(event)
    if not gk_user_id:
        return handler_response.response_envelope(401)

    try:
        response = ddb_table.get_item(**ddb.get_item_request_param({'gk_user_id': gk_user_id}))
    except botocore.exceptions.ClientError as err:
        print(f"Error getting item: {gk_user_id}. Error: {err}")
        raise err

    desired_fields = ["updated_at", "created_at", "gk_user_id",
                      "facebook_external_id", "facebook_ref_id",
                      "user_name"]

    filtered_response = {}
    for k, v in response["Item"].items():
        if k in desired_fields:
            # ignore type in dynamo response cast value to string
            filtered_response[k] = v

    return handler_response.response_envelope(200, None, filtered_response)
