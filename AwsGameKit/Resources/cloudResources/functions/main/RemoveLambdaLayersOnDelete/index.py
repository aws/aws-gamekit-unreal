# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

import logging
from typing import Any, Dict, List

import boto3
from botocore.exceptions import ClientError
from gamekitresourcemanagement.cfn_custom_resource import RequestType, send_success_response, send_failure_response

lambda_client = boto3.client('lambda')
logger = logging.getLogger()
logger.setLevel(logging.INFO)

CUSTOM_RESOURCE_TYPE = 'delete-lambda-layers'
LIST_MAX_ITEMS = 50  # Max allowed by Lambda


def _extract_next_token(response: Any) -> str:
    return response.get('NextMarker', None)


def _get_lambda_layer_names(layer_prefix: str, event: Any) -> List[str]:
    """Find all the lambda layers in the account that start with the provided prefix."""

    def extract_matching_layer_names(layers: List[Any]):
        return [layer['LayerName'] for layer in layers if layer['LayerName'].startswith(layer_prefix)]

    layer_names = []
    try:
        response = lambda_client.list_layers(MaxItems=LIST_MAX_ITEMS)
        layer_names.extend(extract_matching_layer_names(response['Layers']))
        next_token = _extract_next_token(response)

        while next_token is not None:
            response = lambda_client.list_layers(
                Marker=next_token,
                MaxItems=LIST_MAX_ITEMS
            )
            layer_names.extend(extract_matching_layer_names(response['Layers']))
            next_token = _extract_next_token(response)

    except ClientError as e:
        reason = f'{e.response["Error"]["Code"]} - {e.response["Error"]["Message"]}'
        logger.error(f'Failed to list all layers prefixed with {layer_prefix}: {reason}')
        send_failure_response(event, CUSTOM_RESOURCE_TYPE, reason)
        raise e

    logger.info(f'Found matching layers: {layer_names}')
    return layer_names


def _get_lambda_layer_versions_by_name(layer_names: List[str], event: Any) -> Dict[str, List[int]]:
    """Returns a map containing all available layer versions for each of the provided lambda layers."""

    def extract_layer_versions(layer_versions_response: List[Any]):
        return [layer_version['Version'] for layer_version in layer_versions_response]

    layer_versions_by_name = {}
    try:
        for layer_name in layer_names:
            response = lambda_client.list_layer_versions(LayerName=layer_name, MaxItems=LIST_MAX_ITEMS)
            next_token = _extract_next_token(response)
            layer_versions_by_name[layer_name] = extract_layer_versions(response['LayerVersions'])

            while next_token is not None:
                response = lambda_client.list_layer_versions(LayerName=layer_name, Marker=next_token,
                                                             MaxItems=LIST_MAX_ITEMS)
                next_token = _extract_next_token(response)
                layer_versions_by_name[layer_name].extend(extract_layer_versions(response['LayerVersions']))
    except ClientError as e:
        reason = f'{e.response["Error"]["Code"]} - {e.response["Error"]["Message"]}'
        logger.error(f'Failed to list all layer versions for target layers: {reason}')
        send_failure_response(event, CUSTOM_RESOURCE_TYPE, reason)
        raise e

    logger.info(f'Found matching layer versions: {layer_versions_by_name}')
    return layer_versions_by_name


def _delete_layer_versions(layers: Dict[str, List[int]], event: Any):
    """Delete the provided layer versions for each lambda layer."""
    # There is no batch deletion for lambda layer versions; delete them one by one
    for layer_name in layers:
        for layer_version in layers[layer_name]:
            try:
                logger.info(f'Deleting layer version {layer_name}:{layer_version}')
                lambda_client.delete_layer_version(LayerName=layer_name, VersionNumber=layer_version)
            except ClientError as e:
                reason = f'{e.response["Error"]["Code"]} - {e.response["Error"]["Message"]}'
                logger.error(f'Failed to delete layer version {layer_name}:{layer_version}: {reason}')
                send_failure_response(event, CUSTOM_RESOURCE_TYPE, reason)
                raise e


def lambda_handler(event, context):
    """
    Removes all lambda layer versions with the desired prefix on CloudFormation stack deletion.

    AWS GameKit manages lambda layers outside of CloudFormation lifecycle. In order to ensure that all GameKit lambda
    layers for a game are removed when the corresponding CloudFormation stack is deleted, pair this function with a
    custom CloudFormation resource. During stack delete events, this function will delete any left over lambda layers.
    Nothing will happen during other CloudFormation operations.

    If no lambda layers with the desired prefix exist, this lambda function will exit gracefully.

    Parameters:
        event:
            The custom resource lambda request. CloudFormation provides most parameters. See the full request here:
            https://docs.aws.amazon.com/AWSCloudFormation/latest/UserGuide/crpg-ref-requests.html#crpg-ref-request-fields.

            ResourceProperties:
                Contains user defined properties. Passed in by the Properties object of the custom CloudFormation
                resource which calls this lambda function.

                layer_prefix: str
                    All lambda layers with this prefix will be deleted.
        context:
            The lambda context. See the list of methods and properties here:
            https://docs.aws.amazon.com/lambda/latest/dg/python-context.html
    """
    logger.info(event)
    request_type: str = event['RequestType']
    layer_prefix: str = event['ResourceProperties']['layer_prefix']

    if request_type != RequestType.DELETE:
        logger.info(f'Ignoring {request_type} request for layers prefixed with {layer_prefix}')
        send_success_response(event, CUSTOM_RESOURCE_TYPE)
        return

    logger.info(f'Received request to delete all layers prefixed with {layer_prefix}')

    layer_names = _get_lambda_layer_names(layer_prefix, event)
    layer_versions_by_name = _get_lambda_layer_versions_by_name(layer_names, event)
    _delete_layer_versions(layer_versions_by_name, event)

    logger.info(f'Successfully deleted lambda layers with prefix {layer_prefix}')
    send_success_response(event, CUSTOM_RESOURCE_TYPE)
