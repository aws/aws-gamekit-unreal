# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

"""
Helper functions for validating objects.
"""

import re

# Constraints
MAX_PRIMARY_IDENTIFIER_CHARS = 512
MIN_PRIMARY_IDENTIFIER_CHARS = 1

primary_identifier_regex = re.compile('^([a-zA-Z0-9-_.]+)$')
base_64_regex = re.compile('^([A-Za-z0-9+/]{4})*([A-Za-z0-9+/]{3}=|[A-Za-z0-9+/]{2}==)?$')


def is_valid_primary_identifier(identifier: str) -> bool:
    """
    Returns true if the input string is a valid primary identifier, false otherwise.

    A primary identifier is a string between 1 and 512 characters long, using alphanumeric characters, dashes (-),
    underscores (_), and periods (.). Primary identifiers are safe to use in path parameters without being encoded,
    DynamoDB partition and sort keys, and S3 object keys.
    """
    if len(identifier) < MIN_PRIMARY_IDENTIFIER_CHARS or len(identifier) > MAX_PRIMARY_IDENTIFIER_CHARS:
        return False
    return primary_identifier_regex.fullmatch(identifier) is not None


def is_valid_base_64(string: str) -> bool:
    """
    Returns true if the input is a valid Base64 encoded string.

    :param string: Input string to validate
    :return: Whether the input string is a valid Base64 encoded string.
    """
    return base_64_regex.fullmatch(string) is not None
