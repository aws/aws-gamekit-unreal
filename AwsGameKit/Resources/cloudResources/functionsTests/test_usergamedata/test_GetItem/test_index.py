# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

import os
from unittest import TestCase
from unittest.mock import patch, call, MagicMock

with patch("boto3.client") as boto_client_mock:
    from functions.usergamedata.GetItem import index

BUNDLES_TABLE_NAME = 'test_bundles_table'
ITEMS_TABLE_NAME = 'test_bundleitems_table'


def _build_get_item_event(user, bundle_name, bundle_key):
    return {
        'requestContext': {
            'authorizer': {
                'claims': {
                    'custom:gk_user_id': user
                }
            }
        },
        'pathParameters': {
            'bundle_name': bundle_name,
            'bundle_item_key': bundle_key
        }
    }


# Patch Lambda environment variables:
@patch.dict(os.environ, {
    'BUNDLES_TABLE_NAME': BUNDLES_TABLE_NAME,
    'BUNDLE_ITEMS_TABLE_NAME': ITEMS_TABLE_NAME},
            clear=True)
class TestGetItem(TestCase):
    def setUp(self):
        index.ddb_client = MagicMock()

    def test_get_item_returns_success(self):
        test_user = 'u123'
        test_bundle = 'stats'
        test_item = 'xp'
        test_value = '99'
        test_event = _build_get_item_event(test_user, test_bundle, test_item)
        index.ddb_client.get_item.return_value = {'Item': {
            "player_id_bundle": {"S": f"{test_user}_{test_bundle}"},
            "bundle_item_key": {"S": test_item},
            "bundle_item_value": {"S": test_value}}}

        result = index.lambda_handler(test_event, None)

        calls = [
            call(Key={'player_id_bundle': {'S': f'{test_user}_{test_bundle}'}, 'bundle_item_key': {'S': test_item}},
                 TableName=ITEMS_TABLE_NAME)
        ]
        self.assertEqual(result['statusCode'], 200)
        index.ddb_client.get_item.assert_has_calls(calls, any_order=False)

    def test_get_item_invalid_player_returns_401_error(self):
        test_event = _build_get_item_event("u123", "bundleA", "xp")
        test_event['requestContext'] = None

        result = index.lambda_handler(test_event, None)

        self.assertEqual(result['statusCode'], 401)
        self.assertFalse(index.ddb_client.called)

    def test_get_item_invalid_bundle_name_returns_400_error(self):
        test_event = _build_get_item_event("u123", "bundleA", "xp")
        test_event['pathParameters'].pop('bundle_name')

        result = index.lambda_handler(test_event, None)

        self.assertEqual(result['statusCode'], 400)
        self.assertFalse(index.ddb_client.called)

    def test_get_item_invalid_bundle_key_returns_400_error(self):
        test_event = _build_get_item_event("u123", "bundleA", "xp")
        test_event['pathParameters'].pop('bundle_item_key')

        result = index.lambda_handler(test_event, None)

        self.assertEqual(result['statusCode'], 400)
        self.assertFalse(index.ddb_client.called)

    def test_get_item_invalid_result_returns_404_error(self):
        test_event = _build_get_item_event("u123", "bundleA", "xp")
        index.ddb_client.get_item.return_value = {}

        result = index.lambda_handler(test_event, None)

        self.assertEqual(result['statusCode'], 404)
