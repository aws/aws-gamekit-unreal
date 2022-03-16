# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

import json
from unittest import TestCase
from unittest.mock import patch, MagicMock

from layers.main.CommonLambdaLayer.python.gamekithelpers.cfn_custom_resource import (
    send_success_response,
    send_failure_response,
    ResponseStatus
)

CUSTOM_RESOURCE_TYPE = 'some-resource-type'
PHYSICAL_RESOURCE_ID = 'empty-on-delete-some-hash-here'
STACK_ID = 'test_stack_id'
LOGICAL_RESOURCE_ID = 'test_logical_resource_id'
REQUEST_ID = 'test_request_id'
RESPONSE_URL = 'https://website.tld/some_url'


class TestIndex(TestCase):
    @patch('urllib.request.urlopen')
    def test_send_success_response(self, mock_urlopen):
        # Arrange
        event = self.get_event()
        mock_urlopen.return_value = self.get_urllib_context_manager(200)

        # Act
        send_success_response(event, CUSTOM_RESOURCE_TYPE)

        # Assert
        self.validate_cloudformation_request(ResponseStatus.SUCCESS, mock_urlopen)

    @patch('urllib.request.urlopen')
    def test_send_failure_response(self, mock_urlopen):
        # Arrange
        event = self.get_event()
        mock_urlopen.return_value = self.get_urllib_context_manager(200)

        # Act
        send_failure_response(event, CUSTOM_RESOURCE_TYPE, 'some reason')

        # Assert
        self.validate_cloudformation_request(ResponseStatus.FAILED, mock_urlopen)

    @patch('urllib.request.urlopen')
    def test_raises_error_on_invalid_custom_resource_type(self, mock_urlopen):
        # Arrange
        event = self.get_event()
        sub_test = [
            ('too short', ''),
            ('too long', 'a' * 33),
            ('invalid chars', '!$%')
        ]
        for test_name, custom_resource_type in sub_test:
            with self.subTest(test_name):
                # Act
                with self.assertRaises(ValueError):
                    send_success_response(event, custom_resource_type)

                # Assert
                mock_urlopen.assert_not_called()

    def validate_cloudformation_request(self, status: ResponseStatus, mock_urlopen: MagicMock):
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
            'ResourceProperties': {},
            'PhysicalResourceId': PHYSICAL_RESOURCE_ID,
            'StackId': STACK_ID,
            'LogicalResourceId': LOGICAL_RESOURCE_ID,
            'RequestId': REQUEST_ID,
            'ResponseURL': RESPONSE_URL
        }

    @staticmethod
    def get_urllib_context_manager(status_code: int):
        context_manager = MagicMock()
        context_manager.getcode.return_value = status_code
        context_manager.reason.return_value = status_code
        context_manager.read.return_value = 'request_body'
        context_manager.__enter__.return_value = context_manager
        return context_manager
