# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

"""
Type annotations for custom data types.
"""

from typing import Union, Dict, List


# As defined by: https://docs.python.org/3/library/json.html#py-to-json-table
JsonObject = Union[
    Dict[str, 'JsonObject'],
    List['JsonObject'],
    str,
    int,
    float,
    bool,
    None
]
