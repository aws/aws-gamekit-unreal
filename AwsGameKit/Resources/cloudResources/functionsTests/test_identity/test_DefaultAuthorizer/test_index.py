# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0
import json
import os
from unittest import TestCase
from unittest.mock import patch, MagicMock, Mock

import botocore.exceptions
from functionsTests.helpers.boto3.mock_responses.exceptions import new_boto_exception

with patch("boto3.client") as boto_client_mock:
    from functions.identity.DefaultTokenAuthorizer import index, token_verifier


class TestIndex(TestCase):
    def setUp(self):
        token_verifier.secrets_manager_client = MagicMock()

    @patch('functions.identity.DefaultTokenAuthorizer.token_verifier.boto3')
    def test_secret_not_found_unauthorized(self, mock_boto3):
        # Arrange
        event = self.get_lambda_event()
        token_verifier.secrets_manager_client.get_secret_value = Mock(
            side_effect=new_boto_exception(botocore.exceptions.ClientError)
        )

        # Act/Assert
        # API Gateway will automatically transform an Exception('Unauthorized') to a 401
        with self.assertRaises(Exception):
            index.lambda_handler(event, None)

    @patch.dict(os.environ, {'USER_IDENTIFIER_CLAIM_FIELD': 'anotherfield',
                             'ENDPOINTS_ALLOWED': 'achievements/*'})
    @patch('functions.identity.DefaultTokenAuthorizer.token_verifier.boto3')
    def test_secret_missing_claim(self, mock_boto3):
        # Arrange
        event = self.get_lambda_event()
        token_verifier.secrets_manager_client.get_secret_value.return_value = {
            'ARN': 'arn:aws:secretsmanager:us-west-2:123456789012:secret:gamekit_dev_testgame_ThirdPartyJwks-abcdef',
            'Name': 'gamekit_dev_testgame_ThirdPartyJwks',
            'VersionId': '1',
            'SecretString': json.dumps(self.jwk_set())
        }

        # Act/Assert
        # API Gateway will automatically transform an Exception('Unauthorized') to a 401
        with self.assertRaises(Exception):
            index.lambda_handler(event, None)

    @patch.dict(os.environ, {'USER_IDENTIFIER_CLAIM_FIELD': 'sub',
                             'ENDPOINTS_ALLOWED': 'achievements/*'})
    @patch('functions.identity.DefaultTokenAuthorizer.token_verifier.boto3')
    def test_secret_expired_token(self, mock_boto3):
        # Arrange
        event = self.get_lambda_event()
        token_verifier.secrets_manager_client.get_secret_value.return_value = {
            'ARN': 'arn:aws:secretsmanager:us-west-2:123456789012:secret:gamekit_dev_testgame_ThirdPartyJwks-abcdef',
            'Name': 'gamekit_dev_testgame_ThirdPartyJwks',
            'VersionId': '1',
            'SecretString': json.dumps(self.jwk_set())
        }

        # Act/Assert
        # API Gateway will automatically transform an Exception('Unauthorized') to a 401
        with self.assertRaises(Exception):
            index.lambda_handler(event, None)

    @patch.dict(os.environ, {'USER_IDENTIFIER_CLAIM_FIELD': 'sub',
                             'ENDPOINTS_ALLOWED': 'achievements/*',
                             'VERIFY_EXPIRATION': 'false'})
    @patch('functions.identity.DefaultTokenAuthorizer.token_verifier.boto3')
    def test_secret_found_authorized(self, mock_boto3):
        # Arrange
        event = self.get_lambda_event()
        token_verifier.secrets_manager_client.get_secret_value.return_value = {
            'ARN': 'arn:aws:secretsmanager:us-west-2:123456789012:secret:gamekit_dev_testgame_ThirdPartyJwks-abcdef',
            'Name': 'gamekit_dev_testgame_ThirdPartyJwks',
            'VersionId': '1',
            'SecretString': json.dumps(self.jwk_set())
        }

        # Act
        result = index.lambda_handler(event, None)

        # Assert
        self.assertEqual('107932416965203234076', result['principalId'])
        self.assertEqual('execute-api:Invoke', result['policyDocument']['Statement'][0]['Action'])
        self.assertEqual('Allow', result['policyDocument']['Statement'][0]['Effect'])
        self.assertEqual(1, len(result['policyDocument']['Statement'][0]['Resource']))
        self.assertEqual('arn:aws:execute-api:us-west-2:123456789012:a1b2c3d4e5/dev/*/achievements/*',
                         result['policyDocument']['Statement'][0]['Resource'][0])
        self.assertEqual('107932416965203234076', result['context']['custom:thirdparty_player_id'])

    @patch.dict(os.environ, {'USER_IDENTIFIER_CLAIM_FIELD': 'sub',
                             'ENDPOINTS_ALLOWED': 'achievements,achievements/*',
                             'VERIFY_EXPIRATION': 'false'})
    @patch('functions.identity.DefaultTokenAuthorizer.token_verifier.boto3')
    def test_secret_found_multiple_endpoints_authorized(self, mock_boto3):
        # Arrange
        event = self.get_lambda_event()
        token_verifier.secrets_manager_client.get_secret_value.return_value = {
            'ARN': 'arn:aws:secretsmanager:us-west-2:123456789012:secret:gamekit_dev_testgame_ThirdPartyJwks-abcdef',
            'Name': 'gamekit_dev_testgame_ThirdPartyJwks',
            'VersionId': '1',
            'SecretString': json.dumps(self.jwk_set())
        }

        # Act
        result = index.lambda_handler(event, None)

        # Assert
        self.assertEqual('107932416965203234076', result['principalId'])
        self.assertEqual('execute-api:Invoke', result['policyDocument']['Statement'][0]['Action'])
        self.assertEqual('Allow', result['policyDocument']['Statement'][0]['Effect'])
        self.assertEqual(2, len(result['policyDocument']['Statement'][0]['Resource']))
        self.assertEqual('arn:aws:execute-api:us-west-2:123456789012:a1b2c3d4e5/dev/*/achievements',
                         result['policyDocument']['Statement'][0]['Resource'][0])
        self.assertEqual('arn:aws:execute-api:us-west-2:123456789012:a1b2c3d4e5/dev/*/achievements/*',
                         result['policyDocument']['Statement'][0]['Resource'][1])
        self.assertEqual('107932416965203234076', result['context']['custom:thirdparty_player_id'])


    @staticmethod
    def get_lambda_event():
        # Sample token copied from python-jose unit tests
        return {
            'type': 'TOKEN',
            'methodArn': 'arn:aws:execute-api:us-west-2:123456789012:a1b2c3d4e5/dev/POST/achievements/TEST1/unlock',
            'authorizationToken': 'eyJhbGciOiJSUzI1NiIsImtpZCI6IjhmYmJlZWE0MDMzMmQyYzBkMjdlMzdlMTkwNGFmMjliNjQ1OTRlNTcifQ.eyJpc3MiOiJodHRwczovL2FjY291bnRzLmdvb2dsZS5jb20iLCJhdF9oYXNoIjoiUUY5RnRjcHlmbUFBanJuMHVyeUQ5dyIsImF1ZCI6IjQwNzQwODcxODE5Mi5hcHBzLmdvb2dsZXVzZXJjb250ZW50LmNvbSIsInN1YiI6IjEwNzkzMjQxNjk2NTIwMzIzNDA3NiIsImF6cCI6IjQwNzQwODcxODE5Mi5hcHBzLmdvb2dsZXVzZXJjb250ZW50LmNvbSIsImlhdCI6MTQ2ODYyMjQ4MCwiZXhwIjoxNDY4NjI2MDgwfQ.Nz6VREh7smvfVRWNHlbKZ6W_DX57akRUGrDTcns06ndAwrslwUlBeFsWYRLon_tDw0QCeQCGvw7l1AT440UQBRP-mtqK_2Yny2JmIQ7Ll6UAIHRhXOD1uj9w5vX0jyI1MbjDtODeDWWn_9EDJRBd4xmwKhAONuWodTgSi7qGe1UVmzseFNNkKdoo54dXhCJiyiRAMnWB_FQDveRJghche131pd9O_E4Wj6hf_zCcMTaDaLDOmElcQe-WsKWAA3YwHFEWOLO_7x6u4uGmhItPGH7zsOTzYxPYhZMSZusgVg9fbE1kSlHVSyQrcp_rRWNz7vOIbvIlBR9Jrq5MIqbkkg'
        }

    @staticmethod
    def jwk_set():
        # Sample keyset copied from python-jose unit tests
        return {
            "keys": [
                {
                    "alg": "RS256",
                    "e": "AQAB",
                    "kid": "40aa42edac0614d7ca3f57f97ee866cdfba3b61a",
                    "kty": "RSA",
                    "n": "6lm9AEGLPFpVqnfeVFuTIZsj7vz_kxla6uW1WWtosM_MtIjXkyyiSolxiSOs3bzG66iVm71023QyOzKYFbio0hI-yZauG3g9nH-zb_AHScsjAKagHtrHmTdtq0JcNkQnAaaUwxVbjwMlYAcOh87W5jWj_MAcPvc-qjy8-WJ81UgoOUZNiKByuF4-9igxKZeskGRXuTPX64kWGBmKl-tM7VnCGMKoK3m92NPrktfBoNN_EGGthNfQsKFUdQFJFtpMuiXp9Gib7dcMGabxcG2GUl-PU086kPUyUdUYiMN2auKSOxSUZgDjT7DcI8Sn8kdQ0-tImaHi54JNa1PNNdKRpw",
                    "use": "sig",
                },
                {
                    "alg": "RS256",
                    "e": "AQAB",
                    "kid": "8fbbeea40332d2c0d27e37e1904af29b64594e57",
                    "kty": "RSA",
                    "n": "z7h6_rt35-j6NV2iQvYIuR3xvsxmEImgMl8dc8CFl4SzEWrry3QILajKxQZA9YYYfXIcZUG_6R6AghVMJetNIl2AhCoEr3RQjjNsm9PE6h5p2kQ-zIveFeb__4oIkVihYtxtoYBSdVj69nXLUAJP2bxPfU8RDp5X7hT62pKR05H8QLxH8siIQ5qR2LGFw_dJcitAVRRQofuaj_9u0CLZBfinqyRkBc7a0zi7pBxtEiIbn9sRr8Kkb_Boap6BHbnLS-YFBVarcgFBbifRf7NlK5dqE9z4OUb-dx8wCMRIPVAx_hV4Qx2anTgp1sDA6V4vd4NaCOZX-mSctNZqQmKtNw",
                    "use": "sig",
                },
                {
                    "alg": "RS256",
                    "e": "AQAB",
                    "kid": "6758b0b8eb341e90454860432d6a1648bf4de03b",
                    "kty": "RSA",
                    "n": "5K0rYaA7xtqSe1nFn_nCA10uUXY81NcohMeFsYLbBlx_NdpsmbpgtXJ6ektYR7rUdtMMLu2IONlNhkWlx-lge91okyacUrWHP88PycilUE-RnyVjbPEm3seR0VefgALfN4y_e77ljq2F7W2_kbUkTvDzriDIWvQT0WwVF5FIOBydfDDs92S-queaKgLBwt50SXJCZryLew5ODrwVsFGI4Et6MLqjS-cgWpCNwzcRqjBRsse6DXnex_zSRII4ODzKIfX4qdFBKZHO_BkTsK9DNkUayrr9cz8rFRK6TEH6XTVabgsyd6LP6PTxhpiII_pTYRSWk7CGMnm2nO0dKxzaFQ",
                    "use": "sig",
                },
            ]
        }
