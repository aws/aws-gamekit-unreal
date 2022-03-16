# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

"""
This module provides mock responses for the Boto3 DynamoDB Table APIs:
https://boto3.amazonaws.com/v1/documentation/api/latest/reference/services/dynamodb.html#DynamoDB.Table
"""
from typing import Dict, List, Any

from functionsTests.helpers.boto3.mock_responses.DynamoDB.shared import get_response_metadata


# Define types:
Item = Dict[str, Any]
Response = Dict[str, Any]


class GetItem:
    """
    Provides mock responses for the DynamoDB.Table.get_item API:
    https://boto3.amazonaws.com/v1/documentation/api/latest/reference/services/dynamodb.html#DynamoDB.Table.get_item
    """

    @staticmethod
    def mock_response(item: Item = None) -> Response:
        """
        Get a mock response for when a matching item is (or is not) found in the DynamoDB table.

        :param item: [Optional] The item to get from the DynamoDB table. Defaults to None (no matching item found), in
        which case the return object is missing the 'Item' key.

        Examples:
        >>> GetItem.mock_response()  # No item found
        >>> GetItem.mock_response(item=None)  # No item found
        >>> GetItem.mock_response(item={'foo_string': 'example', 'foo_datetime': '2021-07-20T12:32:15.456Z'})
        """
        response = GetItem.mock_response_no_matching_item()
        if item:
            response['Item'] = item

        return response

    @staticmethod
    def mock_response_no_matching_item() -> Response:
        """
        Get a mock response for when there is no matching item found in the DynamoDB table.
        """
        return {
            'ResponseMetadata': get_response_metadata()
        }


class PutItem:
    """
    Provides mock responses for the DynamoDB.Table.put_item API:
    https://boto3.amazonaws.com/v1/documentation/api/latest/reference/services/dynamodb.html#DynamoDB.Table.put_item
    """

    @staticmethod
    def mock_response() -> Response:
        """
        Get a mock response.
        """
        return {
            'ResponseMetadata': get_response_metadata()
        }


class Query:
    """
    Provides mock responses for the DynamoDB.Table.query API:
    https://boto3.amazonaws.com/v1/documentation/api/latest/reference/services/dynamodb.html#DynamoDB.Table.query
    """

    @staticmethod
    def mock_response(items: List[Item] = None, last_evaluated_key: Item = None) -> Response:
        """
        Get a mock response.

        :param items: [Optional] The items to return from the query. Defaults to an empty list (i.e. no matching items
        found).
        :param last_evaluated_key: [Optional] Specify this value if the query result is paginated. If specified, the
        return value will included the key 'LastEvaluatedKey' with this value.
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
