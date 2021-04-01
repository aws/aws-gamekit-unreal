# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

from datetime import datetime
from dateutil.tz import tzlocal
import json
import os
from unittest import TestCase
from unittest.mock import patch, MagicMock

with patch("boto3.client") as boto_client_mock:
    from functions.identity.CognitoFbCallbackHandler import index


# Patch Lambda environment variables:
@patch.dict(os.environ, {
    'APP_CLIENT_ID': '1234567890abcdefghijklmnop',
    'BOOTSTRAP_BUCKET': 'do-not-delete-gamekit-dev-123456789012-foogamename',
    'GAMEKIT_KMS_KEY_ARN': 'arn:aws:kms:us-west-2:123456789012:key/12345678-abcd-1234-abcd-123456789012',
    'GAMEKIT_KMS_KEY_ID': '12345678-abcd-1234-abcd-123456789012',
    'IDENTITY_TABLE_NAME': 'gamekit_dev_foogamename_identities',
    'REDIRECT_URI': 'https://123abcdefg.execute-api.us-west-2.amazonaws.com/dev/identity/fbtokenresp',
    'USER_POOL_DOMAIN': 'https://gamekit-dev-foogamename.auth.us-west-2.amazoncognito.com',
    'USER_POOL_ID': 'us-west-2_123456789',
})
class TestIndex(TestCase):
    def setUp(self):
        index.s3_client = MagicMock()
        index.cidp_client = MagicMock()
        index.dynamodb_client = MagicMock()

    @patch('functions.identity.CognitoFbCallbackHandler.index._get_user_info')
    @patch('functions.identity.CognitoFbCallbackHandler.index._check_object_exists_in_s3')
    @patch('functions.identity.CognitoFbCallbackHandler.index.crypto.decrypt_blob')
    @patch('functions.identity.CognitoFbCallbackHandler.index._exchange_code_for_access_token')
    @patch('functions.identity.CognitoFbCallbackHandler.index.ddb.boto3')
    def test_can_link_facebook_account_to_existing_user_successfully(self,
                                                                     mock_gamekithelpers_ddb_boto3,
                                                                     mock_exchange_code_for_access_token,
                                                                     mock_decrypt_blob,
                                                                     check_object_exists_in_s3,
                                                                     mock_get_user_info):
        # Arrange
        event = self.get_lambda_event()
        context = None
        mock_exchange_code_for_access_token.return_value = (self.get_exchange_code_for_access_token())
        mock_decrypt_blob.return_value = (True, self.get_decrypted_state_for_existing_user())
        check_object_exists_in_s3.return_value = False
        mock_ddb_table = mock_gamekithelpers_ddb_boto3.resource('dynamodb').Table()
        mock_ddb_table.get_item.return_value = {
            'Item': {
                "created_at": "2021-07-06T20:22:25.799025+00:00",
                "gk_user_hash_key": "1Hk6RU80eQvVpg9yyWaNIdrbmSvejd1hVfeVFl0woeQ=",
                "gk_user_id": "0d386660-8f62-4da7-b31c-43ab1237c540",
                "gk_user_id_hash": "nnk9/21CoT8BEmWT7o/XZPtb7FJPdXFC5hUilV7W0fA=",
                "updated_at": "2021-07-06T20:22:25.799025+00:00"
            }
        }
        mock_get_user_info.return_value = {
            'sub': 'abcdefgh-1234-1234-1234-abcdefghijkl',
            'username': 'Facebook_123456789012345'
        }
        index.cidp_client.admin_get_user.return_value = self.get_cognito_user_info()

        # Act
        result = index.lambda_handler(event, context)

        # Assert
        self.assertEqual(200, result['statusCode'])
        mock_ddb_table.update_item.assert_called_once()

    @patch('functions.identity.CognitoFbCallbackHandler.index._write_object_to_s3')
    @patch('functions.identity.CognitoFbCallbackHandler.index._get_user_info')
    @patch('functions.identity.CognitoFbCallbackHandler.index._check_object_exists_in_s3')
    @patch('functions.identity.CognitoFbCallbackHandler.index._exchange_code_for_access_token')
    @patch('functions.identity.CognitoFbCallbackHandler.index.crypto.decrypt_blob')
    @patch('functions.identity.CognitoFbCallbackHandler.index.ddb.boto3')
    @patch('functions.identity.CognitoFbCallbackHandler.index.crypto.boto3')
    def test_can_create_account_for_new_user_successfully(self,
                                                          mock_gamekithelpers_crypto_boto3,
                                                          mock_gamekithelpers_ddb_boto3,
                                                          mock_decrypt_blob,
                                                          mock_exchange_code_for_access_token,
                                                          check_object_exists_in_s3,
                                                          mock_get_user_info,
                                                          write_object_to_s3):
        # Arrange
        event = self.get_lambda_event()
        context = None
        mock_exchange_code_for_access_token.return_value = (self.get_exchange_code_for_access_token())
        check_object_exists_in_s3.return_value = False
        mock_get_user_info.return_value = {
            'sub': 'abcdefgh-1234-1234-1234-abcdefghijkl',
            'username': 'Facebook_123456789012345'
        }
        index.cidp_client.admin_get_user.return_value = self.get_cognito_user_info()
        mock_ddb_table = mock_gamekithelpers_ddb_boto3.resource('dynamodb').Table()
        mock_ddb_table.query.return_value = self.get_dynamodb_query_response()
        mock_gamekithelpers_crypto_boto3.client('kms').generate_data_key.return_value = {
            'CiphertextBlob': b'\x01\x02\x03\x00x\xdd\xc7vu\x9d\xf2\x80]\x98\xa0\\\x86\xfb\xd7QM\x94$,j\xa6a~\x1f\xb9\xd5\xb3\x1b\xc8\xf6\xe7\xf7\x01\x02\xbb\xac\x131\xd2\xf4\x8aw\xd0E\xcd\xda\xca`N\x00\x00\x00~0|\x06\t*\x86H\x86\xf7\r\x01\x07\x06\xa0o0m\x02\x01\x000h\x06\t*\x86H\x86\xf7\r\x01\x07\x010\x1e\x06\t`\x86H\x01e\x03\x04\x01.0\x11\x04\x0c@\x14\xdb\xf3\x93n\xab\x9b;\x0f\xe4s\x02\x01\x10\x80;7\x1e\x7fNr\x9f>bI*\x05C\xe9;\xeb\xf6\xce\x17Sn\xd2\x90x\x98\xa7\x8c\x0b\x0b\xa4U+t\xe6\xe8T\x16\xaf~)?fN\xa4\xc7\t\x02\x892\xfa\xfc\x18\xccCDUk\xd9\xd24',
            'Plaintext': b'\x9ff$\xca\xe6;=;\x02\xe1[\xce\xc1\xda\xa0&\x81M\x83\xb4oK~q>\xf4\xde\xc2\x91m\xca\x12',
        }
        mock_decrypt_blob.return_value = (True, self.get_decrypted_state_for_new_user())

        # Act
        result = index.lambda_handler(event, context)

        # Assert
        self.assertEqual(200, result['statusCode'])
        mock_ddb_table.put_item.assert_called_once()
        self.assertEqual(2, write_object_to_s3.call_count)

    def test_lambda_returns_a_400_error_code_when_the_querystring_state_is_missing(self):
        # Arrange
        event = self.get_lambda_event()
        context = None

        # Simulate the case when 'state' is completely missing from the queryStringParameters:
        event['queryStringParameters'].pop('state')

        # Act
        result = index.lambda_handler(event, context)

        # Assert
        self.assertEqual(400, result['statusCode'])
        self.assert_did_not_write_to_dynamodb(index.dynamodb_client)

    def test_lambda_returns_a_400_error_code_when_the_querystring_state_is_empty(self):
        sub_tests = [
            ("None", None),
            ("empty string", ''),
            ("one whitespace character", ' '),
            ("many whitespace characters", '   '),
        ]

        for test_name, state in sub_tests:
            with self.subTest(test_name):
                # Arrange
                event = self.get_lambda_event()
                context = None

                event['queryStringParameters']['state'] = state

                # Act
                result = index.lambda_handler(event, context)

                # Assert
                self.assertEqual(400, result['statusCode'])
                self.assert_did_not_write_to_dynamodb(index.dynamodb_client)

    @patch('functions.identity.CognitoFbCallbackHandler.index.crypto.decrypt_blob')
    @patch('functions.identity.CognitoFbCallbackHandler.index._exchange_code_for_access_token')
    def test_lambda_returns_an_error_code_when_the_decrypted_state_is_malformed(self,
                                                                                mock_exchange_code_for_access_token,
                                                                                mock_decrypt_blob):
        sub_tests = [
            ("missing request_id", 400, b'{"expiration": 9999999999, "source_ip": "12.123.123.12"}'),
            ("missing expiration", 400, b'{"request_id": "7b8961cb-19db-47e3-88cb-afcd9884b81d", '
                                        b'"source_ip": "12.123.123.12"}'),
            ("missing source_ip", 400, b'{"request_id": "7b8961cb-19db-47e3-88cb-afcd9884b81d",'
                                       b'"expiration": 9999999999}'),
            ("source_ip mismatch", 400, b'{"request_id": "7b8961cb-19db-47e3-88cb-afcd9884b81d",'
                                        b'"expiration": 9999999999, "source_ip": "12.123.123.123"}'),
            ("request has expired", 403, b'{"request_id": "7b8961cb-19db-47e3-88cb-afcd9884b81d", '
                                         b'"source_ip": "12.123.123.12", "expiration": 100}'),
        ]

        for test_name, error_code, decrypted_state in sub_tests:
            with self.subTest(test_name):
                # Arrange
                event = self.get_lambda_event()
                context = None
                mock_exchange_code_for_access_token.return_value = (self.get_exchange_code_for_access_token())
                mock_decrypt_blob.return_value = (True, decrypted_state)

                # Act
                result = index.lambda_handler(event, context)

                # Assert
                self.assertEqual(error_code, result['statusCode'])
                self.assert_did_not_write_to_dynamodb(index.dynamodb_client)

    @patch('functions.identity.CognitoFbCallbackHandler.index._check_object_exists_in_s3')
    @patch('functions.identity.CognitoFbCallbackHandler.index.crypto.decrypt_blob')
    @patch('functions.identity.CognitoFbCallbackHandler.index._exchange_code_for_access_token')
    def test_lambda_returns_a_200_success_code_when_the_completion_callback_file_already_exists(self,
                                                                                                mock_exchange_code_for_access_token,
                                                                                                mock_decrypt_blob,
                                                                                                check_object_exists_in_s3):
        # Arrange
        event = self.get_lambda_event()
        context = None
        mock_exchange_code_for_access_token.return_value = (self.get_exchange_code_for_access_token())
        mock_decrypt_blob.return_value = (True, self.get_decrypted_state_for_new_user())
        check_object_exists_in_s3.return_value = True

        # Act
        result = index.lambda_handler(event, context)

        # Assert
        self.assertEqual(200, result['statusCode'])
        self.assert_did_not_write_to_dynamodb(index.dynamodb_client)

    @staticmethod
    def assert_did_not_write_to_dynamodb(mock_dynamodb: MagicMock):
        mock_dynamodb.put_item.assert_not_called()
        mock_dynamodb.update_item.assert_not_called()

    @staticmethod
    def get_lambda_event():
        return {
            'resource': '/identity/fbtokenresp',
            'path': '/identity/fbtokenresp',
            'httpMethod': 'GET',
            'headers': {
                'Accept': 'text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8',
                'Accept-Encoding': 'gzip,deflate,br',
                'Accept-Language': 'en-US,en;q=0.5',
                'CloudFront-Forwarded-Proto': 'https',
                'CloudFront-Is-Desktop-Viewer': 'true',
                'CloudFront-Is-Mobile-Viewer': 'false',
                'CloudFront-Is-SmartTV-Viewer': 'false',
                'CloudFront-Is-Tablet-Viewer': 'false',
                'CloudFront-Viewer-Country': 'US',
                'Host': '123abcdefg.execute-api.us-west-2.amazonaws.com',
                'Referer': 'https://www.facebook.com/v11.0/dialog/oauth/read/',
                'upgrade-insecure-requests': '1',
                'User-Agent': 'Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:78.0) Gecko/20100101 Firefox/78.0',
                'Via': '2.0 4f3feb5c4393987d42d1971d404d7cea.cloudfront.net (CloudFront)',
                'X-Amz-Cf-Id': 'hczAVi_Amqcn7Ag9TMxZvEQrrnsLBqBayXWnGvRU3_9cHncFCbyduw==',
                'X-Amzn-Trace-Id': 'Root=1-60e7790d-329f1f724ba7741850373cfa',
                'X-Forwarded-For': '12.123.123.12, 56.567.567.567',
                'X-Forwarded-Port': '443',
                'X-Forwarded-Proto': 'https'
            },
            'multiValueHeaders': {
                'Accept': ['text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8'],
                'Accept-Encoding': ['gzip,deflate,br'],
                'Accept-Language': ['en-US,en;q=0.5'],
                'CloudFront-Forwarded-Proto': ['https'],
                'CloudFront-Is-Desktop-Viewer': ['true'],
                'CloudFront-Is-Mobile-Viewer': ['false'],
                'CloudFront-Is-SmartTV-Viewer': ['false'],
                'CloudFront-Is-Tablet-Viewer': ['false'],
                'CloudFront-Viewer-Country': ['US'],
                'Host': ['123abcdefg.execute-api.us-west-2.amazonaws.com'],
                'Referer': ['https://www.facebook.com/v11.0/dialog/oauth/read/'],
                'upgrade-insecure-requests': ['1'],
                'User-Agent': ['Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:78.0) Gecko/20100101 Firefox/78.0'],
                'Via': ['2.0 4f3feb5c4393987d42d1971d404d7cea.cloudfront.net (CloudFront)'],
                'X-Amz-Cf-Id': ['hczAVi_Amqcn7Ag9TMxZvEQrrnsLBqBayXWnGvRU3_9cHncFCbyduw=='],
                'X-Amzn-Trace-Id': ['Root=1-60e7790d-329f1f724ba7741850373cfa'],
                'X-Forwarded-For': ['12.123.123.12, 56.567.567.567'],
                'X-Forwarded-Port': ['443'],
                'X-Forwarded-Proto': ['https']
            },
            'queryStringParameters': {
                'code': '470d5e7e-2f29-4e12-a83d-221605d5a7df',
                'state': 'AAAAuAECAwB43cd2dZ3ygF2YoFyG 9dRTZQkLGqmYX4fudWzG8j25/cB62gLCvpghdqhcaSxP TQtAAAAH4wfAYJKoZIhvcNAQcGoG8wbQIBADBoBgkqhkiG9w0BBwEwHgYJYIZIAWUDBAEuMBEEDMH29Xiv6ZiQIhNXGQIBEIA72fPQlMq7xB/M66hp0GGQqysQtKbpUdCPT0Hf1EcRRGRfYcWF2IbQnI57u1JYYjIOhv6A0a/yOCwK/kRnQUFBQUFCZzUzajY0REgydzBwbmhRSEhweGRNckx2NTdjVFVRaVNUNmhjNFRJOFJLcVlLaE40X1JCQVhPTi1NNVVGZWZyNS1aZ0dWb3cyVkpYMEJBRDNyeXJDZ3NYbTZRN0haZWdhM19LTGZpeWJDNHY2UHQyVFViNmdDUzhOdDBMNWk3cGhhcGVTSXpJcEQ2NV81VVZJek5icHdTWVhLUTFRekJfaEtBWUljbGg5S3ViWnZLMV9DN1VKb3NGdllPcVlVT1l2dDZYekw='
            },
            'multiValueQueryStringParameters': {
                'code': ['470d5e7e-2f29-4e12-a83d-221605d5a7df'],
                'state': ['AAAAuAECAwB43cd2dZ3ygF2YoFyG 9dRTZQkLGqmYX4fudWzG8j25/cB62gLCvpghdqhcaSxP TQtAAAAH4wfAYJKoZIhvcNAQcGoG8wbQIBADBoBgkqhkiG9w0BBwEwHgYJYIZIAWUDBAEuMBEEDMH29Xiv6ZiQIhNXGQIBEIA72fPQlMq7xB/M66hp0GGQqysQtKbpUdCPT0Hf1EcRRGRfYcWF2IbQnI57u1JYYjIOhv6A0a/yOCwK/kRnQUFBQUFCZzUzajY0REgydzBwbmhRSEhweGRNckx2NTdjVFVRaVNUNmhjNFRJOFJLcVlLaE40X1JCQVhPTi1NNVVGZWZyNS1aZ0dWb3cyVkpYMEJBRDNyeXJDZ3NYbTZRN0haZWdhM19LTGZpeWJDNHY2UHQyVFViNmdDUzhOdDBMNWk3cGhhcGVTSXpJcEQ2NV81VVZJek5icHdTWVhLUTFRekJfaEtBWUljbGg5S3ViWnZLMV9DN1VKb3NGdllPcVlVT1l2dDZYekw=']
            },
            'pathParameters': None,
            'stageVariables': None,
            'requestContext': {
                'resourceId': 'xxrv33',
                'resourcePath': '/identity/fbtokenresp',
                'httpMethod': 'GET',
                'extendedRequestId': 'CK_aHFVZPHcFaAQ=',
                'requestTime': '08/Jul/2021:22:15:41 +0000',
                'path': '/dev/identity/fbtokenresp',
                'accountId': '123456789012',
                'protocol': 'HTTP/1.1',
                'stage': 'dev',
                'domainPrefix': '123abcdefg',
                'requestTimeEpoch': 1625782541323,
                'requestId': 'a82d27e1-f89c-4304-9c43-dfe5c2c4464f',
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
                    'userAgent': 'Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:78.0) Gecko/20100101 Firefox/78.0',
                    'user': None
                },
                'domainName': '123abcdefg.execute-api.us-west-2.amazonaws.com',
                'apiId': '123abcdefg'
            },
            'body': None,
            'isBase64Encoded': False
        }

    @staticmethod
    def get_exchange_code_for_access_token():
        token_resp_dict = {
            'id_token': 'eyJraWQiOiJxbmg3NlwvUjRHRnRkSmVZbTh3elFRc2cwXC9jUnZaemZtRDhLS2NRYXZvXC9FPSIsImFsZyI6IlJTMjU2In0.eyJhdF9oYXNoIjoicS0wbk81Zjdub1dkb1lBRVhCdUx6QSIsInN1YiI6ImEyNDJiNDNmLTdkYmEtNGY3OS05ZDgyLWZhMDg4MWQwNDRiNCIsImNvZ25pdG86Z3JvdXBzIjpbInVzLXdlc3QtMl9FMmxlWWR6ZUhfRmFjZWJvb2siXSwiaXNzIjoiaHR0cHM6XC9cL2NvZ25pdG8taWRwLnVzLXdlc3QtMi5hbWF6b25hd3MuY29tXC91cy13ZXN0LTJfRTJsZVlkemVIIiwiY29nbml0bzp1c2VybmFtZSI6IkZhY2Vib29rXzExNjkxNDQxMzk3ODAzNiIsIm9yaWdpbl9qdGkiOiIwY2FmMDI5Yi04M2Q1LTQxNTctYWNiYS1mYjMyNDNlOWUwNTAiLCJhdWQiOiI1N283NmhndWRjbmg4cWFhbTAyOG9uY29qNyIsImlkZW50aXRpZXMiOlt7InVzZXJJZCI6IjExNjkxNDQxMzk3ODAzNiIsInByb3ZpZGVyTmFtZSI6IkZhY2Vib29rIiwicHJvdmlkZXJUeXBlIjoiRmFjZWJvb2siLCJpc3N1ZXIiOm51bGwsInByaW1hcnkiOiJ0cnVlIiwiZGF0ZUNyZWF0ZWQiOiIxNjI1NzgyNTQwNjM0In1dLCJ0b2tlbl91c2UiOiJpZCIsImF1dGhfdGltZSI6MTYyNTkwMDI1MiwiZXhwIjoxNjI1OTAzODUyLCJpYXQiOjE2MjU5MDAyNTIsImp0aSI6IjQyZGZlMzNjLTIwYWUtNDQyZi05YTg0LWMxNWY5NzQ4Mzk1ZCJ9.G3s1nK76Yz9ancOi7BuRd-ZeJVcxuXqYPnbNDEvVukDlL0Yxj23kXeWYIxndnXyexs0IVgi3ml8OjW0POVBaMRFsVtXCkox-nAZXqcObeXYLm0gbbkiXjGl73nVWPl-lBZpB8dCTMpcPDFGwpP22hHbhHH85lYnelppzFQANikktFJ9SdQ0N_aknVIT8YrTkkiElOW0dWJqmqluIle5jb2lgdCfuJl50UyIBT7lt1lPLL6MPFvDA0u4aw6JfhNU7a5E7Ib4pFCW-LoN_Od_gPAfj6GRcY-RbWfGuHM88_apIblfVKcalEivO8O_U-ORFYJHw2pi3OchEKhKY2dFj_w',
            'access_token': 'eyJraWQiOiJvK3NVaXBRSXBQRnp0bFFWcGhweWV2bjQzdXd2cEhwZmNtWHRxSUZlT3BFPSIsImFsZyI6IlJTMjU2In0.eyJzdWIiOiJhMjQyYjQzZi03ZGJhLTRmNzktOWQ4Mi1mYTA4ODFkMDQ0YjQiLCJjb2duaXRvOmdyb3VwcyI6WyJ1cy13ZXN0LTJfRTJsZVlkemVIX0ZhY2Vib29rIl0sImlzcyI6Imh0dHBzOlwvXC9jb2duaXRvLWlkcC51cy13ZXN0LTIuYW1hem9uYXdzLmNvbVwvdXMtd2VzdC0yX0UybGVZZHplSCIsInZlcnNpb24iOjIsImNsaWVudF9pZCI6IjU3bzc2aGd1ZGNuaDhxYWFtMDI4b25jb2o3Iiwib3JpZ2luX2p0aSI6IjBjYWYwMjliLTgzZDUtNDE1Ny1hY2JhLWZiMzI0M2U5ZTA1MCIsInRva2VuX3VzZSI6ImFjY2VzcyIsInNjb3BlIjoiY29tLmF3cy5nZGtcL2dkay5pZGVudGl0eSBvcGVuaWQiLCJhdXRoX3RpbWUiOjE2MjU5MDAyNTIsImV4cCI6MTYyNTkwMzg1MiwiaWF0IjoxNjI1OTAwMjUyLCJqdGkiOiI5NGNkNjUzOS01NThjLTQ3NDQtYWI2OS1mZmNkMGE5MjlkMjEiLCJ1c2VybmFtZSI6IkZhY2Vib29rXzExNjkxNDQxMzk3ODAzNiJ9.bJHOk-gDO7jTklHZPslnwkPnyAClsbjzhEY4vDPZiYpMJYi5IdZp5HQkdeuSNK9DNzN_lx9TSdwVQdzarZhjqrWh7iojQUdc016Csb7X2nFSExNASUx4GmcuKTIZdJhXni5P1Wj_LEpNIsLc_j_FbYkzB3Kp6BtoNJRmjQ8V_lNcKTsjo0btgiuvwVuBWerQIrjoqQsD2vj9-JD929Fhuhl6BxGdNlywZC5oXndnt9oipxioEudqpDOapsWaKLrFb1yKvQsh4CCdWXG4S_9tIOUr-Pi74XMIQRDxR5d-TAJR8flTneZ5AHI1IIpX26zIPOCGwlZLi5BlvxkK25ylGw',
            'refresh_token': 'eyJjdHkiOiJKV1QiLCJlbmMiOiJBMjU2R0NNIiwiYWxnIjoiUlNBLU9BRVAifQ.I7gJohTTKwoHy4yWI-eYxcCXKzmNzt-ktKsqyxQBCOV_VJOP5MjFyfLtbvKo6c3IgrCzBsc9gAw9xnCBCLC5YVf4-dmXgu1QzX3PXIEanynIF2YeeEQ7ZxIkySk9ijgBC7TvXmTF4ayg5c1_rWwAi86DdEjvkqce5M174liWLnB7wrCYWNj7jTItyN2cPFFa4zn4757Uaz_zMhjtSKMRLJz6QAm0v0lD3RmPH1TM0i-KtoENONv6BlYX2zhuAB8ca5HBkL4opkyPTEfBKOgRuqSkCz2qpHgyjann5bueJVFt87Y932E98GvnxEQr6WcuLQDTEM_xQV_fmVKZ72unPw.JPLY9K1TEsUM58nW.iKNPzBiy6XLdLLCBcAtRVhnZsBKtL6PYNXbxOLiBl53N-L1-QAxAbEvASoFLd8Vodw7-ZaRL17YJUGLpeacHh93iMZN8sT-fheiuUf0vvlhaeJnSAJPIgjKzCmRiZiln1bHNJNQLlSP4t1S6S5IJUWtEZsDdAW-g5jaRGJtwej0e1mQcjcAt4fu5y4BWRqDzXx3bKZjql8aE1_tcrrRomk1qK4s5XCEGZSecvyBOuuqioVx0R5UvN3U3DTwcIfBH_SwGIfG71nvKPOL0FByffVvX4z3GtBl27yUDnDKDMxUd0sJLyCxBNq2aEVw6vNnxJPhbioI1-WsqdR5cqVFgwLSd0Xnc3UUGH3i7Td051VU8YmBIwpuyk0eVE8yn-eipXzpyjjIvwTbXc35xElUX-bOjSvXlKJPlN35Gxl2IL-Uz7yRn5YYwNO1mFqpRnujYyH86GPF6ZD2MxwD1-4AU6kpHmoxgfDRn3Rsyo6kNLiDgQEPjQIHFSMWIfOJPngibj7CEz9QRKTmps1f52c19vEJmwYWIEKLysuPsIAIrAO1Z2JDEpMtBBEqJKhofK7gl4I4E7LyF2dMSawQw3exbntoPdxXmi3a942aQU99b2CWeVoJLrJMs2eIo_974CZYqL2DMB1kQjkT_m3oBMyjAiO9kNeGh5jKY9GtDZevS1u8R6PRVhkUM8EVjHMaU0gjI3RVBhMFPgPmhzeP7QLjt2KA4IxPnRz9LFL2EX74po2xSGsKDHoZ_vLX-qVQxX6EXozQeF0bxkFT1sYtMMZM48ls5Dhc-f8AtxlU68nWg_OxAuvToq2_Q0XDjMX4fxmM2oG6hu-by60CKVOBShoe0quc0cp5OfbrbRDSpEZWvxk3RPzJ1cDOrsYVQ3I9WJmALFzYLXTS0-SIxz9QAe3CjVANOncGy0zV9BO_3EZvJ_ZevbUUibQKMrsiZUIm0gMIbCfhG-O0BBKovgZuGVJNaMW5_24THjaTKomA662sAI6OnCQ-gi0nJrYV5H3oeT05TbRDkE1AvgpkWWnOv84aJqANHqF1j26iOiTwWey3c7Debs8AC95QOnAbbx9f8iY6GHFUJUYDqAnzQD18BkHfQuTsEf1iyqvGCWhfv1LQZ9nGIA4PbJtE0aQa_Uc_qFI4T4HHzTbkM8iNgUldtwMrOE7ZkPq_dwP1EpCq5DDJkhNjx19xImOVdTR1sTnwbMB8gJh49QhkycJDJhtW_rBRJOd72fNpt1c8EGDUeBmo.t91CmUnRSxRj70drlQwZCQ',
            'expires_in': 3600,
            'token_type': 'Bearer'
        }
        token_resp = json.dumps(token_resp_dict)

        return token_resp, token_resp_dict

    @staticmethod
    def get_decrypted_state_for_new_user():
        """
        Get data for a new user who does not have a GameKit account yet. For example, they haven't registered with
        email and password yet and haven't signed in through an identity provider before (Amazon, Facebook, etc.).
        """
        state = {
            'request_id': '7b8961cb-19db-47e3-88cb-afcd9884b81d',
            'expiration': 9999999999,
            'source_ip': '12.123.123.12'
        }
        return json.dumps(state).encode('utf-8')

    @staticmethod
    def get_decrypted_state_for_existing_user():
        """
        Get data for a returning user who already has a GameKit account. For example, they've already registered with
        email and password or have already signed in through a different identity provider (ex: Amazon, Google, etc.).
        """
        state = {
            'request_id': '7b8961cb-19db-47e3-88cb-afcd9884b81d',
            'expiration': 9999999999,
            'source_ip': '12.123.123.12',
            'gk_user_id': '0d386660-8f62-4da7-b31c-43ab1237c540',
            'gk_user_id_hash': 'nnk9/21CoT8BEmWT7o/XZPtb7FJPdXFC5hUilV7W0fA=',
        }
        return json.dumps(state).encode('utf-8')

    @staticmethod
    def get_cognito_user_info():
        return {
            'Username': 'Facebook_123456789012345',
            'UserAttributes': [
                {
                    'Name': 'sub',
                    'Value': 'abcdefgh-1234-1234-1234-abcdefghijkl'
                },
                {
                    'Name': 'identities',
                    'Value': '[{"userId":"123456789012345","providerName":"Facebook","providerType":"Facebook","issuer":null,"primary":true,"dateCreated":1625782540634}]'
                },
                {
                    'Name': 'email_verified',
                    'Value': 'false'
                }
            ],
            'UserCreateDate': datetime(2021, 7, 8, 22, 15, 40, 640000, tzinfo=tzlocal()),
            'UserLastModifiedDate': datetime(2021, 7, 10, 6, 44, 29, 194000, tzinfo=tzlocal()),
            'Enabled': True,
            'UserStatus': 'EXTERNAL_PROVIDER',
            'ResponseMetadata': {
                'RequestId': '97c15be8-3951-4e97-a4ce-779e8de762ad',
                'HTTPStatusCode': 200,
                'HTTPHeaders': {
                    'date': 'Sat, 10 Jul 2021 06:57:38 GMT',
                    'content-type': 'application/x-amz-json-1.1',
                    'content-length': '473',
                    'connection': 'keep-alive',
                    'x-amzn-requestid': '97c15be8-3951-4e97-a4ce-779e8de762ad'
                },
                'RetryAttempts': 0
            }
        }

    @staticmethod
    def get_dynamodb_query_response():
        return {
            # No records found:
            'Items': [],
            'Count': 0,
            'ScannedCount': 0,

            # One record found (for future reference):
            # 'Items': [
            #     {
            #         'gk_user_id': {'S': '5bc8a5a0-96ad-4517-b52f-0c6b7180877c'},
            #         'facebook_external_id': {'S': '123456789012345'}
            #     }
            # ],
            # 'Count': 1,
            # 'ScannedCount': 1,

            'ResponseMetadata': {
                'RequestId': 'ELN6BCG2QAVK930L76PBVBB59NVV4KQNSO5AEMVJF66Q9ASUAAJG',
                'HTTPStatusCode': 200,
                'HTTPHeaders': {
                    'server': 'Server',
                    'date': 'Sat, 10 Jul 2021 06:57:38 GMT',
                    'content-type': 'application/x-amz-json-1.0',
                    'content-length': '146',
                    'connection': 'keep-alive',
                    'x-amzn-requestid': 'ELN6BCG2QAVK930L76PBVBB59NVV4KQNSO5AEMVJF66Q9ASUAAJG',
                    'x-amz-crc32': '3550124790'
                },
                'RetryAttempts': 0
            }
        }
