# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

import json
import os
from unittest import TestCase
from unittest.mock import patch, MagicMock

with patch.dict(os.environ, {'ACHIEVEMENTS_TABLE_NAME': 'gamekit_dev_foogamename_game_achievements'}) as env_mock:
    with patch("gamekithelpers.ddb.get_table") as layer_boto_mock:
        from functions.achievements.AdminGetAchievements import index


class TestIndex(TestCase):
    @patch('functions.achievements.AdminGetAchievements.index.ddb.boto3')
    def setUp(self, mock_boto3: MagicMock):
        index.ddb_table = mock_boto3.resource('dynamodb').Table('test_table')

    def test_lambda_returns_a_200_success_code_using_defaults_when_query_string_is_empty(self):
        # Arrange
        event = self.get_lambda_event()
        event['queryStringParameters'] = None
        index.ddb_table.scan.return_value = self.mocked_scan_result()

        # Act
        result = index.lambda_handler(event, None)

        # Assert
        index.ddb_table.scan.assert_called_once()
        self.assertEqual(200, result['statusCode'])
        achievements = json.loads(result['body']).get('data').get('achievements')
        self.assertIsNotNone(achievements)
        self.assertEqual(1, len(achievements))

    def test_lambda_returns_a_200_success_code_when_query_string_passed(self):
        event = self.get_lambda_event()
        index.ddb_table.scan.return_value = self.mocked_scan_result()

        # Act
        result = index.lambda_handler(event, None)

        # Assert
        index.ddb_table.scan.assert_called_once()
        self.assertEqual(200, result['statusCode'])
        results_body = json.loads(result['body'])
        achievements = results_body.get('data').get('achievements')
        next_start_key = results_body.get('next_start_key')
        self.assertIsNotNone(achievements)
        self.assertIsNone(next_start_key)
        self.assertEqual(1, len(achievements))

    def test_lambda_returns_a_200_success_code_when_there_are_more_pages(self):
        event = self.get_lambda_event()
        index.ddb_table.scan.return_value = self.mocked_scan_result()
        index.ddb_table.scan.return_value['LastEvaluatedKey'] = {'achievement_id': 'NEXT_ACHIEVEMENT'}

        # Act
        result = index.lambda_handler(event, None)

        # Assert
        index.ddb_table.scan.assert_called_once()
        self.assertEqual(200, result['statusCode'])
        results_body = json.loads(result['body'])
        achievements = results_body.get('data').get('achievements')
        next_start_key = results_body.get('paging').get('next_start_key')
        self.assertIsNotNone(achievements)
        self.assertIsNotNone(next_start_key)
        self.assertEqual(1, len(achievements))
        self.assertEqual('NEXT_ACHIEVEMENT', next_start_key['achievement_id'])

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
            'queryStringParameters': {'limit': '50', 'use_consistent_read': 'true'},
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
            'body': None,
            'isBase64Encoded': False
        }

    @staticmethod
    def mocked_scan_result():
        return {
            'Items': [
                {
                    'created_at': '2021-07-27T00:38:42.927905+00:00',
                    'locked_description': 'Eat 1,000 bananas',
                    'achievement_id': 'EAT_THOUSAND_BANANAS',
                    'unlocked_icon_url': 'https://mygame.cloudfront.net/achievements/icon/1_unlocked.png',
                    'max_value': 1001,
                    'unlocked_description': 'You ate 1,000 bananas!',
                    'is_secret': False,
                    'locked_icon_url': 'https://mygame.cloudfront.net/achievements/icon/1_locked.png',
                    'updated_at': '2021-07-27T16:54:29.130692+00:00',
                    'is_stateful': True,
                    'points': 10,
                    'order_number': 1,
                    'is_hidden': False,
                    'title': 'Hangry Chicken'
                }
            ],
            'Count': 1,
            'ScannedCount': 1
        }
