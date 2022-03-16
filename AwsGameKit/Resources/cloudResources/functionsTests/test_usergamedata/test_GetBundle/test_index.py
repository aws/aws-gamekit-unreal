# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

import json
import os
from unittest import TestCase
from unittest.mock import patch, MagicMock

import gamekithelpers.pagination

with patch("boto3.client") as boto_client_mock:
    from functions.usergamedata.GetBundle import index

BUNDLES_TABLE_NAME = 'test_bundles_table'
ITEMS_TABLE_NAME = 'test_bundleitems_table'


def _build_get_bundle_event(user, bundle_name, start_key=None, limit=None):
    event = {
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
        'queryStringParameters': {}
    }

    if start_key:
        event['queryStringParameters']['next_start_key'] = start_key
        event['queryStringParameters']['paging_token'] = gamekithelpers.pagination.generate_pagination_token(user, start_key)
    if limit:
        event['queryStringParameters']['limit'] = limit

    return event


# Patch Lambda environment variables:
@patch.dict(os.environ, {
    'BUNDLES_TABLE_NAME': BUNDLES_TABLE_NAME,
    'BUNDLE_ITEMS_TABLE_NAME': ITEMS_TABLE_NAME},
            clear=True)
class TestGetBundle(TestCase):
    def setUp(self):
        index.ddb_client = MagicMock()

    def test_get_bundle_single_page_returns_success(self):
        test_user = 'u123'
        test_bundle = 'stats'
        test_event = _build_get_bundle_event(test_user, test_bundle)

        index.ddb_client.query.return_value = {
            'Items': [{
                "player_id_bundle": {"S": f"{test_user}_{test_bundle}"},
                "bundle_item_key": {"S": "xp"},
                "bundle_item_value": {"S": "99"}}]
        }

        result = index.lambda_handler(test_event, None)
        result_body_obj = json.loads(result['body'])

        self.assertEqual(result['statusCode'], 200)
        self.assertEqual(result_body_obj['data']['bundle_items'],
                         [{"player_id_bundle": f"{test_user}_{test_bundle}", "bundle_item_key": "xp", "bundle_item_value": "99"}])
        self.assertFalse('paging' in result)
        self.assertFalse('next_start_key' in result)
        index.ddb_client.query.assert_called_once_with(
            ExpressionAttributeNames={'#player_id_bundle': 'player_id_bundle'},
            ExpressionAttributeValues={':player_id_bundle': {'S': f'{test_user}_{test_bundle}'}},
            KeyConditionExpression='#player_id_bundle = :player_id_bundle',
            Limit=100,
            TableName=ITEMS_TABLE_NAME)

    def test_get_bundle_pagination_returns_success(self):
        test_user = 'u123'
        test_bundle = 'stats'
        test_start_key = f'{test_user}_{test_bundle},chest1'
        test_limit = 3
        test_event = _build_get_bundle_event(test_user, test_bundle, test_start_key, test_limit)

        index.ddb_client.query.return_value = {
            'Items': [{}],
            'LastEvaluatedKey': {
                "player_id_bundle": {"S": f"{test_user}_{test_bundle}"},
                "bundle_item_key": {"S": "hp"}}
        }

        result = index.lambda_handler(test_event, None)
        result_body_obj = json.loads(result['body'])

        self.assertEqual(result['statusCode'], 200)
        self.assertEqual(result_body_obj['paging']['next_start_key'], f'{test_user}_{test_bundle},hp')
        index.ddb_client.query.assert_called_once_with(
            ExclusiveStartKey={'player_id_bundle': {'S': f'{test_user}_{test_bundle}'}, 'bundle_item_key': {'S': 'chest1'}},
            ExpressionAttributeNames={'#player_id_bundle': 'player_id_bundle'},
            ExpressionAttributeValues={':player_id_bundle': {'S': f'{test_user}_{test_bundle}'}},
            KeyConditionExpression='#player_id_bundle = :player_id_bundle',
            Limit=test_limit,
            TableName=ITEMS_TABLE_NAME)

    def test_get_bundle_invalid_player_returns_401_error(self):
        test_event = _build_get_bundle_event("u123", "stats")
        test_event['requestContext'] = None

        result = index.lambda_handler(test_event, None)

        self.assertEqual(result['statusCode'], 401)
        self.assertFalse(index.ddb_client.called)

    def test_get_bundle_invalid_bundle_name_returns_400_error(self):
        test_event = _build_get_bundle_event("u123", "stats")
        test_event['pathParameters'].pop('bundle_name')

        result = index.lambda_handler(test_event, None)

        self.assertEqual(result['statusCode'], 400)
        self.assertFalse(index.ddb_client.called)
