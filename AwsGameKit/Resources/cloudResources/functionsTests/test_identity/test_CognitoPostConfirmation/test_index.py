# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

import os
from unittest import TestCase
from unittest.mock import patch, MagicMock, Mock
import boto3

with patch("boto3.client") as boto_client_mock:
    from functions.identity.CognitoPostConfirmation import index
from functionsTests.helpers.boto3.mock_responses.exceptions import new_boto_exception


class TestIndex(TestCase):
    def setUp(self):
        index.dynamodb_client = MagicMock()

    @patch.dict(os.environ, {'AWS_DEFAULT_REGION': 'us-west-2'})
    @patch('functions.identity.CognitoPostConfirmation.index.ddb.boto3')
    def test_record_can_be_updated_successfully(self, mock_boto3: MagicMock):
        # Arrange
        event = self.get_lambda_event()
        context = None

        # Act
        index.lambda_handler(event, context)

        # Assert
        mock_ddb_table = mock_boto3.resource('dynamodb').Table()
        mock_ddb_table.update_item.assert_called_once()

    def test_record_is_not_updated_when_lambda_is_invoked_by_an_external_provider(self):
        # Arrange
        event = self.get_lambda_event()
        event['request']['userAttributes']['cognito:user_status'] = 'EXTERNAL_PROVIDER'
        context = None

        # Act
        index.lambda_handler(event, context)

        # Assert
        mock_ddb_table = index.dynamodb_client.Table()
        mock_ddb_table.update_item.assert_not_called()

    # Patch the default AWS region so the 'real_ddb_client' can be created on systems where
    # the AWS credentials file and AWS configuration file are missing.
    # See: https://boto3.amazonaws.com/v1/documentation/api/latest/guide/quickstart.html#configuration
    @patch.dict(os.environ, {'AWS_DEFAULT_REGION': 'us-west-2'})
    @patch('functions.identity.CognitoPostConfirmation.index.ddb.boto3')
    def test_record_is_not_updated_when_hash_does_not_match_existing_record(self, mock_boto3: MagicMock):
        # Arrange
        event = self.get_lambda_event()
        context = None

        # Create a real DDB client for exception assertion
        # This simulates DDB's response when the 'gk_user_id_hash' in the request doesn't match what's in the table:
        real_ddb_client = boto3.client('dynamodb')
        mock_ddb_table = mock_boto3.resource('dynamodb').Table('test_table')
        index.dynamodb_client.exceptions.ConditionalCheckFailedException = real_ddb_client.exceptions.ConditionalCheckFailedException
        mock_ddb_table.update_item = Mock(
            side_effect=new_boto_exception(real_ddb_client.exceptions.ConditionalCheckFailedException)
        )

        # Act/Assert
        with self.assertRaises(real_ddb_client.exceptions.ConditionalCheckFailedException):
            index.lambda_handler(event, context)

    @staticmethod
    def get_lambda_event():
        return {
            'version': '1',
            'region': 'us-west-2',
            'userPoolId': 'us-west-2_123abcdef',
            'userName': 'fooName',
            'callerContext': {
                'awsSdkVersion': 'aws-sdk-cpp-1.9.44',
                'clientId': '1234567890abcdefghijklmnop'
            },
            'triggerSource': 'PostConfirmation_ConfirmSignUp',
            'request': {
                'userAttributes': {
                    'sub': 'a1bae719-631f-42e8-828b-6b51475443bc',
                    'email_verified': 'true',
                    'cognito:user_status': 'CONFIRMED',
                    'custom:gk_user_hash_key': 'TXlA1ZWBCawlVX7nNqKTKgNTTzY6qr8La6ycrx224B8=',
                    'custom:gk_user_id': '8674e4ba-91e4-41d8-9f6f-a51535257ba4',
                    'email': 'foo@bar.com'
                }
            },
            'response': {
            }
        }
