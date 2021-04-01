# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

import re


_invalid_pattern = re.compile('<[\s]*(img|a|script|link|/img|/a|/script|/link)[^>]*>', re.IGNORECASE)
_all_tags_regex = re.compile('<[^>]*>')


def sanitize(text, new_value=''):
    """
    Removes HTML tags that can be executed, used to download from or link to external resources.
    """

    # if input type is anything but string, return as-is
    # not enforcing type in parameters since we don't want this to fail with a non string type
    if not isinstance(text, str):
        return text

    temp = text

    # First use a regular expression that matches ALL tags with or without attributes, white space,
    # dashes, slash, etc...
    for match in re.finditer(_all_tags_regex, text):
        matched_str = match.group(0)

        # Now match and replace the invalid tags pattern
        if re.search(_invalid_pattern, matched_str):
            temp = temp.replace(matched_str, new_value)

    return temp
