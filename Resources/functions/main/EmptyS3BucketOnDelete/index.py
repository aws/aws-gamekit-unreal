# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

import hashlib
import json
import logging
import urllib.request
from enum import Enum

import boto3
from botocore.exceptions import ClientError


class ResponseStatus(str, Enum):
    SUCCESS = 'SUCCESS'
    FAILED = 'FAILED'


class RequestType(str, Enum):
    CREATE = 'Create'
    DELETE = 'Delete'
    UPDATE = 'Update'


s3_resource = boto3.resource('s3')
s3_client = boto3.client('s3')
logger = logging.getLogger()
logger.setLevel(logging.INFO)


def _send_success_response(event):
    _send_response(event, ResponseStatus.SUCCESS, '')


def _send_failure_response(event, reason):
    _send_response(event, ResponseStatus.FAILED, reason)


def _get_physical_resource_id(stack_id, logical_resource_id) -> str:
    """
    Returns a PhysicalResourceId that uniquely identifies the custom CloudFormation resource.

    PhysicalResourceId must be stable - we'll base it on the StackId and LogicalResourceId of the resource.
    As PhysicalResourceId has a limit of 64 characters, we'll take the MD5 hash of these to avoid surpassing the
    resource limit while retaining a unique id per resource.
    """
    # Hex encoded MD5 hash is 32 characters
    physical_resource_id_hash = hashlib.md5(f'{stack_id}-{logical_resource_id}'.encode()).hexdigest()
    return f'empty-on-delete-{physical_resource_id_hash}'


def _send_response(event, response_status: ResponseStatus, reason: str):
    physical_resource_id = _get_physical_resource_id(event['StackId'], event['LogicalResourceId'])
    response_body = {
        'Status': response_status,
        'Reason': reason,
        'PhysicalResourceId': physical_resource_id,
        'StackId': event['StackId'],
        'RequestId': event['RequestId'],
        'LogicalResourceId': event['LogicalResourceId'],
        'Data': json.loads('{}')
    }

    request = urllib.request.Request(event['ResponseURL'],
                                     method='PUT',
                                     data=json.dumps(response_body).encode('utf-8'),
                                     headers={'Content-Type': '""'})
    # If there is an issue when responding to CloudFormation (malformed content, etc.),
    # we can pull this request from the logs and manually send a success response.
    logger.info(f'Sending {response_status} notification request to CloudFormation at {event["ResponseURL"]}')

    with urllib.request.urlopen(request) as response:
        logger.info(f'Received {response.getcode()} response from CloudFormation: {response.reason}')


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
        _send_success_response(event)
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
            _send_success_response(event)
            return
        # 403 or another unknown error was returned
        reason = f'Cannot access {bucket_name}: {e}'
        logger.error(reason)
        _send_failure_response(event, reason)
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
        _send_success_response(event)
    except ClientError as e:
        reason = f'{e.response["Error"]["Code"]} - {e.response["Error"]["Message"]}'
        logger.error(f'Failed to empty bucket {bucket_name}: {reason}')
        _send_failure_response(event, reason)
