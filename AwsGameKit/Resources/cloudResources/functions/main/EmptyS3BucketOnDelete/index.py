# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

from distutils.util import strtobool
import logging

import boto3
from botocore.exceptions import ClientError
from gamekitresourcemanagement.cfn_custom_resource import RequestType, send_success_response, send_failure_response

CUSTOM_RESOURCE_TYPE = 'empty-on-delete'

s3_resource = boto3.resource('s3')
s3_client = boto3.client('s3')
logger = logging.getLogger()
logger.setLevel(logging.INFO)


def lambda_handler(event, context):
    """
    Empties an S3 bucket during CloudFormation stack deletion.

    S3 buckets which aren't empty cannot be deleted by CloudFormation. In order to allow deletion of a stack which
    contains a non-empty S3 bucket, pair this function with a custom CloudFormation resource. During stack delete
    events, this function will empty out the provided S3 bucket. Nothing will happen during other CloudFormation
    operations.

    If the provided S3 bucket doesn't exist, the lambda function will exit gracefully.

    Parameters:
        event:
            The custom resource lambda request. CloudFormation provides most parameters. See the full request here:
            https://docs.aws.amazon.com/AWSCloudFormation/latest/UserGuide/crpg-ref-requests.html#crpg-ref-request-fields.

            ResourceProperties:
                Contains user defined properties. Passed in by the Properties object of the custom CloudFormation
                resource which calls this lambda function.

                bucket_name: str
                    The name of the S3 bucket to empty.
        context:
            The lambda context. See the list of methods and properties here:
            https://docs.aws.amazon.com/lambda/latest/dg/python-context.html
    """
    logger.info(event)
    request_type: str = event['RequestType']
    bucket_name: str = event['ResourceProperties']['bucket_name']

    if request_type != RequestType.DELETE:
        logger.info(f'Ignoring {request_type} request for bucket {bucket_name}')
        send_success_response(event, CUSTOM_RESOURCE_TYPE)
        return

    logger.info(f'Received request to delete objects in bucket {bucket_name}')

    # The high level client relies on the ListBuckets API to determine if a single bucket exists.
    # For users with lots of S3 buckets, it can be expensive to loop through all of their buckets searching for
    # the bucket we wish to empty.
    # Determine if the bucket exists using the low level HeadBucket instead, which only inspects the desired bucket.
    try:
        s3_client.head_bucket(Bucket=bucket_name)
    except ClientError as e:
        if e.response['Error']['Code'] == '404':
            logger.info(f'Ignoring request to delete bucket {bucket_name}, as it does not exist')
            send_success_response(event, CUSTOM_RESOURCE_TYPE)
            return
        # 403 or another unknown error was returned
        reason = f'Cannot access {bucket_name}: {e}'
        logger.error(reason)
        send_failure_response(event, CUSTOM_RESOURCE_TYPE, reason)
        return

    try:
        bucket = s3_resource.Bucket(bucket_name)
        logger.info(f'Found bucket {bucket_name}; proceeding to empty contents')

        # Different APIs are used to delete versioned and non-versioned objects. Check the bucket versioning in order
        # to use the correct API.
        bucket_versioning = s3_resource.BucketVersioning(bucket_name)
        if bucket_versioning.status == 'Enabled':
            bucket.object_versions.all().delete()
        else:
            bucket.objects.all().delete()
        logger.info(f'Emptied bucket {bucket_name}')
    except ClientError as e:
        reason = f'{e.response["Error"]["Code"]} - {e.response["Error"]["Message"]}'
        logger.error(f'Failed to empty bucket {bucket_name}: {reason}')
        send_failure_response(event, CUSTOM_RESOURCE_TYPE, reason)
        return

    # If delete_bucket flag is set to True, also try deleting it.
    if bool(strtobool(event['ResourceProperties'].get('delete_bucket', 'false'))):
        try:
            bucket = s3_resource.Bucket(bucket_name)
            logger.info(f'Proceeding to delete bucket {bucket_name}')

            bucket.delete()
            logger.info(f'Deleted bucket {bucket_name}')
        except ClientError as e:
            reason = f'{e.response["Error"]["Code"]} - {e.response["Error"]["Message"]}'
            logger.error(f'Failed to delete bucket {bucket_name}: {reason}')
            send_failure_response(event, CUSTOM_RESOURCE_TYPE, reason)
            return

    send_success_response(event, CUSTOM_RESOURCE_TYPE)
