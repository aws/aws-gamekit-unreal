# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

import json
from unittest import TestCase
from unittest.mock import patch, MagicMock

from botocore.exceptions import ClientError

with patch("boto3.resource") as boto_resource_mock:
    with patch("boto3.client") as boto_client_mock:
        from functions.main.EmptyS3BucketOnDelete import index

from functions.main.EmptyS3BucketOnDelete.index import ResponseStatus

BUCKET_NAME = 'test_bucket'
LOG_GROUP_NAME = 'test_log_group'
PHYSICAL_RESOURCE_ID = 'empty-on-delete-some-hash-here'
STACK_ID = 'test_stack_id'
LOGICAL_RESOURCE_ID = 'test_logical_resource_id'
REQUEST_ID = 'test_request_id'
RESPONSE_URL = 'https://website.tld/some_url'

HEAD_OBJECT = 'HeadObject'


class TestIndex(TestCase):
    def setUp(self):
        index.s3_resource = MagicMock()
        index.s3_client = MagicMock()

    @patch('urllib.request.urlopen')
    def test_calls_delete_all_bucket_non_versioned_success(self, mock_urlopen: MagicMock):
        # Arrange
        event = self.get_event()
        context = self.get_context()

        mock_urlopen.return_value = self.get_urllib_context_manager(200)

        index.s3_resource.BucketVersioning.return_value.status = None
        index.s3_resource.Bucket.return_value.objects.all.return_value.delete.return_value = {'Deleted': []}

        # Act
        index.lambda_handler(event, context)

        # Assert
        index.s3_resource.Bucket.assert_called_with(BUCKET_NAME)
        index.s3_resource.Bucket.return_value.objects.all.return_value.delete.assert_called_once()
        index.s3_resource.Bucket.return_value.object_versions.all.assert_not_called()

        self.validate_cloudformation_request(mock_urlopen, ResponseStatus.SUCCESS)

    @patch('urllib.request.urlopen')
    def test_calls_delete_all_bucket_versioned_success(self, mock_urlopen: MagicMock):
        # Arrange
        event = self.get_event()
        context = self.get_context()

        index.s3_resource.BucketVersioning.return_value.status = 'Enabled'
        index.s3_resource.Bucket.return_value.object_versions.all.return_value.delete.return_value = {'Deleted': []}

        mock_urlopen.return_value = self.get_urllib_context_manager(200)

        # Act
        index.lambda_handler(event, context)

        # Assert
        index.s3_resource.Bucket.assert_called_with(BUCKET_NAME)
        index.s3_resource.Bucket.return_value.objects.all.assert_not_called()
        index.s3_resource.Bucket.return_value.object_versions.all.return_value.delete.assert_called_once()
        self.validate_cloudformation_request(mock_urlopen, ResponseStatus.SUCCESS)

    @patch('urllib.request.urlopen')
    def test_ignores_non_delete_events(self, mock_urlopen: MagicMock):
        # Arrange
        event = self.get_event()
        event['RequestType'] = 'Create'
        context = self.get_context()

        # Act
        index.lambda_handler(event, context)

        # Assert
        index.s3_resource.Bucket.assert_not_called()
        self.validate_cloudformation_request(mock_urlopen, ResponseStatus.SUCCESS)

    @patch('urllib.request.urlopen')
    def test_fails_on_head_bucket_forbidden(self, mock_urlopen: MagicMock):
        # Arrange
        event = self.get_event()
        context = self.get_context()

        index.s3_client.head_bucket.side_effect = ClientError({
            'Error': {
                'Code': '403',
                'Message': 'Forbidden'
            }
        }, HEAD_OBJECT)

        # Act
        index.lambda_handler(event, context)

        # Assert
        index.s3_resource.Bucket.assert_not_called()
        self.validate_cloudformation_request(mock_urlopen, ResponseStatus.FAILED)

    @patch('urllib.request.urlopen')
    def test_ignores_non_existent_bucket(self, mock_urlopen: MagicMock):
        # Arrange
        event = self.get_event()
        context = self.get_context()

        index.s3_client.head_bucket.side_effect = ClientError({
            'Error': {
                'Code': '404',
                'Message': 'Not Found'
            }
        }, HEAD_OBJECT)

        # Act
        index.lambda_handler(event, context)

        # Assert
        index.s3_resource.Bucket.assert_not_called()
        self.validate_cloudformation_request(mock_urlopen, ResponseStatus.SUCCESS)

    @patch('urllib.request.urlopen')
    def test_calls_delete_all_bucket_non_versioned_failed(self, mock_urlopen: MagicMock):
        # Arrange
        event = self.get_event()
        context = self.get_context()

        mock_urlopen.return_value = self.get_urllib_context_manager(200)

        index.s3_resource.BucketVersioning.return_value.status = None
        index.s3_resource.Bucket.return_value.objects.all.side_effect = ClientError({
            'Error': {
                'Code': 500,
                'Message': 'Some S3 Error'
            }
        }, 'ListObjects')

        # Act
        index.lambda_handler(event, context)

        # Assert
        index.s3_resource.Bucket.assert_called_with(BUCKET_NAME)
        index.s3_resource.Bucket.return_value.objects.all.assert_called_once()
        index.s3_resource.Bucket.return_value.object_versions.all.assert_not_called()

        self.validate_cloudformation_request(mock_urlopen, ResponseStatus.FAILED)

    def validate_cloudformation_request(self, mock_urlopen: MagicMock, status: ResponseStatus):
        mock_urlopen.assert_called_once()
        urlopen_request = mock_urlopen.call_args[0][0]
        request_data = json.loads(urlopen_request.data)
        self.assertTrue(urlopen_request.full_url == RESPONSE_URL)
        self.assertTrue(urlopen_request.method == 'PUT')
        self.assertTrue(request_data['Status'] == status)
        self.assertTrue(request_data['StackId'] == STACK_ID)
        self.assertTrue(request_data['LogicalResourceId'] == LOGICAL_RESOURCE_ID)
        self.assertTrue(request_data['RequestId'] == REQUEST_ID)

    @staticmethod
    def get_event():
        return {
            'RequestType': 'Delete',
            'ResourceProperties': {
                'bucket_name': BUCKET_NAME
            },
            'PhysicalResourceId': PHYSICAL_RESOURCE_ID,
            'StackId': STACK_ID,
            'LogicalResourceId': LOGICAL_RESOURCE_ID,
            'RequestId': REQUEST_ID,
            'ResponseURL': RESPONSE_URL
        }

    @staticmethod
    def get_context():
        context = MagicMock()
        context.log_group_name = LOG_GROUP_NAME
        return context

    @staticmethod
    def get_urllib_context_manager(status_code: int):
        context_manager = MagicMock()
        context_manager.getcode.return_value = status_code
        context_manager.reason.return_value = status_code
        context_manager.read.return_value = 'request_body'
        context_manager.__enter__.return_value = context_manager
        return context_manager
