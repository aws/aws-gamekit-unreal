# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

import json
import os
from unittest import TestCase
from unittest.mock import patch, MagicMock

from functions.achievements.AdminAddAchievements import index


# Patch Lambda environment variables:
@patch.dict(os.environ, {
    'ACHIEVEMENTS_TABLE_NAME': 'gamekit_dev_foogamename_game_achievements',
})
class TestIndex(TestCase):
    @patch('functions.achievements.AdminAddAchievements.index.ddb.boto3')
    def test_lambda_returns_a_400_error_code_when_body_is_empty(self, mock_boto3: MagicMock):
        # Arrange
        event = self.get_lambda_event()
        event['body'] = None
        mock_ddb_table = mock_boto3.resource('dynamodb').Table('test_table')

        # Act
        result = index.lambda_handler(event, None)

        # Assert
        self.assertEqual(400, result['statusCode'])
        mock_ddb_table.update_item.assert_not_called()

    @patch('functions.achievements.AdminAddAchievements.index.ddb.boto3')
    def test_lambda_returns_a_400_error_code_when_achievements_key_is_missing(self, mock_boto3: MagicMock):
        # Arrange
        event = self.get_lambda_event()
        event['body'] = '{}'
        mock_ddb_table = mock_boto3.resource('dynamodb').Table('test_table')

        # Act
        result = index.lambda_handler(event, None)

        # Assert
        self.assertEqual(400, result['statusCode'])
        mock_ddb_table.update_item.assert_not_called()

    @patch('functions.achievements.AdminAddAchievements.index.ddb.boto3')
    def test_lambda_returns_a_400_error_code_when_achievements_array_is_empty(self, mock_boto3: MagicMock):
        # Arrange
        event = self.get_lambda_event()
        event['body'] = '{"achievements": []}'
        mock_ddb_table = mock_boto3.resource('dynamodb').Table('test_table')

        # Act
        result = index.lambda_handler(event, None)

        # Assert
        self.assertEqual(400, result['statusCode'])
        mock_ddb_table.update_item.assert_not_called()

    @patch('functions.achievements.AdminAddAchievements.index.ddb.boto3')
    def test_lambda_returns_a_200_success_code_when_achievements_passed_in_body(self, mock_boto3: MagicMock):
        # Arrange
        event = self.get_lambda_event()
        mock_dynamodb = mock_boto3.client('dynamodb')
        mock_ddb_table = mock_boto3.resource('dynamodb').Table('test_table')
        mock_ddb_table.update_item.return_value = {
            'Attributes': {
                'created_at': '2021-07-27T00:38:42.927855+00:00',
                'locked_description': 'Eat 1,000 bananas',
                'achievement_id': 'EAT_THOUSAND_BANANAS',
                'unlocked_icon_url': 'https://mygame.cloudfront.net/achievements/icon/1_unlocked.png',
                'max_value': 1000,
                'unlocked_description': 'You ate 1,000 bananas!',
                'is_secret': False,
                'locked_icon_url': 'https://mygame.cloudfront.net/achievements/icon/1_locked.png',
                'updated_at': '2021-07-27T17:25:39.302081+00:00',
                'is_stateful': True,
                'points': 10,
                'order_number': 1,
                'is_hidden': False,
                'title': 'Hangry Chicken'
            }
        }

        # Act
        result = index.lambda_handler(event, None)

        # Assert
        self.assertEqual(200, result['statusCode'])
        achievements = json.loads(result['body']).get('data').get('achievements')
        self.assertIsNotNone(achievements)
        self.assertEqual(1, len(achievements))
        mock_ddb_table.update_item.assert_called_once()

    @staticmethod
    def get_lambda_event():
        return {
            'resource': '/achievements/admin',
            'path': '/achievements/admin/',
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
            'body': '{"achievements": [{"achievement_id": "EAT_THOUSAND_BANANAS","title": "Hangry Chicken","locked_description": "Eat 1,000 bananas", "unlocked_description": "You ate 1,000 bananas!", "locked_icon_url": "https://mygame.cloudfront.net/achievements/icon/1_locked.png", "unlocked_icon_url": "https://mygame.cloudfront.net/achievements/icon/1_unlocked.png", "points": 10, "is_stateful": true, "max_value": 1000, "is_secret": false, "is_hidden": false, "order_number": 1 }]}',
            'isBase64Encoded': False
        }
