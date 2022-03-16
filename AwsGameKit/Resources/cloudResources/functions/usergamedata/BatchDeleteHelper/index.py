# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

"""
Helper Lambda function for asynchronous calling batch_write from the DeleteAll Lambda.
Please refer to the UserGameplay Cloudformation file to increase the number of concurrent BatchDeleteHelper Lambdas
https://docs.aws.amazon.com/lambda/latest/dg/configuration-concurrency.html
"""

import boto3
import botocore
import logging

logger = logging.getLogger()
logger.setLevel(logging.INFO)

ddb_resource = boto3.resource('dynamodb')


def lambda_handler(event, context):
    params = {
        'RequestItems': {
            event['TableName']: event['DeleteRequest']
        }
    }
    try:
        ddb_resource.batch_write_item(**params)
    except botocore.exceptions.ClientError as err:
        logger.error(f"Error calling batch_write_item. Error: {err}")
        raise err
