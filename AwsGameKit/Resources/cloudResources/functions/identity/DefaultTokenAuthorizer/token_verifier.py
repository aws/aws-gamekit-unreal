# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

"""
Purpose

This is a custom token verifier.
Adapted from https://github.com/awslabs/aws-support-tools/blob/master/Cognito/decode-verify-jwt/decode-verify-jwt.py
"""

import boto3
import botocore
import os
import logging
import json
import time
from jose import jwk, jwt
from jose.utils import base64url_decode

logger = logging.getLogger()
logger.setLevel(logging.INFO)
secrets_manager_client = boto3.client('secretsmanager')


def verify(token, stage, verify_expiration=True):
    secret_name = os.environ.get('JWKS_SECRET_NAME')

    try:
        response = secrets_manager_client.get_secret_value(
            SecretId=secret_name,
            VersionStage=stage
        )
    except botocore.exceptions.ClientError as err:
        logger.error(f"Error getting secret {secret_name}. Error: {err}")
        return None, False

    jwks = json.loads(response['SecretString'])
    keys = jwks['keys']

    # get the kid from the headers prior to verification
    headers = jwt.get_unverified_headers(token)
    kid = headers['kid']

    # search for the kid in the stored public keys
    key_index = -1
    for i in range(len(keys)):
        if kid == keys[i]['kid']:
            key_index = i
            break

    if key_index == -1:
        logger.error(f"Public key not found in {secret_name} for {kid}")
        return None, False

    # construct the public key
    public_key = jwk.construct(keys[key_index])

    # get the last two sections of the token,
    # message and signature (encoded in base64)
    message, encoded_signature = str(token).rsplit('.', 1)

    # decode the signature
    decoded_signature = base64url_decode(encoded_signature.encode('utf-8'))

    # verify the signature
    if not public_key.verify(message.encode("utf8"), decoded_signature):
        logger.error('Signature verification failed')
        return None, False

    # since we passed the verification, we can now safely
    # use the unverified claims
    claims = jwt.get_unverified_claims(token)

    # additionally, we can verify the token expiration
    if time.time() > claims['exp'] and verify_expiration:
        logger.error('Token is expired')
        return None, False

    return claims, True
