# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

from datetime import datetime
import json
import os
from unittest import TestCase
from unittest.mock import patch, MagicMock, call

with patch("boto3.client") as boto_client_mock:
    from functions.usergamedata.Add import index

BUNDLES_TABLE_NAME = 'test_bundles_table'
ITEMS_TABLE_NAME = 'test_bundleitems_table'


def _build_add_event(user, bundle_name, bundle_key, bundle_value):
    return {
        'requestContext': {
            'authorizer': {
                'claims': {
                    'custom:gk_user_id': user
                }
            }
        },
        'pathParameters': {
            'bundle_name': bundle_name
        },
        'body': f'{{\"{bundle_key}\":\"{bundle_value}\"}}'
    }


def _build_add_two_events(user, bundle_name, bundle_key_one, bundle_value_one, bundle_key_two, bundle_value_two):
    return {
        'requestContext': {
            'authorizer': {
                'claims': {
                    'custom:gk_user_id': user
                }
            }
        },
        'pathParameters': {
            'bundle_name': bundle_name
        },
        'body': f'{{\"{bundle_key_one}\":\"{bundle_value_one}\",\"{bundle_key_two}\":\"{bundle_value_two}\"}}'
    }


class MockClientErrorException(BaseException):
    def __init__(self):
        pass


# Patch Lambda environment variables:
@patch.dict(os.environ, {
    'BUNDLES_TABLE_NAME': BUNDLES_TABLE_NAME,
    'BUNDLE_ITEMS_TABLE_NAME': ITEMS_TABLE_NAME},
            clear=True)
class TestAdd(TestCase):
    def setUp(self):
        index.ddb_client = MagicMock()

    @patch('functions.usergamedata.Add.index.datetime')
    def test_add_bundle_one_failure_returns_unprocessed_item(self, mock_datetime: MagicMock):
        mock_datetime.utcnow.return_value = datetime(2021, 8, 4, 1, 23, 34, 56)
        test_user = 'u123'
        test_bundle = 'stats'
        test_key_one = 'xp'
        test_value_one = '99'
        test_key_two = 'bananas_sold'
        test_value_two = '128'
        test_event = _build_add_two_events(test_user, test_bundle, test_key_one, test_value_one, test_key_two, test_value_two)

        index.botocore.exceptions.ClientError = MockClientErrorException
        index.ddb_client.update_item.side_effect = [
            None,
            None,
            index.botocore.exceptions.ClientError()
        ]

        result = index.lambda_handler(test_event, None)
        result_body_obj = json.loads(result['body'])

        self.assertEqual(result['statusCode'], 201)
        self.assertEqual(result_body_obj['data']['unprocessed_items'], [{'bundle_item_key': 'bananas_sold', 'bundle_item_value': '128'}])
        self.assertEqual(index.ddb_client.update_item.call_count, 3)

    @patch('functions.usergamedata.Add.index.datetime')
    def test_add_bundle_first_item_error_second_item_succeeds_returns_unprocessed_item(self, mock_datetime: MagicMock):
        mock_datetime.utcnow.return_value = datetime(2021, 8, 4, 1, 23, 34, 56)
        test_user = 'u123'
        test_bundle = 'stats'
        test_key_one = 'xp'
        test_value_one = '99'
        test_key_two = 'bananas_sold'
        test_value_two = '128'
        test_event = _build_add_two_events(test_user, test_bundle, test_key_one, test_value_one, test_key_two, test_value_two)

        index.botocore.exceptions.ClientError = MockClientErrorException
        index.ddb_client.update_item.side_effect = [
            None,
            index.botocore.exceptions.ClientError(),
            None
        ]

        result = index.lambda_handler(test_event, None)
        result_body_obj = json.loads(result['body'])

        self.assertEqual(result['statusCode'], 201)
        self.assertEqual(result_body_obj['data']['unprocessed_items'], [{'bundle_item_key': 'xp', 'bundle_item_value': '99'}])
        self.assertEqual(index.ddb_client.update_item.call_count, 3)

    @patch('functions.usergamedata.Add.index.datetime')
    def test_add_bundle_name_returns_422_error_with_unprocessed_items(self, mock_datetime: MagicMock):
        mock_datetime.utcnow.return_value = datetime(2021, 8, 4, 1, 23, 34, 56)
        test_user = 'u123'
        test_bundle = 'stats'
        test_key_one = 'xp'
        test_value_one = '99'
        test_key_two = 'bananas_sold'
        test_value_two = '128'
        test_event = _build_add_two_events(test_user, test_bundle, test_key_one, test_value_one, test_key_two, test_value_two)

        index.botocore.exceptions.ClientError = MockClientErrorException
        index.ddb_client.update_item.side_effect = [
            index.botocore.exceptions.ClientError()
        ]

        result = index.lambda_handler(test_event, None)
        result_body_obj = json.loads(result['body'])

        self.assertEqual(result['statusCode'], 422)
        self.assertEqual(result_body_obj['data']['bundle_name'], 'stats')
        self.assertEqual(result_body_obj['data']['unprocessed_items'], [{'bundle_item_key': 'xp', 'bundle_item_value': '99'}, {'bundle_item_key': 'bananas_sold', 'bundle_item_value': '128'}])
        self.assertEqual(index.ddb_client.update_item.call_count, 1)

    @patch('functions.usergamedata.Add.index.datetime')
    def test_add_invalid_bundle_returns_413_failure(self, mock_datetime: MagicMock):
        mock_datetime.utcnow.return_value = datetime(2021, 8, 4, 1, 23, 34, 56)
        test_user = 'u123'
        test_bundle = 'stats'
        test_key = 'a' * 500;
        test_value = '99'
        test_event = _build_add_event(test_user, test_bundle, test_key, test_value)

        result = index.lambda_handler(test_event, None)

        self.assertEqual(result['statusCode'], 413)

    @patch('functions.usergamedata.Add.index.datetime')
    def test_add_bundle_returns_success(self, mock_datetime: MagicMock):
        mock_datetime.utcnow.return_value = datetime(2021, 8, 4, 1, 23, 34, 56)
        test_user = 'u123'
        test_bundle = 'stats'
        test_key = 'xp'
        test_value = '99'
        test_event = _build_add_event(test_user, test_bundle, test_key, test_value)

        result = index.lambda_handler(test_event, None)

        calls = [
            call(Key={'player_id': {'S': test_user}, 'bundle_name': {'S': test_bundle}},
                 ReturnValues='ALL_NEW',
                 TableName=BUNDLES_TABLE_NAME),
            call(ExpressionAttributeNames={'#bundle_item_value': 'bundle_item_value',
                                           '#created_at': 'created_at',
                                           '#updated_at': 'updated_at'},
                 ExpressionAttributeValues={':bundle_item_value': {'S': test_value},
                                            ':created_at': {'S': '2021-08-04T01:23:34.000056+00:00'},
                                            ':updated_at': {'S': '2021-08-04T01:23:34.000056+00:00'}},
                 Key={'player_id_bundle': {'S': f'{test_user}_{test_bundle}'}, 'bundle_item_key': {'S': test_key}},
                 ReturnValues='ALL_NEW',
                 TableName=ITEMS_TABLE_NAME,
                 UpdateExpression='SET #bundle_item_value = :bundle_item_value, '
                                  '#created_at = if_not_exists(created_at, :created_at), #updated_at = :updated_at')
        ]

        self.assertEqual(result['statusCode'], 201)
        index.ddb_client.update_item.assert_has_calls(calls, any_order=False)

    def test_add_bundle_invalid_player_returns_401_error(self):
        test_event = _build_add_event("u123", "bundleA", "xp", "99")
        test_event['requestContext'] = None

        result = index.lambda_handler(test_event, None)

        self.assertEqual(result['statusCode'], 401)
        self.assertFalse(index.ddb_client.called)

    def test_add_bundle_empty_body_returns_400_error(self):
        test_event = _build_add_event("u123", "bundleA", "xp", "99")
        test_event['body'] = '{}'

        result = index.lambda_handler(test_event, None)

        self.assertEqual(result['statusCode'], 400)
        self.assertFalse(index.ddb_client.called)
