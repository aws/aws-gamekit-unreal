# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

import os
from unittest import TestCase
from unittest.mock import patch
from layers.main.CommonLambdaLayer.python.gamekithelpers.handler_request import (
    log_event
)
import logging


# Patch Lambda environment variables:
@patch.dict(os.environ, {
    'DETAILED_LOGGING_DISABLED': 'false'},
            clear=True)
class TestHandlerRequest(TestCase):
    def test_log_event_disabled_logging_no_logs(self):
        # arrange
        os.environ['DETAILED_LOGGING_DISABLED'] = 'true'
        demo_logger = logging.getLogger()
        demo_logger.setLevel(logging.INFO)
        # act
        with self.assertLogs() as logs_captured:
            # Test log needed to trigger logger at least once otherwise self.assertLogs errors out
            # This is a known issue which will be resolved in future version with assertNoLogs
            demo_logger.info("TEST LOG")
            log_event(self.get_lambda_event())
        # assert
        # The one log count below is the one we logged above. This tests no other logs from method.
        self.assertEqual(1, len(logs_captured.records))

    def test_log_event_missing_env_var_log_exists(self):
        # arrange
        os.environ.pop('DETAILED_LOGGING_DISABLED', None)
        # act
        with self.assertLogs() as logs_captured:
            log_event(self.get_lambda_event())
        # assert
        self.assertEqual(1, len(logs_captured.records))

    def test_log_event_env_var_false_log_exists(self):
        # arrange (Nothing here - default scenario)
        # act
        with self.assertLogs() as logs_captured:
            log_event(self.get_lambda_event())
        # assert
        self.assertEqual(1, len(logs_captured.records))


    @staticmethod
    def get_lambda_event():
        return {
            'resource': '/achievements/admin',
            'path': '/achievements/admin/',
            'httpMethod': 'POST',
            'headers': {
                'Accept': '*/*',
                'Accept-Encoding': 'gzip, deflate, br',
                'Content-Type': 'application/json',
                'Host': 'abcdefghij.execute-api.us-west-2.amazonaws.com',
                'User-Agent': 'TestAgent',
                'X-Amzn-Trace-Id': 'Root=1-61003a02-7e1356b05a1e1569614c0c46',
                'X-Forwarded-For': '127.0.0.1',
                'X-Forwarded-Port': '443',
                'X-Forwarded-Proto': 'https'
            },
            'multiValueHeaders': {
                'Accept': ['*/*'],
                'Accept-Encoding': ['gzip, deflate, br'],
                'Content-Type': ['application/json'],
                'Host': ['abcdefghij.execute-api.us-west-2.amazonaws.com'],
                'User-Agent': ['TestAgent'],
                'X-Amzn-Trace-Id': ['Root=1-61003a02-7e1356b05a1e1569614c0c46'],
                'X-Forwarded-For': ['127.0.0.1'],
                'X-Forwarded-Port': ['443'],
                'X-Forwarded-Proto': ['https']
            },
            'queryStringParameters': {'limit': '50', 'use_consistent_read': 'true'},
            'multiValueQueryStringParameters': None,
            'pathParameters': None,
            'stageVariables': None,
            'requestContext': {
                'resourceId': 'abcdef',
                'authorizer': {
                    'claims': {
                        'sub': '12345678-1234-1234-1234-123456789012',
                        'iss': 'https://cognito-idp.us-west-2.amazonaws.com/us-west-2_123456789',
                        'cognito:username': 'jrnic',
                        'origin_jti': '12345678-1234-1234-1234-123456789012',
                        'aud': '7s24tlabcn8n0defbfoghijsgn',
                        'event_id': '6234d920-b637-4cdf-bd44-3a5e53f51569',
                        'token_use': 'id',
                        'auth_time': '1627438909',
                        'custom:gk_user_id': '12345678-1234-1234-1234-123456789012',
                        'exp': 'Wed Jul 28 03:21:49 UTC 2021',
                        'iat': 'Wed Jul 28 02:21:49 UTC 2021',
                        'jti': '7s24tlabcn8n0defbfoghijsgn',
                        'email': 'xyz@abc.def'
                    }
                },
                'domainName': 'abcdefghij.execute-api.us-west-2.amazonaws.com',
                'apiId': 'abcdefghij'
            },
            'body': None,
            'isBase64Encoded': False
        }