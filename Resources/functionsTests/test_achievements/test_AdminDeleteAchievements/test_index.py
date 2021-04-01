# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

import os
from unittest import TestCase
from unittest.mock import patch, MagicMock

with patch("boto3.client") as boto_client_mock:
    from functions.achievements.AdminDeleteAchievements import index


# Patch Lambda environment variables:
@patch.dict(os.environ, {
    'ACHIEVEMENTS_TABLE_NAME': 'gamekit_dev_foogamename_game_achievements',
})
class TestIndex(TestCase):
    def setUp(self):
        index.ddb_client = MagicMock()

    def test_lambda_returns_a_400_error_code_when_body_is_empty(self):
        # Arrange
        event = self.get_lambda_event()
        event['body'] = None

        # Act
        result = index.lambda_handler(event, None)

        # Assert
        self.assertEqual(400, result['statusCode'])
        index.ddb_client.batch_write_item.assert_not_called()

    def test_lambda_returns_a_400_error_code_when_achievements_id_key_is_missing(self):
        # Arrange
        event = self.get_lambda_event()
        event['body'] = '{}'

        # Act
        result = index.lambda_handler(event, None)

        # Assert
        self.assertEqual(400, result['statusCode'])
        index.ddb_client.batch_write_item.assert_not_called()

    def test_lambda_returns_a_400_error_code_when_achievement_ids_array_is_empty(self):
        # Arrange
        event = self.get_lambda_event()
        event['body'] = '{"achievement_ids": []}'

        # Act
        result = index.lambda_handler(event, None)

        # Assert
        self.assertEqual(400, result['statusCode'])
        index.ddb_client.batch_write_item.assert_not_called()

    def test_lambda_returns_a_200_success_code_when_achievement_ids_passed_in_body(self):
        # Arrange
        event = self.get_lambda_event()

        # Act
        result = index.lambda_handler(event, None)

        # Assert
        self.assertEqual(204, result['statusCode'])
        index.ddb_client.batch_write_item.assert_called_once()

    @staticmethod
    def get_lambda_event():
        return {
            'resource': '/achievements/admin',
            'path': '/achievement/admins/',
            'httpMethod': 'POST',
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
                'resourcePath': '/achievements/admin',
                'httpMethod': 'POST', 
                'extendedRequestId': 'DI4AfFpavHcFdkg=', 
                'requestTime': '27/Jul/2021:16:53:22 +0000', 
                'path': '/dev/achievements/admin/',
                'accountId': '012345678901',
                'protocol': 'HTTP/1.1', 
                'stage': 'dev', 
                'domainPrefix': 'abcdefghij',
                'requestTimeEpoch': 1627404802977, 
                'requestId': 'b3fd487b-8617-49f0-81d8-f0cda11af1fb', 
                'identity': {
                    'cognitoIdentityPoolId': None, 
                    'accountId': None, 
                    'cognitoIdentityId': None, 
                    'caller': None, 
                    'sourceIp': '127.0.0.1',
                    'principalOrgId': None, 
                    'accessKey': None, 
                    'cognitoAuthenticationType': None, 
                    'cognitoAuthenticationProvider': None, 
                    'userArn': None, 
                    'userAgent': 'TestAgent',
                    'user': None
                }, 
                'domainName': 'abcdefghij.execute-api.us-west-2.amazonaws.com',
                'apiId': 'abcdefghij'
            }, 
            'body': '{"achievement_ids": ["EAT_THOUSAND_BANANAS"]}',
            'isBase64Encoded': False
        }
