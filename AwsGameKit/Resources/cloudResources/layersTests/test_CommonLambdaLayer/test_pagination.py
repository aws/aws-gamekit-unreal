import time
from base64 import b64decode, b64encode
from unittest import TestCase, mock

from layers.main.CommonLambdaLayer.python.gamekithelpers import pagination


class TestPagination(TestCase):

    def setUp(self):
        self.player_id = "foo"
        self.start_key = {"bar": "baz"}
        self.token = pagination.generate_pagination_token(self.player_id, self.start_key)

    def test_validate_pagination_token(self):
        self.assertTrue(pagination.validate_pagination_token(self.player_id, self.start_key, self.token))

    def test_rejects_tampered_expired_at(self):
        digest, expires_at = b64decode(self.token).decode().split(":")
        expires_at = str(int(expires_at) + 300) #attempt to extend it another 5 minutes
        tampered_token = b64encode(":".join([digest, expires_at]).encode()).decode()
        self.assertFalse(pagination.validate_pagination_token(self.player_id, self.start_key, tampered_token))

    def test_rejects_with_no_paging_token(self):
        self.assertFalse(pagination.validate_pagination_token(self.player_id, self.start_key, None))

    def test_rejects_tampered_digest(self):
        digest, expires_at = b64decode(self.token).decode().split(":")
        tampered_digest = "".join([digest, "DEADBEEF"])
        tampered_token = b64encode(":".join([tampered_digest, expires_at]).encode()).decode()
        self.assertFalse(pagination.validate_pagination_token(self.player_id, self.start_key, tampered_token))