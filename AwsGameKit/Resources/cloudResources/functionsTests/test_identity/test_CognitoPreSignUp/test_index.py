# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

from unittest import TestCase
from unittest.mock import patch, MagicMock

with patch("boto3.client") as boto_client_mock:
    from functions.identity.CognitoPreSignUp import index


class TestIndex(TestCase):
    def setUp(self):
        index.cidp_client = MagicMock()

    @patch('functions.identity.CognitoPreSignUp.index.ddb.boto3')
    def test_new_user_can_be_registered_by_email_successfully(self, mock_gamekithelpers_ddb_boto3):
        # Arrange
        event = self.get_lambda_event()
        context = None
        index.cidp_client.list_users.return_value = {
            'Users': [
                # 'Users' is empty, which means the user's email is not being used in the Cognito User Pool yet.
            ]
        }

        mock_ddb_table = mock_gamekithelpers_ddb_boto3.resource('dynamodb').Table()
        mock_ddb_table.get_item.return_value = {
            # The result is empty, which means the user has not been registered in the "Identities" table yet.
        }

        # Act
        result = index.lambda_handler(event, context)

        # Assert
        mock_ddb_table.put_item.assert_called_once()
        self.assertFalse(result['response']['autoConfirmUser'])

    @patch('functions.identity.CognitoPreSignUp.index.ddb.boto3')
    def test_user_does_not_get_registered_when_lambda_is_invoked_by_an_external_provider(self, mock_gamekithelpers_ddb_boto3: MagicMock):
        # Arrange
        event = self.get_lambda_event()
        event['triggerSource'] = 'PreSignUp_ExternalProvider'
        context = None

        # Act
        index.lambda_handler(event, context)

        # Assert
        mock_ddb_table = mock_gamekithelpers_ddb_boto3.resource('dynamodb').Table()
        mock_ddb_table.put_item.assert_not_called()

    @patch('functions.identity.CognitoPreSignUp.index.ddb.boto3')
    def test_user_does_not_get_registered_when_gk_user_id_is_not_a_valid_uuidv4(self, mock_gamekithelpers_ddb_boto3: MagicMock):
        # Arrange
        event = self.get_lambda_event()
        event['request']['userAttributes']['custom:gk_user_id'] = 'Not a valid UUIDv4 string.'
        context = None

        # Act
        with self.assertRaises(ValueError):
            index.lambda_handler(event, context)

        # Assert
        mock_ddb_table = mock_gamekithelpers_ddb_boto3.resource('dynamodb').Table()
        mock_ddb_table.put_item.assert_not_called()
        self.assertFalse(event['response']['autoConfirmUser'])
        self.assertFalse(event['response']['autoVerifyEmail'])

    @patch('functions.identity.CognitoPreSignUp.index.ddb.boto3')
    def test_user_does_not_get_registered_when_email_already_exists_in_cognito_user_pool(self, mock_gamekithelpers_ddb_boto3):
        # Arrange
        event = self.get_lambda_event()
        context = None
        index.cidp_client.list_users.return_value = {
            'Users': [
                # 'Users' is non-empty, which means the user's email already exists in the Cognito User Pool:
                {}
            ]
        }

        # Act
        with self.assertRaises(ValueError):
            index.lambda_handler(event, context)

        # Assert
        mock_ddb_table = mock_gamekithelpers_ddb_boto3.resource('dynamodb').Table()
        mock_ddb_table.put_item.assert_not_called()
        self.assertFalse(event['response']['autoConfirmUser'])
        self.assertFalse(event['response']['autoVerifyEmail'])

    @patch('functions.identity.CognitoPreSignUp.index.ddb.boto3')
    def test_user_does_not_get_registered_when_user_is_already_in_the_identities_table(self, mock_gamekithelpers_ddb_boto3):
        # Arrange
        event = self.get_lambda_event()
        context = None
        index.cidp_client.list_users.return_value = {
            'Users': [
                # 'Users' is empty, which means the user's email is not being used in the Cognito User Pool yet.
            ]
        }

        mock_ddb_table = mock_gamekithelpers_ddb_boto3.resource('dynamodb').Table()
        mock_ddb_table.get_item.return_value = {
            # An item is returned, which means the user has already been registered in the "Identities" table:
            'Item': {}
        }

        # Act
        index.lambda_handler(event, context)

        # Assert
        mock_ddb_table.put_item.assert_not_called()
        self.assertFalse(event['response']['autoConfirmUser'])
        self.assertFalse(event['response']['autoVerifyEmail'])

    @staticmethod
    def get_lambda_event():
        return {
            'version': '1',
            'region': 'us-west-2',
            'userPoolId': 'us-west-2_123abcdef',
            'userName': 'fooName',
            'callerContext': {
                'awsSdkVersion': 'aws-sdk-cpp-1.9.32',
                'clientId': '1234567890abcdefghijklmnop'
            },
            'triggerSource': 'PreSignUp_SignUp',
            'request': {
                'userAttributes': {
                    'custom:gk_user_hash_key': 'TXlA1ZWBCawlVX7nNqKTKgNTTzY6qr8La6ycrx224B8=',
                    'custom:gk_user_id': '8674e4ba-91e4-41d8-9f6f-a51535257ba4',
                    'email': 'foo@bar.com'
                },
                'validationData': None
            },
            'response': {
                'autoConfirmUser': False,
                'autoVerifyEmail': False,
                'autoVerifyPhone': False
            }
        }
