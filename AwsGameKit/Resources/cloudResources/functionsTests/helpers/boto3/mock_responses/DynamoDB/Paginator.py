# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

"""
This module provides mock responses for the Boto3 DynamoDB Paginator APIs:
https://boto3.amazonaws.com/v1/documentation/api/latest/reference/services/dynamodb.html#paginators
"""
from typing import Dict, List, Any, Iterable

from functionsTests.helpers.boto3.mock_responses.DynamoDB.Client import Query as ClientQuery

# Define types:
Attribute = Dict[str, Any]  # Example: {'S': 'foo_string'}
Item = Dict[str, Attribute]  # Example: {'player_id': {'S': 'foo_player_id'}}
Response = Dict[str, Any]


class Query:
    """
    Provides mock responses for the DynamoDB.Paginator.Query API:
    https://boto3.amazonaws.com/v1/documentation/api/latest/reference/services/dynamodb.html#DynamoDB.Paginator.Query
    """

    @staticmethod
    def mock_response(pages: Iterable[List[Item]] = None) -> Iterable[Response]:
        """
        Get a mock response.

        Note: the mock response is MISSING the 'NextToken' key. This key is generally not used by developers. Instead,
        it is used by the boto paginator behind the scenes for fetching the next page of data from DynamoDB.

        :param pages: [Optional] The items to return on each page. Defaults to a single page with no items (i.e. no
        matching items found).
        :return: An iterable object which can be used to iterate over the pages of items.
        """
        if pages is None:
            first_page = []  # Empty!
            pages = [
                first_page,
            ]

        return [
            ClientQuery.mock_response(items=page) for page in pages
        ]

    @staticmethod
    def mock_response_select_count(pages: Iterable[int] = None) -> Iterable[Response]:
        """
        Get a mock response for when the API is called with: Select='COUNT'

        :param pages: [Optional] The 'Count' to return on each page. Defaults to a single page with Count=0 (i.e. no
        matching items found).
        :return: An iterable object which can be used to iterate over the pages of items.
        """
        if pages is None:
            first_page = 0  # Empty!
            pages = [
                first_page,
            ]

        return [
            ClientQuery.mock_response_select_count(count=page) for page in pages
        ]
