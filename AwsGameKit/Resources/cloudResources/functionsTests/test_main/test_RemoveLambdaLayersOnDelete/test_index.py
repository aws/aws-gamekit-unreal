# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

from unittest import TestCase
from unittest.mock import patch, MagicMock

from botocore.exceptions import ClientError

with patch("boto3.client") as boto_client_mock:
    with patch("gamekitresourcemanagement.cfn_custom_resource.send_success_response") as mock_send_success_response:
        with patch('gamekitresourcemanagement.cfn_custom_resource.send_failure_response') as mock_send_failure_response:
            from functions.main.RemoveLambdaLayersOnDelete import index

LAYER_1 = 'LambdaLayer1'
LAYER_2 = 'LambdaLayer2'
UNMATCHED_LAYER = 'UnmatchedLambdaLayer'
LAYER_VERSIONS_1 = [1, 2, 3]
LAYER_VERSIONS_2 = [1]
NEXT_MARKER = 'NextMarker'

LAYER_PREFIX = 'LambdaLayer'
PHYSICAL_RESOURCE_ID = 'delete-lambda-layers-some-hash-here'
STACK_ID = 'test_stack_id'
LOGICAL_RESOURCE_ID = 'test_logical_resource_id'
REQUEST_ID = 'test_request_id'
RESPONSE_URL = 'https://website.tld/some_url'


class TestIndex(TestCase):
    def setUp(self):
        index.lambda_client = MagicMock()
        mock_send_success_response.reset_mock()
        mock_send_failure_response.reset_mock()

    def test_removes_all_matching_lambda_layers_no_pagination(self):
        # Arrange
        event = self.get_event()
        context = self.get_context()

        index.lambda_client.list_layers.return_value = self.get_list_layers_response(
            [LAYER_1, LAYER_2, UNMATCHED_LAYER])

        index.lambda_client.list_layer_versions.side_effect = [
            self.get_list_layer_versions_response(LAYER_VERSIONS_1),
            self.get_list_layer_versions_response(LAYER_VERSIONS_2)
        ]

        # Act
        index.lambda_handler(event, context)

        # Assert
        index.lambda_client.list_layers.assert_called_once()
        self.assertEqual(index.lambda_client.list_layer_versions.call_count, 2)
        self.assertEqual(index.lambda_client.delete_layer_version.call_count, 4)
        mock_send_success_response.assert_called_once()
        mock_send_failure_response.assert_not_called()

    def test_removes_all_matching_lambda_layers_pagination(self):
        # Arrange
        event = self.get_event()
        context = self.get_context()

        index.lambda_client.list_layers.side_effect = [
            self.get_list_layers_response([LAYER_1], NEXT_MARKER),
            self.get_list_layers_response([LAYER_2, UNMATCHED_LAYER])
        ]

        # Paginate layer versions request for the first layer
        index.lambda_client.list_layer_versions.side_effect = [
            self.get_list_layer_versions_response(LAYER_VERSIONS_1[0:2], NEXT_MARKER),
            self.get_list_layer_versions_response(LAYER_VERSIONS_1[2:]),
            self.get_list_layer_versions_response(LAYER_VERSIONS_2)
        ]

        # Act
        index.lambda_handler(event, context)

        # Assert
        self.assertEqual(index.lambda_client.list_layers.call_count, 2)
        self.assertEqual(index.lambda_client.list_layer_versions.call_count, 3)
        self.assertEqual(index.lambda_client.delete_layer_version.call_count, 4)
        mock_send_success_response.assert_called_once()
        mock_send_failure_response.assert_not_called()

    def test_deletes_nothing_if_no_layers_exist(self):
        # Arrange
        event = self.get_event()
        context = self.get_context()

        index.lambda_client.list_layers.return_value = self.get_list_layers_response([])

        # Act
        index.lambda_handler(event, context)

        # Assert
        index.lambda_client.list_layers.assert_called_once()
        index.lambda_client.list_layer_versions.assert_not_called()
        index.lambda_client.delete_layer_version.assert_not_called()
        mock_send_success_response.assert_called_once()
        mock_send_failure_response.assert_not_called()

    def test_sends_failure_on_list_layers_error(self):
        # Arrange
        event = self.get_event()
        context = self.get_context()

        index.lambda_client.list_layers.side_effect = ClientError({
            'Error': {
                'Code': 500,
                'Message': 'Some Lambda Error'
            }
        }, 'ListLayers')

        # Act
        with self.assertRaises(ClientError):
            index.lambda_handler(event, context)

        # Assert
        index.lambda_client.list_layers.assert_called_once()
        index.lambda_client.list_layer_versions.assert_not_called()
        index.lambda_client.delete_layer_version.assert_not_called()
        mock_send_success_response.assert_not_called()
        mock_send_failure_response.assert_called_once()

    def test_sends_failure_on_list_layer_versions_error(self):
        # Arrange
        event = self.get_event()
        context = self.get_context()

        index.lambda_client.list_layers.return_value = self.get_list_layers_response([LAYER_1])

        index.lambda_client.list_layer_versions.side_effect = ClientError({
            'Error': {
                'Code': 500,
                'Message': 'Some Lambda Error'
            }
        }, 'ListLayerVersions')

        # Act
        with self.assertRaises(ClientError):
            index.lambda_handler(event, context)

        # Assert
        index.lambda_client.list_layers.assert_called_once()
        index.lambda_client.list_layer_versions.assert_called_once()
        index.lambda_client.delete_layer_version.assert_not_called()
        mock_send_success_response.assert_not_called()
        mock_send_failure_response.assert_called_once()

    def test_sends_failure_on_delete_layer_version_error(self):
        # Arrange
        event = self.get_event()
        context = self.get_context()

        index.lambda_client.list_layers.return_value = self.get_list_layers_response([LAYER_1])

        index.lambda_client.list_layer_versions.return_value = self.get_list_layer_versions_response([LAYER_VERSIONS_1])

        index.lambda_client.delete_layer_version.side_effect = ClientError({
            'Error': {
                'Code': 500,
                'Message': 'Some Lambda Error'
            }
        }, 'DeleteLayerVersion')

        # Act
        with self.assertRaises(ClientError):
            index.lambda_handler(event, context)

        # Assert
        index.lambda_client.list_layers.assert_called_once()
        index.lambda_client.list_layer_versions.assert_called_once()
        index.lambda_client.delete_layer_version.assert_called_once()
        mock_send_success_response.assert_not_called()
        mock_send_failure_response.assert_called_once()

    def test_ignores_non_delete_events(self):
        # Arrange
        event = self.get_event()
        event['RequestType'] = 'Create'
        context = self.get_context()

        # Act
        index.lambda_handler(event, context)

        # Assert
        index.lambda_client.list_layers.assert_not_called()
        index.lambda_client.list_layer_versions.assert_not_called()
        index.lambda_client.delete_layer_versions.assert_not_called()
        mock_send_success_response.assert_called_once()
        mock_send_failure_response.assert_not_called()

    @staticmethod
    def get_list_layers_response(layers: [str], next_marker: str = None):
        return {
            'Layers': [{'LayerName': layer} for layer in layers],
            'NextMarker': next_marker
        }

    @staticmethod
    def get_list_layer_versions_response(layer_versions: [int], next_marker: str = None):
        return {
            'LayerVersions': [{'Version': layer_version} for layer_version in layer_versions],
            'NextMarker': next_marker
        }

    @staticmethod
    def get_event():
        return {
            'RequestType': 'Delete',
            'ResourceProperties': {
                'layer_prefix': LAYER_PREFIX
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
        return context
