# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

import json
import os
from unittest import TestCase
from unittest.mock import patch, MagicMock

with patch.dict(os.environ, {
    'ACHIEVEMENTS_TABLE_NAME': 'gamekit_dev_foogamename_game_achievements',
    'PLAYER_ACHIEVEMENTS_TABLE_NAME': 'gamekit_dev_foogamename_player_achievements'
    }) as env_mock:
    with patch("gamekithelpers.ddb.get_table") as layer_boto_mock:
        from functions.achievements.GetAchievement import index


class TestIndex(TestCase):
    @patch('functions.achievements.GetAchievement.index.ddb.boto3')
    def setUp(self, mock_boto3: MagicMock):
        index.ddb_game_table = mock_boto3.resource('dynamodb').Table('test_table')
        index.ddb_player_table = mock_boto3.resource('dynamodb').Table('test_player_table')

    def test_lambda_returns_a_400_error_code_when_path_parameters_is_empty(self):
        # Arrange
        event = self.get_lambda_event()
        event['pathParameters'] = {}

        # Act
        result = index.lambda_handler(event, None)

        # Assert
        self.assertEqual(400, result['statusCode'])
        self.assert_did_not_call_dynamodb(index.ddb_game_table)

    def test_lambda_returns_a_400_error_code_when_achievement_id_is_empty(self):
        # Arrange
        event = self.get_lambda_event()
        event['pathParameters'] = {'achievement_id': None}

        # Act
        result = index.lambda_handler(event, None)

        # Assert
        self.assertEqual(400, result['statusCode'])
        self.assert_did_not_call_dynamodb(index.ddb_game_table)

    def test_lambda_returns_a_401_error_code_when_player_id_is_empty(self):
        # Arrange
        event = self.get_lambda_event()
        event['requestContext'] = {'authorizer': {'claims': {}}}

        # Act
        result = index.lambda_handler(event, None)

        # Assert
        self.assertEqual(401, result['statusCode'])
        self.assert_did_not_call_dynamodb(index.ddb_game_table)

    def test_lambda_returns_a_404_code_for_hidden_achievement(self):
        # Arrange
        event = self.get_lambda_event()
        index.ddb_game_table.get_item.return_value = self.mocked_get_hidden_achievement_result()

        # Act
        result = index.lambda_handler(event, None)

        # Assert
        self.assertEqual(404, result['statusCode'])

    @patch('functions.achievements.GetAchievement.index._get_player_achievement')
    def test_lambda_returns_a_404_code_for_secret_unearned_achievement(self, mock_get_player_achievement):
        # Arrange
        event = self.get_lambda_event()
        index.ddb_game_table.get_item.return_value = self.mocked_get_secret_achievement_result()
        mock_get_player_achievement.return_value = self.mocked_get_player_unearned_achievement_result()

        # Act
        result = index.lambda_handler(event, None)

        # Assert
        self.assertEqual(404, result['statusCode'])

    @patch('functions.achievements.GetAchievement.index._get_player_achievement')
    def test_lambda_returns_a_200_code_for_secret_earned_achievement(self, mock_get_player_achievement):
        # Arrange
        event = self.get_lambda_event()
        index.ddb_game_table.get_item.return_value = self.mocked_get_secret_achievement_result()
        mock_get_player_achievement.return_value = self.mocked_get_player_achievement_result()

        # Act
        result = index.lambda_handler(event, None)

        # Assert
        self.assertEqual(200, result['statusCode'])
        achievement = json.loads(result['body']).get('data')
        self.assertIsNotNone(achievement['earned'])
        self.assertTrue(achievement['earned'])
        self.assertIsNotNone(achievement['earned_at'])

    def test_lambda_returns_a_200_success_code_with_unearned_achievements(self):
        # Arrange
        event = self.get_lambda_event()
        index.ddb_game_table.get_item.return_value = self.mocked_get_achievement_result()

        # Act
        result = index.lambda_handler(event, None)

        # Assert
        self.assertEqual(200, result['statusCode'])

        achievement = json.loads(result['body']).get('data')
        self.assertIsNotNone(achievement['earned'])
        self.assertEqual(False, achievement['earned'])
        self.assertIsNone(achievement['earned_at'])

    @patch('functions.achievements.GetAchievement.index._get_player_achievement')
    def test_lambda_returns_a_200_success_code_with_earned_achievements(self, mock_get_player_achievement):
        # Arrange
        event = self.get_lambda_event()
        index.ddb_game_table.get_item.return_value = self.mocked_get_achievement_result()
        mock_get_player_achievement.return_value = self.mocked_get_player_achievement_result()

        # Act
        result = index.lambda_handler(event, None)

        # Assert
        index.ddb_player_table.get_item.assert_called_once()
        self.assertEqual(200, result['statusCode'])

        achievement = json.loads(result['body']).get('data')
        self.assertIsNotNone(achievement['earned'])
        self.assertEqual(True, achievement['earned'])
        self.assertIsNotNone(achievement['earned_at'])

    @staticmethod
    def get_lambda_event():
        return {
            'resource': '/achievements/{achievement_id}',
            'path': '/achievements/EAT_THOUSAND_BANANAS',
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
            'pathParameters': {
                'achievement_id': 'EAT_THOUSAND_BANANAS'
            },
            'stageVariables': None,
            'requestContext': {
                'resourceId': 'abcdef',
                'authorizer': {
                  'claims': {
                    'sub': '12345678-1234-1234-1234-123456789012',
                    'iss': 'https://cognito-idp.us-west-2.amazonaws.com/us-west-2_123456789',
                    'cognito:username': 'jrnic',
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

    @staticmethod
    def assert_did_not_call_dynamodb(mock_ddb_table):
        mock_ddb_table.scan.assert_not_called()
        mock_ddb_table.get_item.assert_not_called()

    @staticmethod
    def mocked_get_achievement_result():
        return {
            'Item': {
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
        }

    @staticmethod
    def mocked_get_hidden_achievement_result():
        return {
            'Item': {
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
                'is_hidden': True,
                'title': 'Hangry Chicken'
            }
        }

    @staticmethod
    def mocked_get_secret_achievement_result():
        return {
            'Item': {
                'created_at': '2021-07-27T00:38:42.927905+00:00',
                'locked_description': 'Eat 1,000 bananas',
                'achievement_id': 'EAT_THOUSAND_BANANAS',
                'unlocked_icon_url': 'https://mygame.cloudfront.net/achievements/icon/1_unlocked.png',
                'max_value': 1001,
                'unlocked_description': 'You ate 1,000 bananas!',
                'is_secret': True,
                'locked_icon_url': 'https://mygame.cloudfront.net/achievements/icon/1_locked.png',
                'updated_at': '2021-07-27T16:54:29.130692+00:00',
                'is_stateful': True,
                'points': 10,
                'order_number': 1,
                'is_hidden': False,
                'title': 'Hangry Chicken'
            }
        }

    @staticmethod
    def mocked_get_player_achievement_result():
        return {
            'updated_at': '2021-07-28T03:37:37.267711+00:00',
            'created_at': '2021-07-28T03:37:32.227830+00:00',
            'earned': True,
            'achievement_id': 'EAT_THOUSAND_BANANAS',
            'current_value': 5,
            'player_id': '12345678-1234-1234-1234-123456789012',
            'earned_at': '2021-07-28T03:37:37.267711+00:00'
        }

    @staticmethod
    def mocked_get_player_unearned_achievement_result():
        return {
            'updated_at': '2021-07-28T03:37:37.267711+00:00',
            'created_at': '2021-07-28T03:37:32.227830+00:00',
            'earned': False,
            'achievement_id': 'EAT_THOUSAND_BANANAS',
            'current_value': 5,
            'player_id': '12345678-1234-1234-1234-123456789012',
        }