# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

import os
from unittest import TestCase
from unittest.mock import patch, MagicMock, call

with patch("boto3.resource") as boto_resource_mock:
    with patch("boto3.client") as boto_client_mock:
        from functions.usergamedata.DeleteAll import index

BUNDLES_TABLE_NAME = 'test_bundles_table'
ITEMS_TABLE_NAME = 'test_bundleitems_table'
BATCH_DELETE_HELPER_LAMBDA_NAME = "test_lambda_arn"


# Patch Lambda environment variables:
@patch.dict(os.environ, {
    'BUNDLES_TABLE_NAME': BUNDLES_TABLE_NAME,
    'BUNDLE_ITEMS_TABLE_NAME': ITEMS_TABLE_NAME,
    'BATCH_DELETE_HELPER_LAMBDA_NAME': BATCH_DELETE_HELPER_LAMBDA_NAME
})
class TestDeleteAll(TestCase):
    def setUp(self):
        index.ddb_resource = MagicMock()
        index.lambda_client = MagicMock()

    def test_delete_all_invalid_player_returns_401_error(self):
        # Arrange
        event = self.get_lambda_event()
        event['requestContext'] = {'authorizer': {'claims': {}}}

        # Act
        result = index.lambda_handler(event, None)

        # Assert
        self.assertEqual(401, result['statusCode'])
        index.ddb_resource.batch_write_item.assert_not_called()

    def test_delete_all_player_has_no_bundles_returns_204_error(self):
        # Arrange
        event = self.get_lambda_event()
        index.ddb_resource.Table().query.side_effect = [{'Items': []}]

        # Act
        result = index.lambda_handler(event, None)

        # Assert
        self.assertEqual(204, result['statusCode'])

    def test_delete_all_player_has_data_success(self):
        # Arrange
        event = self.get_lambda_event()
        index.ddb_resource.Table().query.side_effect = [
            {'Items': [{'player_id': '12345', 'bundle_name': 'TestBundle'}]},
            {'Items': [{'player_id_bundle': '12345_TestBundle', 'bundle_item_key': 'Key'}]}, {'Items': []},
            {'Items': []}]

        # Act
        result = index.lambda_handler(event, None)

        # Assert
        self.assertEqual(204, result['statusCode'])
        calls = [call(FunctionName=BATCH_DELETE_HELPER_LAMBDA_NAME, InvocationType='Event',
                      Payload='{"TableName": "test_bundleitems_table", "DeleteRequest": [{"DeleteRequest": {"Key": {'
                              '"player_id_bundle": "12345_TestBundle", "bundle_item_key": "Key"}}}]}'),
                 call(FunctionName=BATCH_DELETE_HELPER_LAMBDA_NAME, InvocationType='Event',
                      Payload='{"TableName": "test_bundles_table", "DeleteRequest": [{'
                              '"DeleteRequest": {"Key": {"player_id": "12345", "bundle_name": "TestBundle"}}}]}')]

        index.lambda_client.invoke.assert_has_calls(calls, any_order=False)

    @staticmethod
    def get_lambda_event():
        return {
            'resource': '/usergamedata',
            'path': '/usergamedata/',
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
            'pathParameters': None,
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