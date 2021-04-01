# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

from datetime import datetime
import os
from unittest import TestCase
from unittest.mock import patch, MagicMock, call

with patch("boto3.client") as boto_client_mock:
    from functions.usergamedata.UpdateItem import index

ITEMS_TABLE_NAME = 'test_bundleitems_table'


class MockConditionalCheckFailedException(BaseException):
    def __init__(self):
        pass


# Patch Lambda environment variables:
@patch.dict(os.environ, {
    'BUNDLE_ITEMS_TABLE_NAME': ITEMS_TABLE_NAME
})
class TestUpdateItem(TestCase):
    def setUp(self):
        index.ddb_client = MagicMock()

    def test_update_item_invalid_player_returns_403_error(self):
        # Arrange
        event = self.get_lambda_event()
        event['requestContext'] = {'authorizer': {'claims': {}}}

        # Act
        result = index.lambda_handler(event, None)

        # Assert
        self.assertEqual(403, result['statusCode'])
        index.ddb_client.batch_write_item.assert_not_called()

    def test_update_item_path_parameters_empty_returns_400_error(self):
        # Arrange
        event = self.get_lambda_event()
        event['pathParameters'] = {}

        # Act
        result = index.lambda_handler(event, None)

        # Assert
        self.assertEqual(400, result['statusCode'])
        index.ddb_client.batch_write_item.assert_not_called()

    def test_update_item_bundle_name_is_empty_returns_400_error(self):
        # Arrange
        event = self.get_lambda_event()
        event['pathParameters'] = {'bundle_name': None}

        # Act
        result = index.lambda_handler(event, None)

        # Assert
        self.assertEqual(400, result['statusCode'])
        index.ddb_client.batch_write_item.assert_not_called()

    def test_update_item_bundle_name_invalid_returns_414_error(self):
        # Arrange
        event = self.get_lambda_event()
        event['pathParameters'] = {'bundle_name': 'x' * 256}

        # Act
        result = index.lambda_handler(event, None)

        # Assert
        self.assertEqual(414, result['statusCode'])
        index.ddb_client.batch_write_item.assert_not_called()

    def test_update_item_bundle_item_key_is_empty_returns_400_error(self):
        # Arrange
        event = self.get_lambda_event()
        event['pathParameters'] = {'bundle_item_key': None}

        # Act
        result = index.lambda_handler(event, None)

        # Assert
        self.assertEqual(400, result['statusCode'])
        index.ddb_client.batch_write_item.assert_not_called()

    def test_update_item_bundle_item_key_invalid_returns_414_error(self):
        # Arrange
        event = self.get_lambda_event()
        event['pathParameters'] = {'bundle_name': 'TESTBundle', 'bundle_item_key': 'x' * 256}

        # Act
        result = index.lambda_handler(event, None)

        # Assert
        self.assertEqual(414, result['statusCode'])
        index.ddb_client.batch_write_item.assert_not_called()

    def test_update_item_bundle_item_key_is_missing_returns_400_error(self):
        # Arrange
        event = self.get_lambda_event()
        event['body'] = '{}'

        # Act
        result = index.lambda_handler(event, None)

        # Assert
        self.assertEqual(400, result['statusCode'])
        index.ddb_client.batch_write_item.assert_not_called()

    def test_update_item_bundle_item_key_is_invalid_returns_400_error(self):
        # Arrange
        event = self.get_lambda_event()
        event['body'] = '{"bundle_item_value": "<script></script>"}'

        # Act
        result = index.lambda_handler(event, None)

        # Assert
        self.assertEqual(400, result['statusCode'])
        index.ddb_client.batch_write_item.assert_not_called()

    def test_update_item_bundle_item_not_found_returns_404_error(self):
        # Arrange
        event = self.get_lambda_event()
        index.ddb_client.exceptions.ConditionalCheckFailedException = MockConditionalCheckFailedException
        index.ddb_client.update_item.side_effect = index.ddb_client.exceptions.ConditionalCheckFailedException()

        # Act
        result = index.lambda_handler(event, None)

        # Assert
        self.assertEqual(404, result['statusCode'])
        index.ddb_client.batch_write_item.assert_not_called()

    @patch('functions.usergamedata.UpdateItem.index.datetime')
    def test_update_item_bundle_returns_success(self, mock_datetime: MagicMock):
        # Arrange
        event = self.get_lambda_event()
        mock_datetime.utcnow.return_value = datetime(2021, 8, 4, 1, 23, 34, 56)

        result = index.lambda_handler(event, None)

        calls = [
            call(ExpressionAttributeNames={'#bundle_item_value': 'bundle_item_value',
                                           '#updated_at': 'updated_at'},
                 ExpressionAttributeValues={':bundle_item_value': {'S': 'Banana'},
                                            ':updated_at': {'S': '2021-08-04T01:23:34.000056+00:00'}},
                 Key={'player_id_bundle': {'S': 'test_gamekit_player_id_BANANA_BUNDLE'}, 'bundle_item_key': {'S': 'MAX_BANANAS'}},
                 ReturnValues='UPDATED_NEW',
                 TableName=ITEMS_TABLE_NAME,
                 ConditionExpression= 'attribute_exists(player_id_bundle) and attribute_exists(bundle_item_key)',
                 UpdateExpression='SET #bundle_item_value = :bundle_item_value, '
                                  '#updated_at = :updated_at')
        ]
        self.assertEqual(result['statusCode'], 204)
        index.ddb_client.update_item.assert_has_calls(calls, any_order=False)

    @staticmethod
    def get_lambda_event():
        return {
            'resource': '/usergamedata/{bundle_name}/{bundle_item_id}',
            'path': '/usergamedata/BANANA_BUNDLE/MAX_BANANAS',
            'httpMethod': 'PUT',
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
                'bundle_name': 'BANANA_BUNDLE',
                'bundle_item_key': 'MAX_BANANAS'
            },
            'stageVariables': None,
            'requestContext': {
                'resourceId': 'abcdef',
                'authorizer': {
                    'claims': {
                        'sub': 'test_gamekit_player_id',
                        'iss': 'https://cognito-idp.us-west-2.amazonaws.com/us-west-2_123456789',
                        'cognito:username': 'jakschic',
                        'origin_jti': 'test_gamekit_player_id',
                        'aud': '7s24tlabcn8n0defbfoghijsgn',
                        'event_id': '6234d920-b637-4cdf-bd44-3a5e53f51569',
                        'token_use': 'id',
                        'auth_time': '1627438909',
                        'custom:gk_user_id': 'test_gamekit_player_id',
                        'exp': 'Wed Jul 28 03:21:49 UTC 2021',
                        'iat': 'Wed Jul 28 02:21:49 UTC 2021',
                        'jti': '7s24tlabcn8n0defbfoghijsgn',
                        'email': 'xyz@abc.def'
                    }
                },
                'domainName': 'abcdefghij.execute-api.us-west-2.amazonaws.com',
                'apiId': 'abcdefghij'
            },
            'body': '{"bundle_item_value": "Banana"}',
            'isBase64Encoded': False
        }
