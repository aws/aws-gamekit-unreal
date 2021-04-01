# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

import os
from unittest import TestCase
from unittest.mock import patch, MagicMock, ANY

with patch("boto3.resource") as boto_resource_mock:
    from functions.gamesaving.UpdateSlotMetadata import index

from functionsTests.helpers.boto3.mock_responses.DynamoDB.Table import PutItem


TEST_BUCKET_NAME = 'gamekit-dev-123456789012-foogamename-player-gamesaves'
TEST_LAST_MODIFIED_TIME = '1628726400000'  # 2021-08-12 0:00:00 UTC

# When there is no metadata for a save slot, the S3 object's metadata contains this string:
EMPTY_METADATA = ''

# Sample base64 encoded metadata
BASE_64_METADATA = 'eydkZXNjcmlwdGlvbic6J2xldmVsIDMgY29tcGxldGUnLCdwZXJjZW50Y29tcGxldGUnOjM1fQ'

# Patch Lambda environment variables:
@patch.dict(os.environ, {
    'GAMESAVES_TABLE_NAME': 'gamekit_dev_foogamename_player_gamesaves',
    'GAMESAVES_BUCKET_NAME': TEST_BUCKET_NAME,
})
class TestIndex(TestCase):
    def setUp(self):
        index.s3_resource = MagicMock()

    @patch('functions.gamesaving.UpdateSlotMetadata.index.write_metadata_to_dynamodb')
    def test_can_write_metadata_to_dynamodb_for_any_kind_of_s3_key(self, mock_write_metadata_to_dynamodb: MagicMock):
        sub_tests = [
            ('ascii letters only', 'foo_player_id/foo_slot_name', 'foo_player_id', 'foo_slot_name'),
            # Note: special characters (!@#$%^&*_) and non-ascii characters (參賽者姓名) are returned as URL encoded strings by S3:
            ('ascii special characters', 'foo_player_id_%21%40%23%24%25%5E%26*%28%29_%2B/foo_slot_name_%21%40%23%24%25%5E%26*%28%29_%2B', 'foo_player_id_!@#$%^&*()_+', 'foo_slot_name_!@#$%^&*()_+'),
            ('non-ascii', '%E5%8F%83%E8%B3%BD%E8%80%85%E5%A7%93%E5%90%8D/%E6%8F%92%E6%A7%BD%E5%90%8D%E7%A8%B1', '參賽者姓名', '插槽名稱'),
        ]

        for test_name, s3_object_key, expected_player_id, expected_slot_name in sub_tests:
            with self.subTest(test_name):
                # Arrange
                event = self.get_lambda_event(s3_object_key)
                context = None
                self.set_mock_s3_object_metadata(EMPTY_METADATA, TEST_LAST_MODIFIED_TIME, index.s3_resource)
                self.set_mock_put_item_response(index.s3_resource)

                # Act
                index.lambda_handler(event, context)

                # Assert
                mock_write_metadata_to_dynamodb.assert_called_with(expected_player_id, expected_slot_name, ANY, ANY, ANY)

    @patch('functions.gamesaving.UpdateSlotMetadata.index.write_metadata_to_dynamodb')
    def test_can_write_metadata_to_dynamodb(self, mock_write_metadata_to_dynamodb: MagicMock):
        for test_name, metadata in [("empty metadata", EMPTY_METADATA), ("base64 metadata", BASE_64_METADATA)]:
            with self.subTest(test_name):
                # Arrange
                event = self.get_lambda_event()
                context = None
                self.set_mock_s3_object_metadata(metadata, TEST_LAST_MODIFIED_TIME, index.s3_resource)
                self.set_mock_put_item_response(index.s3_resource)

                # Act
                index.lambda_handler(event, context)

                # Assert
                mock_write_metadata_to_dynamodb.assert_called_with(ANY, ANY, metadata, ANY, ANY)

    @patch('functions.gamesaving.UpdateSlotMetadata.index.write_metadata_to_dynamodb')
    def test_can_write_metadata_to_dynamodb_when_the_event_contains_multiple_records(self, mock_write_metadata_to_dynamodb: MagicMock):
        # Arrange
        event = self.get_lambda_event()
        event['Records'] = [
            self.get_record('foo_player_id/save-01'),
            self.get_record('foo_player_id/save-02'),
            self.get_record('foo_player_id/save-03'),
        ]
        context = None
        self.set_mock_s3_object_metadata(EMPTY_METADATA, TEST_LAST_MODIFIED_TIME, index.s3_resource)
        self.set_mock_put_item_response(index.s3_resource)

        # Act
        index.lambda_handler(event, context)

        # Assert
        self.assertEqual(3, mock_write_metadata_to_dynamodb.call_count)

    @patch('functions.gamesaving.UpdateSlotMetadata.index.write_metadata_to_dynamodb')
    def test_dynamo_item_is_created_with_last_modified_time_matching_s3_object_metadata(self, mock_write_metadata_to_dynamodb: MagicMock):
        # Arrange
        event = self.get_lambda_event()
        context = None
        expected_last_modified_time = int(TEST_LAST_MODIFIED_TIME)
        self.set_mock_s3_object_metadata(EMPTY_METADATA, str(expected_last_modified_time), index.s3_resource)
        self.set_mock_put_item_response(index.s3_resource)

        # Act
        index.lambda_handler(event, context)

        # Assert
        mock_write_metadata_to_dynamodb.assert_called_once_with(ANY, ANY, ANY, expected_last_modified_time, ANY)

    @staticmethod
    def set_mock_put_item_response(mock_boto3: MagicMock) -> None:
        mock_gamesaves_table = mock_boto3.resource('dynamodb').Table()
        mock_gamesaves_table.put_item.return_value = PutItem.mock_response()

    @staticmethod
    def set_mock_s3_object_metadata(metadata: str, last_modified: str, mock_boto3_resource: MagicMock) -> None:
        mock_s3_object = mock_boto3_resource.Object()
        mock_s3_object.metadata = {
            'slot_metadata': metadata,
            'epoch': last_modified
        }

    @staticmethod
    def get_lambda_event(s3_object_key: str = 'foo_player_id/foo_slot_name'):
        """
        Sample Lambda event for an S3 ObjectCreated:Put event.

        All ObjectCreated event types (Put, Post, Copy, CompleteMultipartUpload) follow the same event format:
        https://docs.aws.amazon.com/AmazonS3/latest/userguide/notification-content-structure.html
        """
        return {
            'Records': [
                TestIndex.get_record(s3_object_key)
            ]
        }

    @staticmethod
    def get_record(s3_object_key: str = 'foo_player_id/foo_slot_name'):
        return {
            'eventVersion': '2.1',
            'eventSource': 'aws:s3',
            'awsRegion': 'us-west-2',
            'eventTime': '2021-08-05T21:47:10.086Z',
            'eventName': 'ObjectCreated:Put',
            'userIdentity': {
                'principalId': 'AWS:ABCDEFGHIJKLMNOPQRSTU'
            },
            'requestParameters': {
                'sourceIPAddress': '123.123.123.12'
            },
            'responseElements': {
                'x-amz-request-id': 'BZ8B5M78W5WEVN58',
                'x-amz-id-2': 'aC7P6jPGAtdMhWqqxQAgG0vnvImqUAq5LZzT1ruFudKFzhWzf4KEQ4HHXB0ukDiiFlWR7lPWRun+viCrbMM/rOW926zBxIKW'
            },
            's3': {
                's3SchemaVersion': '1.0',
                'configurationId': 'OGQ2OThjNGYtODliNS00YTEzLWJjZDUtOWFlOWUzYzhkZTU5',
                'bucket': {
                    'name': TEST_BUCKET_NAME,
                    'ownerIdentity': {
                        'principalId': 'ABCDEFGHIJKLM'
                    },
                    'arn': f'arn:aws:s3:::{TEST_BUCKET_NAME}'
                },
                'object': {
                    'key': s3_object_key,
                    'size': 123456,
                    'eTag': 'e4d7f1b4ed2e42d15898f4b27b019da4',
                    'sequencer': '00610C5C5EBE33DE0C'
                }
            }
        }
