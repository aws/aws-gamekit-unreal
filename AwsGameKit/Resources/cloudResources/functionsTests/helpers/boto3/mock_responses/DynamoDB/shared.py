# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

from typing import Dict, Any


def get_response_metadata() -> Dict[str, Any]:
    """
    Get a sample "ResponseMetadata" object.

    All DynamoDB APIs include the key 'ResponseMetadata' in their API response. The metadata looks like this:
    """
    return {
        'RequestId': '5LAM4TN08CAOH89NC7HNQE4567VV4KQNSO5AEMVJF66Q9ASUAAJG',
        'HTTPStatusCode': 200,
        'HTTPHeaders': {
            'server': 'Server',
            'date': 'Wed, 28 Jul 2021 21:05:35 GMT',
            'content-type': 'application/x-amz-json-1.0',
            'content-length': '167',
            'connection': 'keep-alive',
            'x-amzn-requestid': '5LAM4TN08CAOH89NC7HNQE4567VV4KQNSO5AEMVJF66Q9ASUAAJG',
            'x-amz-crc32': '1788032195'
        },
        'RetryAttempts': 0
    }
