# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

import json

import boto3
from requests import request

"""
Use this script to call an API Gateway Endpoint.

PREREQUISITES:
* You've deployed the Identity/Authentication feature.
* You've created and "confirmed" a user.
* Your [default] AWS credentials are configured in `~/.aws/credentials`. See https://docs.aws.amazon.com/cli/latest/userguide/cli-configure-files.html#cli-configure-files-where
* You've installed the "requests" library to your Python environment. See https://docs.python-requests.org/en/master/
"""


# Get your Cognito App Client Id here:
# 1. Open: https://us-west-2.console.aws.amazon.com/cognito/home?region=us-west-2
# 2. Click: "Manager User Pools" > "<YOUR_POOL>" > App clients
COGNITO_APP_CLIENT_ID = '123abc'

# Copy from: https://us-west-2.console.aws.amazon.com/apigateway/main/apis?region=us-west-2
API_GATEWAY_API_ID = '123abc'

AWS_REGION = 'us-west-2'


def main():
    # Log-in the player and get their IdToken:
    username = 'foo'
    password = 'bar'
    id_token = get_id_token(username, password, COGNITO_APP_CLIENT_ID)

    # Configure the REST endpoint here:
    slot_name = 'autosave-01'
    time_to_live = '60'

    method = 'GET'
    rest_endpoint = f'game_saving/{slot_name}/upload_url?time_to_live={time_to_live}'

    headers = {
        'authorization': id_token,
    }
    body = json.dumps({
        'foo': 'bar',
    })

    # Call the endpoint:
    response = request(
        method=method,
        url=get_request_url(rest_endpoint, API_GATEWAY_API_ID, AWS_REGION),
        headers=headers,
        data=body,
    )

    log_response(response)


def get_id_token(username: str, password: str, cognito_app_client_id: str) -> str:
    cognito = boto3.client('cognito-idp')
    auth_response = cognito.initiate_auth(
        AuthFlow='USER_PASSWORD_AUTH',
        AuthParameters={
            'USERNAME': username,
            'PASSWORD': password,
        },
        ClientId=cognito_app_client_id,
    )
    return auth_response['AuthenticationResult']['IdToken']


def get_request_url(rest_endpoint: str, api_gateway_api_id: str, aws_region: str) -> str:
    return f'https://{api_gateway_api_id}.execute-api.{aws_region}.amazonaws.com/dev/{rest_endpoint}'


def log_response(response):
    print("Status Code:")
    print(response.status_code)

    print('\nResponse (text):')
    print(response.text)

    print('\nResponse (json):')
    try:
        print(format_response_text(response.text))
    except json.decoder.JSONDecodeError:
        print('ERROR: Response is not in JSON format.')


def format_response_text(response_text: str) -> str:
    # Load from string:
    response_dict = json.loads(response_text)

    # Convert back to string with proper indentation:
    return json.dumps(response_dict, indent=4)


if __name__ == '__main__':
    main()
