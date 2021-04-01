# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

from typing import Dict, List, Optional


# Define Types:
StrDict = Dict[str, str]
MultiValueStrDict = Dict[str, List[str]]


def http_event(http_method: str = 'GET',
               player_id: str = 'foo_player_id',
               headers: StrDict = None,
               path_parameters: StrDict = None,
               query_string_parameters: StrDict = None,
               body: str = None) -> dict:

    if headers is None:
        headers = {}

    return {
        'resource': 'foo_resource',
        'path': 'foo_path',
        'httpMethod': http_method,
        'headers': {
            'Accept': '*/*',
            'Accept-Encoding': 'gzip, deflate, br',
            'Authorization': '123abc',
            'Host': '123abcdefg.execute-api.us-west-2.amazonaws.com',
            'Postman-Token': 'f22256ac-0fab-4fc8-a15b-cdd7b2e32aa8',
            'User-Agent': 'PostmanRuntime/7.28.3',
            'X-Amzn-Trace-Id': 'Root=1-61170a71-57463b427d347a5e686548b6',
            'X-Forwarded-For': '123.123.123.123',
            'X-Forwarded-Port': '443',
            'X-Forwarded-Proto': 'https',
            **headers
        },
        'multiValueHeaders': {
            'Accept': [
                '*/*'
            ],
            'Accept-Encoding': [
                'gzip, deflate, br'
            ],
            'Authorization': [
                '123abc'
            ],
            'Host': [
                '123abcdefg.execute-api.us-west-2.amazonaws.com'
            ],
            'Postman-Token': [
                'f22256ac-0fab-4fc8-a15b-cdd7b2e32aa8'
            ],
            'User-Agent': [
                'PostmanRuntime/7.28.3'
            ],
            'X-Amzn-Trace-Id': [
                'Root=1-61170a71-57463b427d347a5e686548b6'
            ],
            'X-Forwarded-For': [
                '123.123.123.123'
            ],
            'X-Forwarded-Port': [
                '443'
            ],
            'X-Forwarded-Proto': [
                'https'
            ],
            **_convert_to_multi_value_dict(headers)
        },
        'queryStringParameters': query_string_parameters,
        'multiValueQueryStringParameters': _convert_to_multi_value_dict(query_string_parameters),
        'pathParameters': path_parameters,
        'stageVariables': None,
        'requestContext': {
            'resourceId': 'aonh33',
            'authorizer': {
                'claims': {
                    'sub': 'bd59fff7-268d-4024-92e4-622566855ad4',
                    'iss': 'https://cognito-idp.us-west-2.amazonaws.com/us-west-2_JWQExKpsh',
                    'cognito:username': 'foo_username',
                    'origin_jti': '64622d67-0935-46bf-9263-5687f799e68e',
                    'aud': '6sjnfgjkg9ut5cdisc3r6fcugk',
                    'event_id': '8bf1c93e-db8e-4fda-b10a-b1189fdece47',
                    'token_use': 'id',
                    'auth_time': '1628899824',
                    'custom:gk_user_id': player_id,
                    'exp': 'Sat Aug 14 01:10:24 UTC 2021',
                    'iat': 'Sat Aug 14 00:10:24 UTC 2021',
                    'jti': '2d407015-100a-4476-bfb7-0fa5dd59eab8',
                    'email': 'foo@bar.com'
                }
            },
            'resourcePath': 'foo_resource_path',
            'httpMethod': http_method,
            'extendedRequestId': 'EB6RvE3lPHcFWig=',
            'requestTime': '14/Aug/2021:00:12:33 +0000',
            'path': '/dev/foo_path',
            'accountId': '123456789012',
            'protocol': 'HTTP/1.1',
            'stage': 'dev',
            'domainPrefix': '123abcdefg',
            'requestTimeEpoch': 1628899953352,
            'requestId': '286451ef-6462-4d6a-a674-0a74236f1ccc',
            'identity': {
                'cognitoIdentityPoolId': None,
                'accountId': None,
                'cognitoIdentityId': None,
                'caller': None,
                'sourceIp': '123.123.123.123',
                'principalOrgId': None,
                'accessKey': None,
                'cognitoAuthenticationType': None,
                'cognitoAuthenticationProvider': None,
                'userArn': None,
                'userAgent': 'PostmanRuntime/7.28.3',
                'user': None
            },
            'domainName': '123abcdefg.execute-api.us-west-2.amazonaws.com',
            'apiId': '123abcdefg'
        },
        'body': body,
        'isBase64Encoded': False
    }


def _convert_to_multi_value_dict(single_value_dict: StrDict = None) -> Optional[MultiValueStrDict]:
    if single_value_dict is None:
        return None

    return {
        key: [value]
        for key, value in single_value_dict.items()
    }
