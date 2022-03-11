# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

import os
from unittest import TestCase
from unittest.mock import patch, MagicMock

with patch("boto3.resource") as boto_resource_mock:
    from functions.usergamedata.DeleteBundle import index

BUNDLES_TABLE_NAME = 'test_bundles_table'
ITEMS_TABLE_NAME = 'test_bundleitems_table'


# Patch Lambda environment variables:
@patch.dict(os.environ, {
    'BUNDLES_TABLE_NAME': BUNDLES_TABLE_NAME,
    'BUNDLE_ITEMS_TABLE_NAME': ITEMS_TABLE_NAME
})
class TestDeleteBundle(TestCase):
    def setUp(self):
        index.ddb_resource = MagicMock()

    def test_delete_bundle_item_key_missing_returns_400_error(self):
        # Arrange
        event = self.get_lambda_event()
        event['body'] = '{}'

        # Act
        result = index.lambda_handler(event, None)

        # Assert
        self.assertEqual(400, result['statusCode'])
        index.ddb_resource.batch_write_item.assert_not_called()

    def test_delete_bundle_path_parameters_is_empty_returns_400_error(self):
        # Arrange
        event = self.get_lambda_event()
        event['pathParameters'] = {}

        # Act
        result = index.lambda_handler(event, None)

        # Assert
        self.assertEqual(400, result['statusCode'])
        index.ddb_resource.batch_write_item.assert_not_called()

    def test_delete_bundle_bundle_name_empty_returns_400_error(self):
        # Arrange
        event = self.get_lambda_event()
        event['pathParameters'] = {'bundle_name': None}

        # Act
        result = index.lambda_handler(event, None)

        # Assert
        self.assertEqual(400, result['statusCode'])
        index.ddb_resource.batch_write_item.assert_not_called()

    def test_delete_bundle_bundle_name_too_long_returns_400_error(self):
        # Arrange
        event = self.get_lambda_event()
        event['pathParameters'] = {'bundle_name': 'x' * 256}

        # Act
        result = index.lambda_handler(event, None)

        # Assert
        self.assertEqual(400, result['statusCode'])
        index.ddb_resource.batch_write_item.assert_not_called()

    def test_delete_bundle_invalid_player_returns_401_error(self):
        # Arrange
        event = self.get_lambda_event()
        event['requestContext'] = {'authorizer': {'claims': {}}}

        # Act
        result = index.lambda_handler(event, None)

        # Assert
        self.assertEqual(401, result['statusCode'])
        index.ddb_resource.batch_write_item.assert_not_called()

    def test_delete_bundle_key_passed_in_body_returns_204_success(self):
        # Arrange
        event = self.get_lambda_event()
        event['body'] = '{"bundle_item_keys": ["SCORE1","SCORE2"]}'

        # Act
        result = index.lambda_handler(event, None)

        # Assert
        self.assertEqual(204, result['statusCode'])
        index.ddb_resource.batch_write_item.assert_called_once_with(
            RequestItems={'test_bundleitems_table': [
                {'DeleteRequest':
                     {'Key': {'player_id_bundle': '12345678-1234-1234-1234-123456789012_BANANA_BUNDLE', 'bundle_item_key': 'SCORE1'}}},
                {'DeleteRequest':
                     {'Key': {'player_id_bundle': '12345678-1234-1234-1234-123456789012_BANANA_BUNDLE', 'bundle_item_key': 'SCORE2'}}}]})

    def test_delete_bundle_no_body_passed_returns_success(self):
        # Arrange
        event = self.get_lambda_event()
        event['body'] = None
        index.ddb_resource.Table().query.side_effect = [{'Items': [{'player_id_bundle': '12345_TestBushel', 'bundle_item_key': 'TestBanana'}]}, {'Items': []}]

        # Act
        result = index.lambda_handler(event, None)

        # Assert
        self.assertEqual(204, result['statusCode'])
        index.ddb_resource.batch_write_item.assert_called_once_with(
            RequestItems={'test_bundleitems_table': [
                {'DeleteRequest': {'Key': {'player_id_bundle': '12345_TestBushel', 'bundle_item_key': 'TestBanana'}}}]})
        index.ddb_resource.Table().delete_item.assert_called()

    @staticmethod
    def get_lambda_event():
        return {
            'resource': '/usergamedata/{bundle_name}',
            'path': '/usergamedata/BANANA_BUNDLE',
            'httpMethod': 'DELETE',
            'headers': {
                'Accept': '*/*',
                'Accept-Encoding': 'gzip, deflate, br',
                'Content-Type': 'application/json',
                'Host': 'abcdefghij.execute-api.us-west-2.amazonaws.com',
                'User-Agent': 'TestAgent',
                'X-Amzn-Trace-Id': 'Root=1-61003a02-7e1356b05a1e1569614c0c46',
                'X-Forwarded-For': '127.0.0.1',
                'X-Forwarded-Port': '443',
                'X-Forwarded-Proto': 'https'
            },
            'multiValueHeaders': {
                'Accept': ['*/*'],
                'Accept-Encoding': ['gzip, deflate, br'],
                'Content-Type': ['application/json'],
                'Host': ['abcdefghij.execute-api.us-west-2.amazonaws.com'],
                'User-Agent': ['TestAgent'],
                'X-Amzn-Trace-Id': ['Root=1-61003a02-7e1356b05a1e1569614c0c46'],
                'X-Forwarded-For': ['127.0.0.1'],
                'X-Forwarded-Port': ['443'],
                'X-Forwarded-Proto': ['https']
            },
            'queryStringParameters': None,
            'multiValueQueryStringParameters': None,
            'pathParameters': {
                'bundle_name': 'BANANA_BUNDLE'
            },
            'stageVariables': None,
            'requestContext': {
                'resourceId': 'abcdef',
                'authorizer': {
                    'claims': {
                        'sub': '12345678-1234-1234-1234-123456789012',
                        'iss': 'https://cognito-idp.us-west-2.amazonaws.com/us-west-2_123456789',
                        'cognito:username': 'jakschic',
                        'origin_jti': '12345678-1234-1234-1234-123456789012',
                        'aud': '7s24tlabcn8n0defbfoghijsgn',
                        'event_id': '6234d920-b637-4cdf-bd44-3a5e53f51569',
                        'token_use': 'id',
                        'auth_time': '1627438909',
                        'custom:gk_user_id': '12345678-1234-1234-1234-123456789012',
                        'exp': 'Wed Jul 28 03:21:49 UTC 2021',
                        'iat': 'Wed Jul 28 02:21:49 UTC 2021',
                        'jti': '7s24tlabcn8n0defbfoghijsgn',
                        'email': 'xyz@abc.def'
                    }
                },
                'domainName': 'abcdefghij.execute-api.us-west-2.amazonaws.com',
                'apiId': 'abcdefghij'
            },
            'body': None,
            'isBase64Encoded': False
        }
