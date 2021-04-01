# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

"""
Purpose

Resizes a PNG according to the defined dimensions os.environ['RESIZE_WIDTH'], os.environ['RESIZE_HEIGHT'].

This Lambda Function is configured in AchievementsBucket to execute on s3:ObjectCreated:Put for the uploads/ prefix.
The resized images are moved to the os.environ['DESTINATION_PREFIX'] prefix.
"""

import boto3
import botocore
from io import BytesIO
from gamekithelpers import handler_response
import os
from PIL import Image

s3_resource = boto3.resource('s3')


def _get_bucket_and_key_from_event(event_record):
    s3_record = event_record["s3"]
    bucket = s3_record["bucket"]["name"]
    key = s3_record["object"]["key"]
    return bucket, key


def lambda_handler(event, context):
    """
    This is the lambda function handler.
    """

    # Get the aws account id from the env vars
    aws_account_id = os.environ.get('AWS_ACCOUNT_ID')

    # Read source image data from S3
    try:
        for event_record in event['Records']:
            bucket, key = _get_bucket_and_key_from_event(event_record)
            s3_source_obj = s3_resource.Object(bucket, key)
            source_data = s3_source_obj.get(ExpectedBucketOwner=aws_account_id)['Body'].read()

            # Resize image
            img = Image.open(BytesIO(source_data))
            resized_img = img.resize((int(os.environ['RESIZE_WIDTH']), int(os.environ['RESIZE_HEIGHT'])))

            # Save resized data to a new key
            resized_data = BytesIO()
            resized_img.save(resized_data, img.format)
            resized_key = f"{os.environ['DESTINATION_PREFIX']}/{key.split('/')[-1]}"
            s3_resized_obj = s3_resource.Object(bucket, resized_key)
            resized_data.seek(0)
            resp = s3_resized_obj.put(
                Body=resized_data,
                ContentType=img.get_format_mimetype(),
                ExpectedBucketOwner=aws_account_id
            )

            # Delete source object
            s3_source_obj.delete(ExpectedBucketOwner=aws_account_id)

        handler_response.return_response(200, f"Resized {len(event['Records'])} images")
    except botocore.exceptions.ClientError as error:
        if error.response['Error']['Code'] == 'AccessDenied':
            print('Access Denied: User is forbidden to access S3 bucket')
            handler_response.forbidden_request()
        else:
            # Maintain previous behavior until more error codes are implemented
            raise error
