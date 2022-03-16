# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

import os
from unittest import TestCase
from unittest.mock import patch, MagicMock

import botocore

from functions.identity.GenerateFacebookLoginUrl import index
from functionsTests.helpers.boto3.mock_responses.exceptions import new_boto_exception


# Patch Lambda environment variables:
@patch.dict(os.environ, {
    'COGNITO_BASE_URL': 'foo_cognito_base_url',
    'APP_CLIENT_ID': 'foo_app_client_id',
    'REDIRECT_URI': 'foo_redirect_uri',
})
class TestIndex(TestCase):

    @patch('functions.identity.GenerateFacebookLoginUrl.index.crypto.boto3')
    def test_can_get_login_url_successfully(self, mock_boto3: MagicMock):
        # Arrange
        event = self.get_lambda_event()
        context = None
        mock_boto3.client('kms').generate_data_key.return_value = self.get_kms_generated_key()

        # Act
        result = index.lambda_handler(event, context)

        # Assert
        self.assertEqual(200, result['statusCode'])
        self.assertIsNotNone(result['body'])

        # Assert the encryption was successful, by confirming the last portion of the URL is not blank:
        __, encrypted_blob = result['body'].split('Facebook&state=')
        self.assertNotEqual('', encrypted_blob)

    @patch('functions.identity.GenerateFacebookLoginUrl.index.crypto.boto3')
    def test_encrypted_text_is_missing_from_login_url_when_kms_throws_an_error(self, mock_boto3: MagicMock):
        # Arrange
        event = self.get_lambda_event()
        context = None
        mock_boto3.client('kms').generate_data_key.side_effect = new_boto_exception(botocore.exceptions.ClientError)

        # Act
        result = index.lambda_handler(event, context)

        # Assert
        self.assertEqual(200, result['statusCode'])
        self.assertIsNotNone(result['body'])

        # Assert the encryption was NOT successful, by confirming the last portion of the URL is blank:
        __, encrypted_blob = result['body'].split('Facebook&state=')
        self.assertEqual('', encrypted_blob)

    def test_returns_400_when_request_id_is_invalid(self):
        # Arrange
        event = self.get_lambda_event_with_invalid_request_id()
        context = None

        # Act
        result = index.lambda_handler(event, context)

        # Assert
        self.assertEqual(400, result['statusCode'])

    @staticmethod
    def get_lambda_event():
        """
        Get a sample Lambda event.

        Note:
            The only field used by the Lambda is event['body']. We still use the whole event for the unit tests to
            ensure the Lambda has correct behavior when provided a realistic event.
        """
        return {
            'resource': '/identity/fbloginurl',
            'path': '/identity/fbloginurl',
            'httpMethod': 'POST',
            'headers': {
                'Accept': '*/*',
                'cache-control': 'no-cache',
                'CloudFront-Forwarded-Proto': 'https',
                'CloudFront-Is-Desktop-Viewer': 'true',
                'CloudFront-Is-Mobile-Viewer': 'false',
                'CloudFront-Is-SmartTV-Viewer': 'false',
                'CloudFront-Is-Tablet-Viewer': 'false',
                'CloudFront-Viewer-Country': 'US',
                'content-type': 'application/json',
                'Host': '123abcdefg.execute-api.us-west-2.amazonaws.com',
                'pragma': 'no-cache',
                'User-Agent': 'Amazon CloudFront',
                'Via': '2.0 aef00f14752da9aa504d392fd46eff94.cloudfront.net (CloudFront)',
                'X-Amz-Cf-Id': 'pMF_g83JIwbkZxiMacpvUfrAea2u7fCmi0eGNWMvVxOEqMLnDr3IwA==',
                'X-Amzn-Trace-Id': 'Root=1-60e778f5-01728f93222e2e903175b51e',
                'X-Forwarded-For': '12.123.123.123, 56.567.567.56',
                'X-Forwarded-Port': '443',
                'X-Forwarded-Proto': 'https'
            },
            'multiValueHeaders': {
                'Accept': ['*/*'],
                'cache-control': ['no-cache'],
                'CloudFront-Forwarded-Proto': ['https'],
                'CloudFront-Is-Desktop-Viewer': ['true'],
                'CloudFront-Is-Mobile-Viewer': ['false'],
                'CloudFront-Is-SmartTV-Viewer': ['false'],
                'CloudFront-Is-Tablet-Viewer': ['false'],
                'CloudFront-Viewer-Country': ['US'],
                'content-type': ['application/json'],
                'Host': ['123abcdefg.execute-api.us-west-2.amazonaws.com'],
                'pragma': ['no-cache'],
                'User-Agent': ['Amazon CloudFront'],
                'Via': ['2.0 aef00f14752da9aa504d392fd46eff94.cloudfront.net (CloudFront)'],
                'X-Amz-Cf-Id': ['pMF_g83JIwbkZxiMacpvUfrAea2u7fCmi0eGNWMvVxOEqMLnDr3IwA=='],
                'X-Amzn-Trace-Id': ['Root=1-60e778f5-01728f93222e2e903175b51e'],
                'X-Forwarded-For': ['12.123.123.123, 56.567.567.56'],
                'X-Forwarded-Port': ['443'],
                'X-Forwarded-Proto': ['https']
            },
            'queryStringParameters': None,
            'multiValueQueryStringParameters': None,
            'pathParameters': None,
            'stageVariables': None,
            'requestContext': {
                'resourceId': '123abc',
                'resourcePath': '/identity/fbloginurl',
                'httpMethod': 'POST',
                'extendedRequestId': 'CK_WaFkOPHcFxXg=',
                'requestTime': '08/Jul/2021:22:15:17 +0000',
                'path': '/dev/identity/fbloginurl',
                'accountId': '123456789012',
                'protocol': 'HTTP/1.1',
                'stage': 'dev',
                'domainPrefix': '123abcdefg',
                'requestTimeEpoch': 1625782517651,
                'requestId': '83ecb1c7-4eec-498b-b3d6-d6acece8a0a7',
                'identity': {
                    'cognitoIdentityPoolId': None,
                    'accountId': None,
                    'cognitoIdentityId': None,
                    'caller': None,
                    'sourceIp': '12.123.123.12',
                    'principalOrgId': None,
                    'accessKey': None,
                    'cognitoAuthenticationType': None,
                    'cognitoAuthenticationProvider': None,
                    'userArn': None,
                    'userAgent': 'Amazon CloudFront',
                    'user': None
                },
                'domainName': '123abcdefg.execute-api.us-west-2.amazonaws.com',
                'apiId': '123abcdefg'
            },
            'body': '{"request_id": "f0a7fa67-8997-4ea6-8a0e-c81799209f71"}',
            'isBase64Encoded': False
        }

    @staticmethod
    def get_lambda_event_with_invalid_request_id():
        """
        Get a sample Lambda event with invalid request_id.
        """
        return {
            'resource': '/identity/fbloginurl',
            'path': '/identity/fbloginurl',
            'httpMethod': 'POST',
            'headers': {
                'Accept': '*/*',
                'cache-control': 'no-cache',
                'CloudFront-Forwarded-Proto': 'https',
                'CloudFront-Is-Desktop-Viewer': 'true',
                'CloudFront-Is-Mobile-Viewer': 'false',
                'CloudFront-Is-SmartTV-Viewer': 'false',
                'CloudFront-Is-Tablet-Viewer': 'false',
                'CloudFront-Viewer-Country': 'US',
                'content-type': 'application/json',
                'Host': '123abcdefg.execute-api.us-west-2.amazonaws.com',
                'pragma': 'no-cache',
                'User-Agent': 'Amazon CloudFront',
                'Via': '2.0 aef00f14752da9aa504d392fd46eff94.cloudfront.net (CloudFront)',
                'X-Amz-Cf-Id': 'pMF_g83JIwbkZxiMacpvUfrAea2u7fCmi0eGNWMvVxOEqMLnDr3IwA==',
                'X-Amzn-Trace-Id': 'Root=1-60e778f5-01728f93222e2e903175b51e',
                'X-Forwarded-For': '12.123.123.123, 56.567.567.56',
                'X-Forwarded-Port': '443',
                'X-Forwarded-Proto': 'https'
            },
            'multiValueHeaders': {
                'Accept': ['*/*'],
                'cache-control': ['no-cache'],
                'CloudFront-Forwarded-Proto': ['https'],
                'CloudFront-Is-Desktop-Viewer': ['true'],
                'CloudFront-Is-Mobile-Viewer': ['false'],
                'CloudFront-Is-SmartTV-Viewer': ['false'],
                'CloudFront-Is-Tablet-Viewer': ['false'],
                'CloudFront-Viewer-Country': ['US'],
                'content-type': ['application/json'],
                'Host': ['123abcdefg.execute-api.us-west-2.amazonaws.com'],
                'pragma': ['no-cache'],
                'User-Agent': ['Amazon CloudFront'],
                'Via': ['2.0 aef00f14752da9aa504d392fd46eff94.cloudfront.net (CloudFront)'],
                'X-Amz-Cf-Id': ['pMF_g83JIwbkZxiMacpvUfrAea2u7fCmi0eGNWMvVxOEqMLnDr3IwA=='],
                'X-Amzn-Trace-Id': ['Root=1-60e778f5-01728f93222e2e903175b51e'],
                'X-Forwarded-For': ['12.123.123.123, 56.567.567.56'],
                'X-Forwarded-Port': ['443'],
                'X-Forwarded-Proto': ['https']
            },
            'queryStringParameters': None,
            'multiValueQueryStringParameters': None,
            'pathParameters': None,
            'stageVariables': None,
            'requestContext': {
                'resourceId': '123abc',
                'resourcePath': '/identity/fbloginurl',
                'httpMethod': 'POST',
                'extendedRequestId': 'CK_WaFkOPHcFxXg=',
                'requestTime': '08/Jul/2021:22:15:17 +0000',
                'path': '/dev/identity/fbloginurl',
                'accountId': '123456789012',
                'protocol': 'HTTP/1.1',
                'stage': 'dev',
                'domainPrefix': '123abcdefg',
                'requestTimeEpoch': 1625782517651,
                'requestId': '83ecb1c7-4eec-498b-b3d6-d6acece8a0a7',
                'identity': {
                    'cognitoIdentityPoolId': None,
                    'accountId': None,
                    'cognitoIdentityId': None,
                    'caller': None,
                    'sourceIp': '12.123.123.12',
                    'principalOrgId': None,
                    'accessKey': None,
                    'cognitoAuthenticationType': None,
                    'cognitoAuthenticationProvider': None,
                    'userArn': None,
                    'userAgent': 'Amazon CloudFront',
                    'user': None
                },
                'domainName': '123abcdefg.execute-api.us-west-2.amazonaws.com',
                'apiId': '123abcdefg'
            },
            'body': '{"request_id": "THIS-IS-NOT-A-UUID"}',
            'isBase64Encoded': False
        }

    @staticmethod
    def get_kms_generated_key():
        """
        Get a sample response for boto3.client(kms).generate_data_key(KeyId=cmk_id, KeySpec='AES_256').
        """
        return {
            'CiphertextBlob': b'\x01\x02\x03\x00x\xdd\xc7vu\x9d\xf2\x80]\x98\xa0\\\x86\xfb\xd7QM\x94$,'
                              b'j\xa6a~\x1f\xb9\xd5\xb3\x1b\xc8\xf6\xe7\xf7\x01\xda\xf4\'\x94\x0b\xa6*\x11\x86.\xa1'
                              b'\x0b\x17a\xe8\xb8\x00\x00\x00~0|\x06\t*\x86H\x86\xf7\r\x01\x07\x06\xa0o0m\x02\x01'
                              b'\x000h\x06\t*\x86H\x86\xf7\r\x01\x07\x010\x1e\x06\t`\x86H\x01e\x03\x04\x01.0\x11\x04'
                              b'\x0cQ|\x11\x12\xf1\x88\xc0\xee+o\xc8\xf8\x02\x01\x10\x80;\\\xf1\xe1\x04:g\x81\x8bR'
                              b'\x02\xaa\x08\xa8\xc4Dm\x88\x9f\x84\xb4/\x82\xfb/x\xa8\xfb\x8c\x83\xc1\x8e\x0c\xac\xfe'
                              b'\x16\x0fR]\xbeZ\x13}\x85\x93\xad`;\xa2|O\xe0\xb4mc\xdbp\x1fY\xbb',
            'Plaintext': b'$\x07\x00\xab#\xb2\x13\\\xf7X\xde\xb7\xf2\x84\x00<\x82\xa4Y\xb8\xdc\xb1\xcb\x8d\x1d\xbc'
                         b'\xb6v\x0e\xc7\xcdE',
            'KeyId': 'arn:aws:kms:us-west-2:123456789012:key/foo-gamekit-kms-key-id',
            'ResponseMetadata': {
                'RequestId': '5538dc81-c856-456b-ba59-247ef556ce8a',
                'HTTPStatusCode': 200,
                'HTTPHeaders': {
                    'x-amzn-requestid': '5538dc81-c856-456b-ba59-247ef556ce8a',
                    'cache-control': 'no-cache, no-store, must-revalidate, private',
                    'expires': '0',
                    'pragma': 'no-cache',
                    'date': 'Fri, 09 Jul 2021 08:35:08 GMT',
                    'content-type': 'application/x-amz-json-1.1',
                    'content-length': '414'
                },
                'RetryAttempts': 0
            }
        }
