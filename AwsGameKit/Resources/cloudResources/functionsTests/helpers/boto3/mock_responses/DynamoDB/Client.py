# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

"""
This module provides mock responses for the Boto3 DynamoDB Client APIs:
https://boto3.amazonaws.com/v1/documentation/api/latest/reference/services/dynamodb.html#client
"""
from typing import Dict, List, Any

from functionsTests.helpers.boto3.mock_responses.DynamoDB.shared import get_response_metadata


# Define types:
Attribute = Dict[str, Any]  # Example: {'S': 'foo_string'}
Item = Dict[str, Attribute]  # Example: {'player_id': {'S': 'foo_player_id'}}
Response = Dict[str, Any]


class Query:
    """
    Provides mock responses for the DynamoDB.Client.query API:
    https://boto3.amazonaws.com/v1/documentation/api/latest/reference/services/dynamodb.html#DynamoDB.Client.query
    """

    @staticmethod
    def mock_response(items: List[Item] = None, last_evaluated_key: Item = None) -> Response:
        """
        Get a mock response.

        :param items: [Optional] The items to return from the query. Defaults to an empty list (i.e. no matching items
        found).
        :param last_evaluated_key: [Optional] Specify this value if the query result is paginated. If specified, the
        return value will included the key 'LastEvaluatedKey' with this value.

        Example:
        >>> items = {
        ...     'player_id': {'S': 'foo_player_id'},
        ...     'total_points': {'N': '12'},
        ... }
        >>> response = Query.mock_response(items)
        """
        if items is None:
            items = []

        response = {
            'Items': items,
            'Count': len(items),
            'ScannedCount': len(items),
            'ResponseMetadata': get_response_metadata(),
        }
        if last_evaluated_key:
            response['LastEvaluatedKey'] = last_evaluated_key

        return response

    @staticmethod
    def mock_response_select_count(count: int) -> Response:
        """
        Get a mock response for when the API is called with: Select='COUNT'

        :param count: The count to return from the query.
        """
        return {
            'Count': count,
            'ScannedCount': count,
            'ResponseMetadata': get_response_metadata(),
        }
