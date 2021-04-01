# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

import datetime
from dateutil.tz import tzutc
from unittest import TestCase
from unittest.mock import patch, MagicMock

import botocore

with patch("boto3.client") as boto_client_mock:
    from functions.identity.PollFacebookLoginCompletion import index
from functionsTests.helpers.boto3.mock_responses.exceptions import new_boto_exception


class TestIndex(TestCase):
    def setUp(self):
        index.s3_client = MagicMock()

    def test_can_retrieve_encrypted_key_location_successfully(self):
        # Arrange
        event = self.get_lambda_event()
        context = None
        # Mock the return_value of decode() instead of get_object['Body'] because the latter can't be easily mocked.
        # (it's a 'botocore.response.StreamingBody' object)
        index.s3_client.get_object['Body'].read().decode.return_value = 'foo_encrypted_key_location'

        # Act
        result = index.lambda_handler(event, context)

        # Assert
        self.assertEqual(200, result['statusCode'])
        self.assertNotEqual('', result['body'])
        self.assertIsNotNone(result['body'])

    def test_lambda_returns_a_400_error_code_when_the_event_body_is_malformed(self):
        sub_tests = [
            ("missing event body", None),
            ("missing request_id", '{}'),
            ("misspelled request_id", '{"misspelled_request_id": "f0a7fa67-8997-4ea6-8a0e-c81799209f71"}'),
        ]

        for test_name, event_body in sub_tests:
            with self.subTest(test_name):
                # Arrange
                event = self.get_lambda_event()
                context = None
                event['body'] = event_body

                # Act
                result = index.lambda_handler(event, context)

                # Assert
                self.assertEqual(400, result['statusCode'])

    def test_returns_400_when_request_id_is_invalid(self):
        # Arrange
        event = self.get_lambda_event_with_invalid_request_id()
        context = None

        # Act
        result = index.lambda_handler(event, context)

        # Assert
        self.assertEqual(400, result['statusCode'])

    def test_lambda_returns_an_error_code_when_s3_raises_exception(self):
        sub_tests = [
            # A few exceptions which may be raised by S3, not a comprehensive list:
            ("NoSuchKey exception", self.get_s3_no_such_key_exception(), 404),
            ("AccessDenied exception", self.get_s3_access_denied_exception(), 403),
        ]

        for test_name, exception, error_code in sub_tests:
            with self.subTest(test_name):
                # Arrange
                event = self.get_lambda_event()
                context = None
                index.s3_client.get_object.side_effect = exception

                # Act
                result = index.lambda_handler(event, context)

                # Assert
                self.assertEqual(error_code, result['statusCode'])

    @staticmethod
    def get_lambda_event():
        """
        Get a sample Lambda event.

        Note:
            The only field used by the Lambda is event['body']. Despite this, the whole event is used for the unit tests
            to ensure the Lambda has correct behavior when provided a realistic event.
        """
        return {
            'resource': '/identity/fblogincheck',
            'path': '/identity/fblogincheck',
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
                'Via': '2.0 ce71f64ad5dca81beca846466f2d5008.cloudfront.net (CloudFront)',
                'X-Amz-Cf-Id': 'jHDS1YmB070nMchYAw7dUhtpeEgkCuugaoVMZ3MKQT8tfJ2R6ImKIg==',
                'X-Amzn-Trace-Id': 'Root=1-60e77918-6d42b88f7912b97934b463ce',
                'X-Forwarded-For': '12.123.123.12, 56.567.567.56',
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
                'Via': ['2.0 ce71f64ad5dca81beca846466f2d5008.cloudfront.net (CloudFront)'],
                'X-Amz-Cf-Id': ['jHDS1YmB070nMchYAw7dUhtpeEgkCuugaoVMZ3MKQT8tfJ2R6ImKIg=='],
                'X-Amzn-Trace-Id': ['Root=1-60e77918-6d42b88f7912b97934b463ce'],
                'X-Forwarded-For': ['12.123.123.12, 56.567.567.56'],
                'X-Forwarded-Port': ['443'],
                'X-Forwarded-Proto': ['https']
            },
            'queryStringParameters': None,
            'multiValueQueryStringParameters': None,
            'pathParameters': None,
            'stageVariables': None,
            'requestContext': {
                'resourceId': 'i9buwo',
                'resourcePath': '/identity/fblogincheck',
                'httpMethod': 'POST',
                'extendedRequestId': 'CK_b7FNnPHcF0qw=',
                'requestTime': '08/Jul/2021:22:15:52 +0000',
                'path': '/dev/identity/fblogincheck',
                'accountId': '123456789012',
                'protocol': 'HTTP/1.1',
                'stage': 'dev',
                'domainPrefix': '123abcdefg',
                'requestTimeEpoch': 1625782552978,
                'requestId': '6c7d0197-2676-43f0-a544-f93b835cab4e',
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
            'resource': '/identity/fblogincheck',
            'path': '/identity/fblogincheck',
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
                'Via': '2.0 ce71f64ad5dca81beca846466f2d5008.cloudfront.net (CloudFront)',
                'X-Amz-Cf-Id': 'jHDS1YmB070nMchYAw7dUhtpeEgkCuugaoVMZ3MKQT8tfJ2R6ImKIg==',
                'X-Amzn-Trace-Id': 'Root=1-60e77918-6d42b88f7912b97934b463ce',
                'X-Forwarded-For': '12.123.123.12, 56.567.567.56',
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
                'Via': ['2.0 ce71f64ad5dca81beca846466f2d5008.cloudfront.net (CloudFront)'],
                'X-Amz-Cf-Id': ['jHDS1YmB070nMchYAw7dUhtpeEgkCuugaoVMZ3MKQT8tfJ2R6ImKIg=='],
                'X-Amzn-Trace-Id': ['Root=1-60e77918-6d42b88f7912b97934b463ce'],
                'X-Forwarded-For': ['12.123.123.12, 56.567.567.56'],
                'X-Forwarded-Port': ['443'],
                'X-Forwarded-Proto': ['https']
            },
            'queryStringParameters': None,
            'multiValueQueryStringParameters': None,
            'pathParameters': None,
            'stageVariables': None,
            'requestContext': {
                'resourceId': 'i9buwo',
                'resourcePath': '/identity/fblogincheck',
                'httpMethod': 'POST',
                'extendedRequestId': 'CK_b7FNnPHcF0qw=',
                'requestTime': '08/Jul/2021:22:15:52 +0000',
                'path': '/dev/identity/fblogincheck',
                'accountId': '123456789012',
                'protocol': 'HTTP/1.1',
                'stage': 'dev',
                'domainPrefix': '123abcdefg',
                'requestTimeEpoch': 1625782552978,
                'requestId': '6c7d0197-2676-43f0-a544-f93b835cab4e',
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
    def get_s3_completion_object():
        """
        Get a sample S3 object that signals the Facebook login is complete.
        """
        return {
            'ResponseMetadata': {
                'RequestId': 'A20N3KPJP8TCHDE4',
                'HostId': '/7tkvcV0U3eCKZ99dXiBApBNRs/PXPafgsMj/NYs8uv8S9c+doqkqXZeWSOkEa8oaeGqVQgM1uU=',
                'HTTPStatusCode': 200,
                'HTTPHeaders': {
                    'x-amz-id-2': '/7tkvcV0U3eCKZ99dXiBApBNRs/PXPafgsMj/NYs8uv8S9c+doqkqXZeWSOkEa8oaeGqVQgM1uU=',
                    'x-amz-request-id': 'A20N3KPJP8TCHDE4',
                    'date': 'Fri, 09 Jul 2021 10:15:59 GMT',
                    'last-modified': 'Fri, 09 Jul 2021 10:15:57 GMT',
                    'x-amz-expiration': 'expiry-date="Sun, 11 Jul 2021 00:00:00 GMT", rule-id="MzRhMDNjNmEtYzFjYy00NjlmLWE1ZGYtZTAxMzU1NTMyYzc3"',
                    'etag': '"0148cae6a7669f79518035f99e0b0736"',
                    'accept-ranges': 'bytes',
                    'content-type': 'binary/octet-stream',
                    'server': 'AmazonS3',
                    'content-length': '440'
                },
                'RetryAttempts': 0
            },
            'AcceptRanges': 'bytes',
            'Expiration': 'expiry-date="Sun, 11 Jul 2021 00:00:00 GMT", rule-id="MzRhMDNjNmEtYzFjYy00NjlmLWE1ZGYtZTAxMzU1NTMyYzc3"',
            'LastModified': datetime.datetime(2021, 7, 9, 10, 15, 57, tzinfo=tzutc()),
            'ContentLength': 440,
            'ETag': '"0148cae6a7669f79518035f99e0b0736"',
            'ContentType': 'binary/octet-stream',
            'Metadata': {},

            # This should be a StreamingBody object instead of a string, but it's difficult to create one.
            # Instead, the Body is mocked wherever it's used in the unit tests.
            'Body': '<botocore.response.StreamingBody object at 0x7f167719e250>'
        }

    @staticmethod
    def get_s3_no_such_key_exception():
        exception = new_boto_exception(botocore.exceptions.ClientError)
        exception.__dict__['response'] = {
            'Error': {
                'Code': 'NoSuchKey',
                'Message': 'The specified key does not exist.',
                'Key': '123abc'
            },
            'ResponseMetadata': {
                'RequestId': 'Y40NPSEG9RYQF62Q',
                'HostId': 't6ZnBMCvoebGEfV1zOkOpyukfP8SSzmLOqUGvj3ct/elbGSD2I6/DHt1OIBJK92jHm8fFZXUlWU=',
                'HTTPStatusCode': 404,
                'HTTPHeaders': {
                    'x-amz-request-id': 'Y40NPSEG9RYQF62Q',
                    'x-amz-id-2': 't6ZnBMCvoebGEfV1zOkOpyukfP8SSzmLOqUGvj3ct/elbGSD2I6/DHt1OIBJK92jHm8fFZXUlWU=',
                    'content-type': 'application/xml',
                    'transfer-encoding': 'chunked',
                    'date': 'Fri, 09 Jul 2021 17:00:55 GMT',
                    'server': 'AmazonS3'
                },
                'RetryAttempts': 0
            }
        }
        return exception

    @staticmethod
    def get_s3_access_denied_exception():
        exception = new_boto_exception(botocore.exceptions.ClientError)
        exception.__dict__['response'] = {
            'Error': {
                'Code': 'AccessDenied',
                'Message': 'Access Denied'
            },
            'ResponseMetadata': {
                'RequestId': 'FH5AGEPWV4PX2E2F',
                'HostId': 'Y6G3xTFXrbNXR3m+Wb20fJyYzS9NiAiZEGi8gQMNqyr5uKmuVBb+GB/792TsTdk7TOuU8/NMIjA=',
                'HTTPStatusCode': 403,
                'HTTPHeaders': {
                    'x-amz-request-id': 'FH5AGEPWV4PX2E2F',
                    'x-amz-id-2': 'Y6G3xTFXrbNXR3m+Wb20fJyYzS9NiAiZEGi8gQMNqyr5uKmuVBb+GB/792TsTdk7TOuU8/NMIjA=',
                    'content-type': 'application/xml',
                    'transfer-encoding': 'chunked',
                    'date': 'Fri, 09 Jul 2021 17:49:10 GMT',
                    'server': 'AmazonS3'
                },
                'RetryAttempts': 1
            }
        }
        return exception
