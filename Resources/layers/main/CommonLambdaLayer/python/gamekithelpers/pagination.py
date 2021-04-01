# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

import hmac
import json

from base64 import b64encode, b64decode
from hashlib import sha256
from time import time

def validate_pagination_token(player_id: str = None,
                              next_start_key: dict = None,
                              pagination_token: str = None) -> bool:
    """
    Calculate the HMAC pagination token and verify it matches
    what the player sent up in their request.
    """
    if pagination_token is None:
        return False

    digest, expires_at = b64decode(pagination_token).decode().split(":")
    now = time()
    if now > int(expires_at):
        logger.warning("{} tried to use pagination token after expiration".format(player_id))
        return False
    return digest == generate_hmac(player_id, next_start_key, expires_at)


def generate_hmac(key: str = None, data: dict = None, expires_at: int = 0) -> str:
    byte_key = key.encode()
    message = "".join([json.dumps(data), "1.0.0", str(expires_at)])
    byte_msg = message.encode()
    return hmac.new(byte_key, byte_msg, sha256).hexdigest().upper()


def generate_pagination_token(key: str = "", start_key: dict = None) -> str:
    # Expires 5 minutes from now
    expires_at = int(time() + 300)
    digest = generate_hmac(key, start_key, expires_at)
    token = ":".join([digest, str(expires_at)]).encode()
    # b64encode gets us bytes,
    #  .decode() gets us a string
    return b64encode(token).decode()
