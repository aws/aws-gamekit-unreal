# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

import os
from unittest import TestCase
from unittest.mock import patch, MagicMock, Mock

import botocore.exceptions

with patch("boto3.resource") as boto_resource_mock:
    from functions.gamesaving.DeleteSaveSlot import index

from functionsTests.helpers.sample_lambda_events import http_event
from functionsTests.helpers.boto3.mock_responses.exceptions import new_boto_exception


# Patch Lambda environment variables:
@patch.dict(os.environ, {
    'GAMESAVES_TABLE_NAME': 'gamekit_dev_foogamename_player_gamesaves',
    'GAMESAVES_BUCKET_NAME': 'gamekit-dev-123456789012-foogamename-player-gamesaves',
    'AWS_ACCOUNT_ID': '123456789012'
})
class TestIndex(TestCase):
    def setUp(self):
        index.s3_resource = MagicMock()

    @patch('gamekithelpers.ddb.get_table')
    def test_can_delete_save_slot_successfully(self, mock_get_table: MagicMock):
        # Arrange
        event = self.get_lambda_event()
        context = None
        # Note: the mock objects are automatically used by lambda_handler().
        # We don't need to configure them for this test case.

        # Act
        result = index.lambda_handler(event, context)

        # Assert
        self.assertEqual(204, result['statusCode'])

    def test_lambda_returns_a_401_error_code_when_player_id_is_missing_from_the_request_context(self):
        # Arrange
        event = self.get_lambda_event()
        event['requestContext']['authorizer']['claims'].pop('custom:gk_user_id')
        context = None

        # Act
        result = index.lambda_handler(event, context)

        # Assert
        self.assertEqual(401, result['statusCode'])

    @patch('gamekithelpers.ddb.get_table')
    def test_lambda_returns_a_403_error_code_when_s3_bucket_owner_mismatch(self, mock_get_table: MagicMock):
        # Arrange
        event = self.get_lambda_event()
        context = None

        mock_s3_object = index.s3_resource.Object('test_object', 'test_key')
        mock_s3_object.delete = Mock(
            side_effect=self.get_s3_access_denied_exception()
        )

        # Act
        result = index.lambda_handler(event, context)

        # Assert
        self.assertEqual(403, result['statusCode'])

    @patch('gamekithelpers.ddb.get_table')
    def test_lambda_returns_a_400_error_code_when_slot_name_is_malformed(self, mock_get_table: MagicMock):
        # Arrange
        event = self.get_lambda_event()
        event['pathParameters']['slot_name'] = '$om3 ma!f*rm#d slot name%^z__09'
        context = None

        # Act
        result = index.lambda_handler(event, context)

        # Assert
        self.assertEqual(400, result['statusCode'])

    @staticmethod
    def get_lambda_event() -> dict:
        """Get a sample Lambda event with none of the optional parameters included."""
        return http_event(
            path_parameters={
                'slot_name': 'foo_slot_name'
            }
        )

    @staticmethod
    def get_s3_access_denied_exception():
        exception = new_boto_exception(botocore.exceptions.ClientError)
        exception.__dict__['response'] = {
            'Error': {
                'Code': 'AccessDenied',
                'Message': 'Access Denied'
            }
        }
        return exception
