# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

from unittest import TestCase
from unittest.mock import patch, call, MagicMock

with patch("boto3.resource") as boto_resource_mock:
    from functions.usergamedata.BatchDeleteHelper import index


def _build_batch_delete_helper_event():
    return {
        'TableName': 'test_bundleitems_table',
        'DeleteRequest': [
            {'DeleteRequest': {'Key': {'player_id_bundle': '12345678-1234-1234-1234-123456789012_BANANA_BUNDLE',
                                       'bundle_item_key': 'SCORE1'}}},
            {'DeleteRequest': {'Key': {'player_id_bundle': '12345678-1234-1234-1234-123456789012_BANANA_BUNDLE',
                                       'bundle_item_key': 'SCORE2'}}}
        ]
    }


class TestGetItem(TestCase):
    def setUp(self):
        index.ddb_resource = MagicMock()

    def test_batch_delete_helper_event_calls_batch_write_item(self):
        test_event = _build_batch_delete_helper_event()

        index.lambda_handler(test_event, None)

        index.ddb_resource.batch_write_item.assert_called_once_with(
            RequestItems={'test_bundleitems_table': [
                {'DeleteRequest':
                     {'Key': {'player_id_bundle': '12345678-1234-1234-1234-123456789012_BANANA_BUNDLE',
                              'bundle_item_key': 'SCORE1'}}},
                {'DeleteRequest':
                     {'Key': {'player_id_bundle': '12345678-1234-1234-1234-123456789012_BANANA_BUNDLE',
                              'bundle_item_key': 'SCORE2'}}}]})
