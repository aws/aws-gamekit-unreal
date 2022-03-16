# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

from unittest import TestCase
from layers.main.CommonLambdaLayer.python.gamekithelpers.validation import (
    is_valid_primary_identifier,
    is_valid_base_64,
    MAX_PRIMARY_IDENTIFIER_CHARS,
    MIN_PRIMARY_IDENTIFIER_CHARS
)


class TestValidation(TestCase):
    def test_primary_identifier_short(self):
        # arrange
        identifier = 'a' * (MIN_PRIMARY_IDENTIFIER_CHARS - 1)
        # act
        result = is_valid_primary_identifier(identifier)

        # assert
        self.assertFalse(result)

    def test_primary_identifier_long(self):
        # arrange
        identifier = 'a' * (MAX_PRIMARY_IDENTIFIER_CHARS + 1)

        # act
        result = is_valid_primary_identifier(identifier)

        # assert
        self.assertFalse(result)

    def test_primary_identifier_limit(self):
        # arrange
        identifier = 'a' * MAX_PRIMARY_IDENTIFIER_CHARS

        # act
        result = is_valid_primary_identifier(identifier)

        # assert
        self.assertTrue(result)

    def test_primary_identifier_invalid_chars(self):
        # arrange
        identifier = '$0me>.!dentifier_#\\/+=~`?'

        # act
        result = is_valid_primary_identifier(identifier)

        # assert
        self.assertFalse(result)

    def test_primary_identifier_spaces(self):
        # arrange
        identifier = 'Valid-id.1_ Valid-id.2_'

        # act
        result = is_valid_primary_identifier(identifier)

        # assert
        self.assertFalse(result)

    def test_primary_identifier_success(self):
        # arrange
        identifier = 'This.is_a-valid.ident1fier__'

        # act
        result = is_valid_primary_identifier(identifier)

        # assert
        self.assertTrue(result)

    def test_base_64(self):
        sub_tests = [
            ('Base64 - no padding', 'abcd+/+/', True),
            ('Base64 - 1 padding', 'abcde+/=', True),
            ('Base64 - 2 padding', 'abcdef==', True),
            ('Empty string', '', True),
            ('Malformed - HTML characters', '<script>document.getElementById("userId")</script>', False),
            ('Malformed - Non-ASCII characters', '限界を超える', False),
            ('Malformed - Not a multiple of 4', 'ab+/c==', False)
        ]

        for test_name, test_string, expected_result in sub_tests:
            with self.subTest(test_name):
                # arrange
                # act
                result = is_valid_base_64(test_string)

                # assert
                self.assertEqual(expected_result, result)
