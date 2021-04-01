# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

import json
import os
from unittest import TestCase
from unittest.mock import patch, call, MagicMock

import gamekithelpers.pagination

with patch("boto3.client") as boto_client_mock:
    from functions.usergamedata.ListBundles import index

BUNDLES_TABLE_NAME = 'test_bundles_table'
ITEMS_TABLE_NAME = 'test_bundleitems_table'

def _build_get_all_event(user, start_bundle_key=None, limit=None):
    event = {
        'requestContext': {
            'authorizer': {
                'claims': {
                    'custom:gk_user_id': user
                }
            }
        },
        'queryStringParameters': {}
    }

    if start_bundle_key:
        event['queryStringParameters']['next_start_key'] = start_bundle_key
        event['queryStringParameters']['paging_token'] = gamekithelpers.pagination.generate_pagination_token(user, start_bundle_key)
    if limit:
        event['queryStringParameters']['limit'] = limit

    return event


# Patch Lambda environment variables:
@patch.dict(os.environ, {
    'BUNDLES_TABLE_NAME': BUNDLES_TABLE_NAME,
    'BUNDLE_ITEMS_TABLE_NAME': ITEMS_TABLE_NAME},
            clear=True)
class TestListBundles(TestCase):
    def setUp(self):
        index.ddb_client = MagicMock()

    def test_list_bundles_single_page_returns_success(self):
        test_user = 'u123'
        test_bundle = 'stats'
        test_event = _build_get_all_event(test_user)

        bundles_query_result = {
            'Items': [
                {"bundle_name": {"S": f"{test_bundle}"}}
            ]
        }

        index.ddb_client.query.side_effect = [bundles_query_result]

        result = index.lambda_handler(test_event, None)
        result_body_obj = json.loads(result['body'])

        self.assertEqual(result['statusCode'], 200)
        self.assertEqual(result_body_obj['data']['bundle_names'], [{'bundle_name': 'stats'}])
        self.assertFalse('paging' in result)
        self.assertFalse('next_start_key' in result)

        bundle_query_call = call(
            ExpressionAttributeNames={'#player_id': 'player_id'},
            ExpressionAttributeValues={':player_id': {'S': test_user}},
            KeyConditionExpression='#player_id = :player_id',
            Limit=100,
            TableName=BUNDLES_TABLE_NAME)

        index.ddb_client.query.assert_has_calls([bundle_query_call], any_order=False)

    def test_list_bundles_pagination_returns_success(self):
        test_user = 'u123'
        test_start_key = 'stats'
        test_limit = 3
        test_event = _build_get_all_event(test_user, test_start_key, test_limit)

        bundles_query_result = {
            'Items': [
                {"bundle_name": {"S": f"{test_start_key}"}},
                {"bundle_name": {"S": "chest1"}}
            ],
            'LastEvaluatedKey': {
                'bundle_name': {'S': 'chest1'}
            }
        }

        index.ddb_client.query.side_effect = [bundles_query_result]

        result = index.lambda_handler(test_event, None)
        result_body_obj = json.loads(result['body'])

        self.assertEqual(result['statusCode'], 200)
        self.assertTrue(result_body_obj['paging']['next_start_key'], 'chest1')

        bundle_query_call = call(
            ExclusiveStartKey={'player_id': {'S': test_user}, 'bundle_name': {'S': test_start_key}},
            ExpressionAttributeNames={'#player_id': 'player_id'},
            ExpressionAttributeValues={':player_id': {'S': test_user}},
            KeyConditionExpression='#player_id = :player_id',
            Limit=test_limit,
            TableName=BUNDLES_TABLE_NAME)

        index.ddb_client.query.assert_has_calls([bundle_query_call], any_order=False)

    def test_list_bundles_invalid_player_returns_403_error(self):
        test_event = _build_get_all_event("u123")
        test_event['requestContext'] = None

        result = index.lambda_handler(test_event, None)

        self.assertEqual(result['statusCode'], 403)
        self.assertFalse(index.ddb_client.called)
