# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

import os
from unittest import TestCase
from unittest.mock import patch

import boto3
import botocore

from functionsTests.helpers.boto3.mock_responses.exceptions import new_boto_exception


class TestBoto3Helper(TestCase):

    def test_can_create_botocore_client_error_successfully(self):
        # Arrange
        exception_constructor = botocore.exceptions.ClientError

        # Act
        exception = new_boto_exception(exception_constructor)

        # Assert
        self.assertIsNotNone(exception)

    # Patch the default AWS region so 'ddb_client' can be created on systems where
    # the AWS credentials file and AWS configuration file are missing.
    # See: https://boto3.amazonaws.com/v1/documentation/api/latest/guide/quickstart.html#configuration
    @patch.dict(os.environ, {'AWS_DEFAULT_REGION': 'us-west-2'})
    def test_can_create_dynamodb_error_successfully(self):
        # Arrange
        ddb_client = boto3.client('dynamodb')
        exception_constructor = ddb_client.exceptions.ConditionalCheckFailedException

        # Act
        exception = new_boto_exception(exception_constructor)

        # Assert
        self.assertIsNotNone(exception)
