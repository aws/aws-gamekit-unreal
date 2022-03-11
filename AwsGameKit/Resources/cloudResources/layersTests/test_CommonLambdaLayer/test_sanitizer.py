# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

from unittest import TestCase
from layers.main.CommonLambdaLayer.python.gamekithelpers.sanitizer import sanitize


class TestSanitization(TestCase):
    def test_remove_tags(self):
        text = "easiest way is just regex and remove <SCRIPT> and </script> tags. " \
               "and for HTML we can just remove  the <a href='https://blah'> links</a>, " \
               "<img>, <img /> and <  link  > that can download other resource files " \
               "but keep <br/>, <!-- --> and <PRE>"
        expected = "easiest way is just regex and remove  and  tags. " \
                   "and for HTML we can just remove  the  links, " \
                   ",  and  that can download other resource files " \
                   "but keep <br/>, <!-- --> and <PRE>"

        result = sanitize(text)

        self.assertEqual(result, expected)

    def test_int_input_type_returns_as_is(self):
        text = 10
        expected = 10

        result = sanitize(text)

        self.assertEqual(result, expected)
        self.assertIsInstance(result, type(text))

