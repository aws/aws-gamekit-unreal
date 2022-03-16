# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

import boto3

"""
Use this script to obtain a valid IdToken which can be used to invoke an AWS GameKit API Gateway Endpoint.

Most game developer facing API Gateway Endpoints expect the request header to contain a key "authorization" with a
logged-in player's IdToken. For example:
http_request = {
    'headers': {
        'authentication': '<ID_TOKEN>'
    }
}

This script logs in a player and returns their valid IdToken. The token will expire after some time.

PREREQUISITES:
* You've deployed the Identity/Authentication feature.
* You've created and "confirmed" a user.
* Your [default] AWS credentials are configured in `~/.aws/credentials`. See https://docs.aws.amazon.com/cli/latest/userguide/cli-configure-files.html#cli-configure-files-where
"""


def main():
    username = 'foo'
    password = 'bar'

    # Find your client id here:
    #   1. Open: https://us-west-2.console.aws.amazon.com/cognito/home?region=us-west-2
    #   2. Click: "Manager User Pools" > "<YOUR_POOL>" > App clients
    cognito_app_client_id = '123abc'

    id_token = get_id_token(username, password, cognito_app_client_id)
    print(id_token)


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


if __name__ == '__main__':
    main()
