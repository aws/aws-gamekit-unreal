# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

from datetime import datetime
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


# Patch Lambda environment variables:
@patch.dict(os.environ, {
    'BUNDLES_TABLE_NAME': BUNDLES_TABLE_NAME,
    'BUNDLE_ITEMS_TABLE_NAME': ITEMS_TABLE_NAME},
            clear=True)
class TestAdd(TestCase):
    def setUp(self):
        index.ddb_client = MagicMock()

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

    def test_add_bundle_invalid_player_returns_403_error(self):
        test_event = _build_add_event("u123", "bundleA", "xp", "99")
        test_event['requestContext'] = None

        result = index.lambda_handler(test_event, None)

        self.assertEqual(result['statusCode'], 403)
        self.assertFalse(index.ddb_client.called)

    def test_add_bundle_empty_body_returns_400_error(self):
        test_event = _build_add_event("u123", "bundleA", "xp", "99")
        test_event['body'] = '{}'

        result = index.lambda_handler(test_event, None)

        self.assertEqual(result['statusCode'], 400)
        self.assertFalse(index.ddb_client.called)
