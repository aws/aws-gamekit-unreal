# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

"""
Helper functions for KMS cryptography
"""

from cryptography.fernet import Fernet
from base64 import b64decode, b64encode
import botocore
import boto3
import os

NUM_BYTES_FOR_LEN = 4


def get_gamekit_key():
    return os.environ.get('GAMEKIT_KMS_KEY_ID'), os.environ.get('GAMEKIT_KMS_KEY_ARN')


def create_data_key(cmk_id):
    """Generate a data key to use when encrypting and decrypting data
    """

    # Create data key
    kms = boto3.client('kms')
    try:
        response = kms.generate_data_key(KeyId=cmk_id, KeySpec='AES_256')
    except botocore.exceptions.ClientError as e:
        print(e)
        return None, None

    # Return the encrypted and plaintext data key
    return response['CiphertextBlob'], b64encode(response['Plaintext'])


def encrypt_text(cmk_id, text):
    """Encrypt text using an AWS KMS CMK
    """

    # Create data key
    data_key_encrypted, data_key_plaintext = create_data_key(cmk_id)
    if data_key_encrypted is None:
        return False, None

    # Encrypt text
    f = Fernet(data_key_plaintext)
    encrypted_text = f.encrypt(text.encode('utf-8'))

    # Write the encrypted text key and encrypted file contents together
    encrypted_blob = len(data_key_encrypted).to_bytes(NUM_BYTES_FOR_LEN, byteorder='big')
    encrypted_blob += data_key_encrypted
    encrypted_blob += encrypted_text

    return True, encrypted_blob


def decrypt_data_key(data_key_encrypted):
    """
    Decrypt an encrypted data key
    """

    # Decrypt the data key
    kms_client = boto3.client('kms')
    try:
        response = kms_client.decrypt(CiphertextBlob=data_key_encrypted)
    except botocore.exceptionsClientError as e:
        print(e)
        return None

    # Return plaintext base64-encoded binary data key
    return b64encode((response['Plaintext']))


def decrypt_blob(cmk_id, encrypted_blob):
    """
    Decrypt text using an AWS KMS CMK
    """
    encrypted_data = b64decode(encrypted_blob)

    data_key_encrypted_len = int.from_bytes(encrypted_data[:NUM_BYTES_FOR_LEN], byteorder='big') + NUM_BYTES_FOR_LEN
    data_key_encrypted = encrypted_data[NUM_BYTES_FOR_LEN:data_key_encrypted_len]
    data_key_plaintext = decrypt_data_key(data_key_encrypted)
    if data_key_plaintext is None:
        return False, None

    f = Fernet(data_key_plaintext)
    decrypted_data = f.decrypt(encrypted_data[data_key_encrypted_len:])
    return True, decrypted_data
