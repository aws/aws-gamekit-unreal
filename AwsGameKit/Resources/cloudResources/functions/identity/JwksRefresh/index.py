# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

"""
Purpose

This retrieves the JSON Web Key Set (JWKS) from the configured source
"""


import boto3
import botocore
import logging
import os
from urllib.request import urlopen

logger = logging.getLogger()
logger.setLevel(logging.INFO)
secrets_manager_client = boto3.client('secretsmanager')


def lambda_handler(event, context):
    """
    This is the lambda function handler.
    """

    # Download JWKS file
    with urlopen(os.environ.get('JWKS_THIRDPARTY_URI')) as f:
        jwks = f.read().decode('utf-8')

    # Store JWKS contents in Secrets Manager
    secret_name = os.environ.get('JWKS_SECRET_NAME')
    try:
        secrets = secrets_manager_client.list_secrets(
            Filters=[ { 'Key': 'name', 'Values': [secret_name] } ]
        )

        if len(secrets['SecretList']) > 0:
            logger.info(f"Updating secret {secret_name}.")
            secrets_manager_client.put_secret_value(
                SecretId=secret_name,
                SecretString=jwks
            )
        else:
            logger.info(f"Creating secret {secret_name}.")
            secrets_manager_client.create_secret(
                Name=secret_name,
                SecretString=jwks
            )

    except botocore.exceptions.ClientError as err:
        logger.error(f"Error storing secret {secret_name}. Error: {err}")
        raise err
