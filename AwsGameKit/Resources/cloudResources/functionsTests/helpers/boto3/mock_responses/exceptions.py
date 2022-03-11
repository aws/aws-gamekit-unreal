# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

import botocore


def new_boto_exception(exception_constructor):
    """
    Get a new boto3 exception of the specified type with a mock exception message.

    The mock exception message will look like this:
        >>> 'An error occurred (MockError) when calling the MockOperation operation: mock message'

    Example (different exception types):
        >>> import botocore
        >>> new_boto_exception(botocore.exceptions.ClientError)
        >>>
        >>> import boto3
        >>> ddb_client = boto3.client('dynamodb')
        >>> new_boto_exception(ddb_client.exceptions.ConditionalCheckFailedException)

    Example (inside a unit test):
        >>> from unittest.mock import patch, MagicMock
        >>> from functionsTests.helpers.boto3.mock_responses.exceptions import new_boto_exception
        >>>
        >>> @patch('path.to.test.file.boto3')
        >>> def test_can_handle_error_gracefully(self, mock_boto3: MagicMock):
        >>>     # Arrange
        >>>     mock_boto3.client('kms').generate_data_key.side_effect = new_boto_exception(botocore.exceptions.ClientError)
        >>>
        >>>     # ... etc.
    """
    return exception_constructor(
        operation_name='MockOperation',
        error_response={
            'Error': {
                'Code': 'MockError',
                'Message': 'mock message'
            }
        }
    )
