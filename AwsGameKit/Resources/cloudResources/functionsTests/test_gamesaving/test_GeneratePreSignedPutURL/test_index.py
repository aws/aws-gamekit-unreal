# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0
import base64
import os
from unittest import TestCase
from unittest.mock import patch, MagicMock

with patch("boto3.client") as boto_client_mock:
    from functions.gamesaving.GeneratePreSignedPutURL import index

from functionsTests.helpers.boto3.mock_responses.DynamoDB import Paginator
from functionsTests.helpers.boto3.mock_responses.DynamoDB.Table import GetItem
from functionsTests.helpers.sample_lambda_events import http_event


MAX_SAVE_SLOTS_PER_PLAYER_FOR_TESTING = 2
MAX_METADATA_UNENCODED_BYTES = 1413
BASE_64_ENCODED_SHA_256_HASH = 'msIZfZJYJXsa6EY+QhTkzQpXi8FRfyQVkouRvkKD/Eg='


# Patch Lambda environment variables:
@patch.dict(os.environ, {
    'GAMESAVES_TABLE_NAME': 'gamekit_dev_foogamename_player_gamesaves',
    'GAMESAVES_BUCKET_NAME': 'gamekit-dev-123456789012-foogamename-player-gamesaves',
    'MAX_SAVE_SLOTS_PER_PLAYER': str(MAX_SAVE_SLOTS_PER_PLAYER_FOR_TESTING)
})
class TestIndex(TestCase):
    def setUp(self):
        index.s3_client = MagicMock()
        index.ddb_client = MagicMock()

    @patch('gamekithelpers.ddb.get_table')
    def test_can_generate_presigned_url_for_new_save_slot(self, mock_get_table: MagicMock):
        # Arrange
        event = self.get_lambda_event()
        context = None
        self.set_is_new_save_slot(True, mock_get_table)
        self.set_would_exceed_slot_limit(False, index.ddb_client)
        self.set_presigned_url('foo_url', index.s3_client)

        # Act
        result = index.lambda_handler(event, context)

        # Assert
        self.assertEqual(200, result['statusCode'])

    @patch('gamekithelpers.ddb.get_table')
    def test_can_generate_presigned_url_for_exising_save_slot(self, mock_get_table: MagicMock):
        # Arrange
        event = self.get_lambda_event()
        context = None
        self.set_is_new_save_slot(False, mock_get_table)
        self.set_presigned_url('foo_url', index.s3_client)

        # Act
        result = index.lambda_handler(event, context)

        # Assert
        self.assertEqual(200, result['statusCode'])

    @patch('gamekithelpers.ddb.get_table')
    def test_can_generate_presigned_url_with_default_parameters(self, mock_get_table: MagicMock):
        # Arrange
        event = self.get_lambda_event()
        context = None
        self.set_is_new_save_slot(True, mock_get_table)
        self.set_would_exceed_slot_limit(False, index.ddb_client)
        self.set_presigned_url('foo_url', index.s3_client)

        # Act
        result = index.lambda_handler(event, context)

        # Assert
        self.assertEqual(200, result['statusCode'])

    @patch('gamekithelpers.ddb.get_table')
    def test_can_generate_presigned_url_with_overridden_default_parameters(self, mock_get_table: MagicMock):
        # Arrange
        event = self.get_lambda_event()
        event['headers']['metadata'] = self.get_base64_string()
        event['queryStringParameters'] = {
            'time_to_live': '600',
            'consistent_read': 'False',
        }
        context = None
        self.set_is_new_save_slot(True, mock_get_table)
        self.set_would_exceed_slot_limit(False, index.ddb_client)
        self.set_presigned_url('foo_url', index.s3_client)

        # Act
        result = index.lambda_handler(event, context)

        # Assert
        self.assertEqual(200, result['statusCode'])

    @patch('gamekithelpers.ddb.get_table')
    def test_lambda_returns_a_400_error_code_when_generating_presigned_url_with_non_base_64_metadata(self, mock_get_table: MagicMock):
        # Arrange
        event = self.get_lambda_event()
        event['headers']['metadata'] = '{"description": "你好世界"}'
        context = None
        self.set_is_new_save_slot(True, mock_get_table)
        self.set_would_exceed_slot_limit(False, index.ddb_client)
        self.set_presigned_url('foo_url', index.s3_client)

        # Act
        result = index.lambda_handler(event, context)

        # Assert
        self.assertEqual(400, result['statusCode'])

    @patch('gamekithelpers.ddb.get_table')
    def test_lambda_returns_a_400_error_code_when_the_sha_256_hash_is_malformed(self, mock_get_table: MagicMock):
        sub_tests = [
            ('malformed SHA-256', '', 400),
            ('malformed SHA-256 - not long enough', 'asdfjh12314', 400),
            ('malformed SHA-256 - not base64 encoded', 'msIZfZJYJXsa6EY+QhTkzQpXi8FRfyQVkouRvkKD你好世界', 400),
            ('malformed SHA-256 - too long', 'a' * (index.BASE_64_ENCODED_SHA_256_BYTES + 1), 400),
            ('normal SHA-256', BASE_64_ENCODED_SHA_256_HASH, 200)
        ]

        for test_name, sha_hash, expected_http_status_code in sub_tests:
            with self.subTest(test_name):
                # Arrange
                event = self.get_lambda_event()
                event['headers']['hash'] = sha_hash
                context = None
                self.set_is_new_save_slot(True, mock_get_table)
                self.set_would_exceed_slot_limit(False, index.ddb_client)
                self.set_presigned_url('foo_url', index.s3_client)

                # Act
                result = index.lambda_handler(event, context)

                # Assert
                self.assertEqual(expected_http_status_code, result['statusCode'])

    @patch('gamekithelpers.ddb.get_table')
    def test_lambda_returns_a_400_error_code_when_the_metadata_parameter_exceeds_the_limit(self, mock_get_table: MagicMock):
        sub_tests = [
            ('exceeding the limit', 'a', MAX_METADATA_UNENCODED_BYTES + 1, 400),  # 1 byte over after encoding
            ('at the limit', 'a', MAX_METADATA_UNENCODED_BYTES, 200),  # 3 bytes less after encoding
            ('less than the limit', 'a', MAX_METADATA_UNENCODED_BYTES - 7, 200),  # 7 bytes less after encoding
        ]

        for test_name, character, number_of_bytes, expected_http_status_code in sub_tests:
            with self.subTest(test_name):
                # Arrange
                event = self.get_lambda_event()
                context = None
                self.set_is_new_save_slot(True, mock_get_table)
                self.set_would_exceed_slot_limit(False, index.ddb_client)
                self.set_presigned_url('foo_url', index.s3_client)

                event['headers']['metadata'] = self.get_base64_string(number_of_bytes)

                # Act
                result = index.lambda_handler(event, context)

                # Assert
                self.assertEqual(expected_http_status_code, result['statusCode'])

    @patch('gamekithelpers.ddb.get_table')
    def test_lambda_returns_a_400_error_code_when_the_slot_limit_would_be_exceeded(self, mock_get_table: MagicMock):
        # Arrange
        event = self.get_lambda_event()
        context = None
        self.set_is_new_save_slot(True, mock_get_table)
        self.set_would_exceed_slot_limit(True, index.ddb_client)

        # Act
        result = index.lambda_handler(event, context)

        # Assert
        self.assertEqual(400, result['statusCode'])

    def test_lambda_returns_a_401_error_code_when_player_id_is_missing_from_the_request_context(self):
        # Arrange
        event = self.get_lambda_event()
        event['requestContext']['authorizer']['claims'].pop('custom:gk_user_id')
        context = None

        # Act
        result = index.lambda_handler(event, context)

        # Assert
        self.assertEqual(401, result['statusCode'])

    def test_lambda_returns_a_400_error_code_when_slot_name_is_malformed(self):
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
            },
            headers={
                'hash': BASE_64_ENCODED_SHA_256_HASH,
                'last_modified_epoch_time': '123'
            }
        )

    @staticmethod
    def set_is_new_save_slot(is_new_save_slot: bool, mock_get_table: MagicMock) -> None:
        item = {
            'player_id': 'foo_player_id',
            'slot_name': 'foo_slot_name',
            'last_modified': 1626924349000,
            'size': 123456,
            'metadata': '{"description": "foo_description"}'
        }
        if is_new_save_slot:
            item = None

        mock_get_table().get_item.return_value = GetItem.mock_response(item=item)

    @staticmethod
    def set_would_exceed_slot_limit(would_exceed_slot_limit: bool, mock_boto3_client: MagicMock) -> None:
        number_of_slots_used = 0
        if would_exceed_slot_limit:
            number_of_slots_used = MAX_SAVE_SLOTS_PER_PLAYER_FOR_TESTING

        mock_paginator = mock_boto3_client.get_paginator('query')
        mock_paginator.paginate.return_value = Paginator.Query.mock_response_select_count([
            # First page:
            number_of_slots_used
        ])

    @staticmethod
    def set_presigned_url(url: str, mock_boto3_client: MagicMock) -> None:
        mock_boto3_client.generate_presigned_url.return_value = url

    @staticmethod
    def get_base64_string(unencoded_bytes = MAX_METADATA_UNENCODED_BYTES) -> str:
        unencoded_str = 'a' * unencoded_bytes
        base64_str = base64.b64encode(unencoded_str.encode('ascii'))
        return base64_str.decode('ascii')
