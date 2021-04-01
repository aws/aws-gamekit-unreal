# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

from decimal import Decimal
from distutils.util import strtobool
import json
import os
from unittest import TestCase
from unittest.mock import patch, MagicMock

import gamekithelpers.pagination
from functions.gamesaving.GetAllSlotsMetadata import index
from functionsTests.helpers.boto3.mock_responses.DynamoDB.Table import Query
from functionsTests.helpers.sample_lambda_events import http_event


# Patch Lambda environment variables:
@patch.dict(os.environ, {
    'GAMESAVES_TABLE_NAME': 'gamekit_dev_foogamename_player_gamesaves'
})
class TestIndex(TestCase):

    @patch('gamekithelpers.ddb.get_table')
    def test_can_get_all_data_in_a_single_lambda_invocation(self, mock_get_table: MagicMock()):
        # Arrange
        event = self.get_lambda_event()
        mock_items = self.get_mock_items()
        mock_gamesaves_table = mock_get_table()
        mock_gamesaves_table.query.return_value = Query.mock_response(items=mock_items)

        # Act
        result = index.lambda_handler(event, context=None)

        # Assert
        self.assertEqual(200, result['statusCode'])

        response_body = json.loads(result['body'])
        all_slots_metadata = response_body['data']['slots_metadata']
        self.assertEqual(len(mock_items), len(all_slots_metadata))

        paging = response_body.get('paging')
        self.assertIsNone(paging)

    @patch('gamekithelpers.ddb.get_table')
    def test_can_get_first_page_of_paginated_data(self, mock_get_table: MagicMock()):
        # Arrange
        event = self.get_lambda_event()
        page_size = 1
        event['queryStringParameters'] = {
            'page_size': str(page_size)
        }
        mock_item = self.get_mock_items()[0]
        mock_gamesaves_table = mock_get_table()
        mock_gamesaves_table.query.return_value = Query.mock_response(
            items=[mock_item],
            last_evaluated_key={
                'player_id': mock_item['player_id'],
                'slot_name': mock_item['slot_name'],
            }
        )

        # Act
        result = index.lambda_handler(event, context=None)

        # Assert
        self.assertEqual(200, result['statusCode'])

        response_body = json.loads(result['body'])
        all_slots_metadata = response_body['data']['slots_metadata']
        self.assertEqual(page_size, len(all_slots_metadata))

        paging = response_body['paging']
        self.assertIsNotNone(paging)

    @patch('gamekithelpers.ddb.get_table')
    def test_can_get_subsequent_page_of_paginated_data(self, mock_get_table: MagicMock()):
        # Arrange
        event = self.get_lambda_event()
        page_size = 1
        last_item = self.get_mock_items()[0]
        event['queryStringParameters'] = {
            'page_size': str(page_size),
            'start_key': last_item['slot_name'],
            'paging_token': gamekithelpers.pagination.generate_pagination_token('foo_player_id', {'slot_name': last_item['slot_name']})
        }
        mock_item = self.get_mock_items()[1]
        mock_gamesaves_table = mock_get_table()
        mock_gamesaves_table.query.return_value = Query.mock_response(
            items=[mock_item],
            last_evaluated_key={
                'player_id': mock_item['player_id'],
                'slot_name': mock_item['slot_name'],
            }
        )

        # Act
        result = index.lambda_handler(event, context=None)

        # Assert
        self.assertEqual(200, result['statusCode'])

        response_body = json.loads(result['body'])
        all_slots_metadata = response_body['data']['slots_metadata']
        self.assertEqual(page_size, len(all_slots_metadata))

        paging = response_body['paging']
        self.assertIsNotNone(paging)

    @patch('gamekithelpers.ddb.get_table')
    def test_can_get_final_page_of_paginated_data(self, mock_get_table: MagicMock()):
        # Arrange
        event = self.get_lambda_event()
        page_size = 1
        last_item = self.get_mock_items()[1]
        event['queryStringParameters'] = {
            'page_size': str(page_size),
            'start_key': last_item['slot_name'],
            'paging_token': gamekithelpers.pagination.generate_pagination_token('foo_player_id', {'slot_name': last_item['slot_name']})
        }
        mock_item = self.get_mock_items()[2]
        mock_gamesaves_table = mock_get_table()
        mock_gamesaves_table.query.return_value = Query.mock_response(items=[mock_item])

        # Act
        result = index.lambda_handler(event, context=None)

        # Assert
        self.assertEqual(200, result['statusCode'])

        response_body = json.loads(result['body'])
        all_slots_metadata = response_body['data']['slots_metadata']
        self.assertEqual(page_size, len(all_slots_metadata))

        paging = response_body.get('paging')
        self.assertIsNone(paging)

    @patch('gamekithelpers.ddb.get_table')
    def test_can_get_data_using_default_query_string_parameters(self, mock_get_table: MagicMock()):
        # Arrange
        event = self.get_lambda_event()
        mock_items = self.get_mock_items()
        mock_gamesaves_table = mock_get_table()
        mock_gamesaves_table.query.return_value = Query.mock_response(items=mock_items)
        default_consistent_read = bool(strtobool(index.DEFAULT_CONSISTENT_READ))

        # Act
        result = index.lambda_handler(event, context=None)

        # Assert
        self.assertEqual(200, result['statusCode'])

        response_body = json.loads(result['body'])
        all_slots_metadata = response_body['data']['slots_metadata']
        self.assertEqual(len(mock_items), len(all_slots_metadata))

        paging = response_body.get('paging')
        self.assertIsNone(paging)

        mock_gamesaves_table.query.assert_called_once()
        args, kwargs = mock_gamesaves_table.query.call_args
        self.assertEqual(int(index.DEFAULT_PAGE_SIZE), kwargs['Limit'])
        self.assertEqual(default_consistent_read, kwargs['ConsistentRead'])

    @patch('gamekithelpers.ddb.get_table')
    def test_can_get_data_using_specified_query_string_parameters(self, mock_get_table: MagicMock()):
        # Arrange
        event = self.get_lambda_event()
        page_size = 1
        consistent_read = False
        event['queryStringParameters'] = {
            'page_size': str(page_size),
            'consistent_read': str(consistent_read)
        }
        mock_items = self.get_mock_items()
        mock_gamesaves_table = mock_get_table()
        mock_gamesaves_table.query.return_value = Query.mock_response(items=mock_items)

        # Act
        result = index.lambda_handler(event, context=None)

        # Assert
        self.assertEqual(200, result['statusCode'])

        response_body = json.loads(result['body'])
        all_slots_metadata = response_body['data']['slots_metadata']
        self.assertEqual(len(mock_items), len(all_slots_metadata))

        paging = response_body.get('paging')
        self.assertIsNone(paging)

        mock_gamesaves_table.query.assert_called_once()
        args, kwargs = mock_gamesaves_table.query.call_args
        self.assertEqual(page_size, kwargs['Limit'])
        self.assertEqual(consistent_read, kwargs['ConsistentRead'])

    def test_lambda_returns_a_401_error_code_when_player_id_is_missing_from_the_request_context(self):
        # Arrange
        event = self.get_lambda_event()
        event['requestContext']['authorizer']['claims'].pop('custom:gk_user_id')
        context = None

        # Act
        result = index.lambda_handler(event, context)

        # Assert
        self.assertEqual(401, result['statusCode'])

    @staticmethod
    def get_lambda_event() -> dict:
        """Get a sample Lambda event with none of the optional parameters included."""
        return http_event()

    @staticmethod
    def get_mock_items():
        """Get the mock DynamoDB items that are returned from the query. Each item is a save slot's metadata."""
        return [
            {
                'player_id': 'foo_player_id',
                'slot_name': 'autosave-slot-0',
                'last_modified': 1626924349000,
                'description': '',
                'size': Decimal(123456789)
            },
            {
                'player_id': 'foo_player_id',
                'slot_name': 'autosave-slot-1',
                'last_modified': 1627010749000,
                'description': '',
                'size': Decimal(123456789)
            },
            {
                'player_id': 'foo_player_id',
                'slot_name': 'autosave-slot-2',
                'last_modified': 1627097149000,
                'description': '',
                'size': Decimal(123456789)
            }
        ]
