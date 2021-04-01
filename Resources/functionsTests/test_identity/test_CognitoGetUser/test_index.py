# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

import os
from unittest import TestCase
from unittest.mock import patch, MagicMock

from functions.identity.CognitoGetUser import index


class TestIndex(TestCase):

    @patch('functions.identity.CognitoGetUser.index.ddb.boto3')
    def test_details_can_be_fetched_for_a_user_successfully(self, mock_boto3):
        # Arrange
        event = self.get_lambda_event()
        context = None
        mock_ddb_table = mock_boto3.resource('dynamodb').Table()
        mock_ddb_table.get_item.return_value = {
            'Item': {
                # Some of the desired fields:
                'gk_user_id': {
                    'S': 'foo_gk_user_id'
                },
                'created_at': {
                    'S': 'foo_created_at'
                },

                # Some of the filtered-out fields:
                'gk_user_hash_key': {
                    'S': 'foo_gk_user_hash_key'
                },
                'gk_user_id_hash': {
                    'S': 'foo_gk_user_id_hash'
                },
            },
        }

        # Act
        result = index.lambda_handler(event, context)

        # Assert
        self.assertEqual(200, result['statusCode'])
        self.assertIsNotNone(result['body'])

    def test_lambda_returns_a_401_error_code_when_player_id_is_missing_from_event(self):
        # Arrange
        event = self.get_lambda_event()
        context = None
        del event['requestContext']['authorizer']['claims']['custom:gk_user_id']

        # Act
        result = index.lambda_handler(event, context)

        # Assert
        self.assertEqual(401, result['statusCode'])

    @staticmethod
    def get_lambda_event():
        return {
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
            'headers': {
                'accesskey': 'foo_accesskey'
            }
        }
