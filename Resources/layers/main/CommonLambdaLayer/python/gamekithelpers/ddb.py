# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

"""
Helper functions for DynamoDB
"""

import boto3
from boto3.dynamodb.types import TypeDeserializer, TypeSerializer
from boto3.dynamodb.conditions import Key
from datetime import timezone, datetime
from typing import Dict, Any


def get_table(table_name):
    """
    Returns a resource representing a DynamoDB Table
    :param table_name: Name of table
    :return: DynamoDB Table
    """
    return boto3.resource('dynamodb').Table(table_name)


def deserialize_response_item(response_item) -> Dict[str, Any]:
    """
    Deserializes a DynamoDB object to JSON
    :param response_item: DynamoDB object
    :return: JSON representation of DynamoDB object
    """
    deserializer = TypeDeserializer()
    return {k: deserializer.deserialize(value=v) for k, v in response_item.items()}


def query_request_param(key_id, key_value,
                        index_name=None,
                        response_limit=100,
                        use_consistent_read=False,
                        start_key=None) -> Dict[str, Any]:
    """
    Creates a Query request used as an argument to Table.query()
    :param key_id: Key to use in query
    :param key_value: Key value
    :param index_name: The name of the index to query
    :param response_limit: Number of items to return
    :param use_consistent_read: Indicates whether to use consistent read. Default is eventually consistent read.
    :param start_key: If the results are paginated, this is the next start key to use.
    :return: Query request
    """
    param = {
        'Select': 'ALL_ATTRIBUTES' if index_name is None else 'ALL_PROJECTED_ATTRIBUTES',
        'Limit': response_limit,
        'ConsistentRead': use_consistent_read,
        'KeyConditionExpression': Key(key_id).eq(key_value)
    }

    if index_name:
        param['IndexName'] = index_name

    if start_key is not None:
        param['ExclusiveStartKey'] = start_key

    return param


def scan_request_param(response_limit,
                       use_consistent_read,
                       start_key=None,
                       filter: boto3.dynamodb.conditions.Key = None) -> Dict[str, Any]:
    """
    Creates a Scan request used as an argument to Table.scan()
    :param response_limit: Number of items to return
    :param use_consistent_read: Indicates whether to use consistent read. Default is eventually consistent read.
    :param start_key: If the results are paginated, this is the next start key to use.
    :param filter: The filter to use in the scan.
    :return: Scan request
    """
    param = {
        'Select': 'ALL_ATTRIBUTES',
        'Limit': response_limit,
        'ConsistentRead': use_consistent_read
    }

    if start_key is not None:
        param['ExclusiveStartKey'] = start_key

    if filter is not None:
        param['FilterExpression'] = filter

    return param


def get_item_request_param(keys: dict, use_consistent_read: bool = False) -> Dict[str, Any]:
    """
    Creates a GetItem request used as an argument to Table.get_item()
    :param use_consistent_read: Indicates whether to use consistent read. Default is eventually consistent read.
    :param keys: A dictionary of keys to use in the GetItem request.
    :return: GetItem request
    """
    return {
        'Key': keys,
        'ConsistentRead': use_consistent_read,
    }


def put_item_request_param(item):
    """
    Creates a PutItem request used as an argument to Table.put_item()
    :param item: The item to put in DynamoDB
    :return: PutItem request
    """
    return {
        'Item': item,
        'ReturnConsumedCapacity': 'TOTAL'
    }


def get_response_items(response) -> ([Dict[str, Any]], Dict[str, Any]):
    """
    Iterates through the response of Scan or Query operation and returns an array of deserialized DynamoDB objects and the next
    start key if the result is paginated.
    :param response: The response of Scan or Query operation
    :return: An array of deserialized DynamoDB objects and the next start key if the result is paginated.
    """
    items = []
    for item in response.get('Items'):
        items.append(item)

    last_evaluated_key = response.get('LastEvaluatedKey')
    next_start_key = None
    if last_evaluated_key is not None:
        next_start_key = last_evaluated_key
    return items, next_start_key


def get_response_item(response) -> ([Dict[str, Any]], Dict[str, Any]):
    """
    Gets the response of a GetItem operation
    :param response: The response of GetItem operation
    :return: A deserialized DynamoDB object
    """
    return response.get('Item')


def get_update_response_item(response) -> Dict[str, Any]:
    """
    Deserializes the response of Update operation and deserialized DynamoDB objects
    :param response: The response of Update operation
    :return: A deserialized DynamoDB object
    """
    return response.get('Attributes')


def timestamp() -> str:
    """
    Returns a UTC timestamp in ISO format
    """
    return datetime.utcnow().replace(tzinfo=timezone.utc).isoformat()
