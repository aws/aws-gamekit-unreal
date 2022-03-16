# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

import os
from unittest import TestCase
from unittest.mock import patch, MagicMock

with patch("boto3.client") as boto_client_mock:
    from functions.identity.RetrieveFacebookTokens import index

# Patch Lambda environment variables:
@patch.dict(os.environ, {
    'GAMEKIT_KMS_KEY_ID': '12345678-abcd-1234-abcd-123456789012',
    'GAMEKIT_KMS_KEY_ARN': 'arn:aws:kms:us-west-2:123456789012:key/12345678-abcd-1234-abcd-123456789012',
})
class TestIndex(TestCase):
    def setUp(self):
        index.s3_client = MagicMock()

    @patch('functions.identity.RetrieveFacebookTokens.index.crypto.boto3')
    def test_can_write_token_to_s3_successfully(self, mock_gamekithelpers_crypto_boto3):
        # Arrange
        event = self.get_lambda_event('12.123.123.123')
        context = None
        mock_gamekithelpers_crypto_boto3.client('kms').decrypt.side_effect = [
            # The first call of decrypt() returns:
            self.get_kms_decrypted_event_body(),

            # The second call of decrypt() returns:
            self.get_kms_decrypted_s3_tokens(),
        ]
        index.s3_client.get_object()['Body'].read().decode.return_value = 'AAAAuAECAwB4gjEk2Zmmx/bj19DfBb4LR+Ewtl4LkhZ89uZAIwWOdqIBPX4PKahsEPlIyCc+UtJ86QAAAH4wfAYJKoZIhvcNAQcGoG8wbQIBADBoBgkqhkiG9w0BBwEwHgYJYIZIAWUDBAEuMBEEDKVecKcT0u585eI9HgIBEIA77izI8kPilLg5j7Yg6fnVjf5Mkk4/xdiOwYQZ1H97117iA7O9fV2lhLRNGncS/pcZ/YoXjd4HYNPDWnVnQUFBQUFCZzdPcEkxRWVocUFXbGswUzN6a21vWjRDQkd6VE9RMTNHcFJhNmozTGtPektzYUN6UjdOQ0hxNzZIcG5hanU0STN3alVvcjZlTXB6U1c2UTVZdFdibldXSktIWlZoSjdtdGw1b2dTck5Qb1RaNy0yOHVBYVFCcm1TV1BFTU1UWXFuMlROenhSU3hZNzBOQXF2V1J6QTFJcFVHMXFfRnYwbzllNkYxTEdHZWVHMUtibFhoQXliel9RX0ZDaDBnTXhyX3hrREYwTF9CWWhvdU00ajN5VFRRUzdIU2tWU0Q5TTRWYWJBWHpnX3l1a0tGSzUzd3pGWkNibVRBR3h4cUxPQzJMZGhDMUZJM2ZHVDM4eDJKSmp1bGFSR084amJ4Qjd2SkI5VF82REQ0aUFXakhYZFUzclZyWnlWWjZRQndIVG42VHRuWi1jSTZMam9IdTlPbFRNeVF0SlB0a1B6Y2NfZTk3X1psQ1Vmd2dsY09FS0tUZjhjZ1h4NkVIdlh5QTRpQlJDSHZ1b3BzSjhhcDVveTJZY2V6emRxX2RWS1lOSEY3SFNITnJqMDRTTUl1cHJ2R0VNMmlmSUN4Q0hhOEctSUZsZzdIanNyUG42QkxVQUU0SlJNejFjS3ZaVHZWeEN6bFo5WE1SaGVfOFFoS1Z6aW1zYkZNdHlUbHIxTHRTRGNRbjlBRWdPNTBVOUlJR0EtNE9vdjdKM0dEWWVXUDVMbXpOUlQwMHJUdDRHWW1DSXJwRGY5VzhqVVkzdFB4d3E1aEg4cnRUMDNWaDBSOTU5cHoydmtlcVpSYTFiVFNTcHYwaFhGc2lnckVfQWlzQWJRa3RQNzR5dnJRQ1ZqanF5Zlo1dUI0S25EakxhN0NZQXNkcWxEb2x3Qk8tX3BNWTk3UkFWanJxQTdQdXhEVUFoaW41SVJBZEN2VTRGdXZOV1R0cVdOUWdYN011S2x5akFiN1pIeS1vV1lHTnV0Z2ljTXZQOEhScERKVEU1ZzY5ZGpxcTBvZS1oSlVZdXJpWHN1eHR1N2I0LTkyQjlIM1lqb2ZOLXFiNWRrdERuQ3JEZ3d1dTZId1pISjVQdzlvc29tb1Z6OV9zWjlQamFsQ2FSU1RPeE1TaXhtZ0JKNzlzM1BoQXJQN0lZcFpnbjZUamMtTWlpNGdqTE1sRGhhaWhveVNyVTlJQW9weU1LcHhzMkNnRmRkbU9QenQ0eFZxUEFzQmtTQ1BVaS14RWNRMGNLY1ZLN2pMR2tUUEpodUJHdlc1cjJiVGhPbWZ5c01tSlFZMXBTY3NuejlISDNHcjdMdEg2WTZ6OE1nODZEZDg5UHRwWkl3cWU3LWl5TWFKRWtNT3ZjU0tzNFJVdmdyZ1dDSEw3bXRrcmJ6MnlPYlVlX3RsU3U0YXREVjhWWWNETnNxalMtYTFOTmdUUkctNWxlVHB0eHZfTE0tUVJ2d2JLLUJlR3dzSVkzckxsbnRFQ3hYZHlMVWkxcGRraDQwUXl5dUZVV3VsRXEtNTZoQ0J5RlRfeDk1WlBfWmZPQ0pkb3VmNkFOWmdkY3M5Zk5RTmZoQ0dKRnp3NnhrY0pxbzBfOGJ2bWlOWnZPUjN0WXFhRkdUWHltQW1YN0VxTS1KbmpUU0lBR0FTdXZvbTQwbHFwN0NZYVhETlZRdnJqeUQwUUgtYTY4alBkbnJXdGVfOHVBTzdTMERpQ0F5b0UyZWMtV3lSUm1zSzRBVU1WVGo1SGFmOXl0OEs4LTRJd21tci1GNEp1NTRiUnRRbzZYNGI5cS1tajdVUjU3TWN3S3RRT0tCS3p6a1dHQmVlYUlvdW9iZjNHaFdKVVQ5a25saDZyUTQxRk13X2plN0JmbnJVRVFwOEd2ejdHUlcxbjNsTjFPcVhJN1lVOW9OOGMxM2c0OVF4ZGdORjJDVU1nSkFaOTdXQ3BidWMyZFpoTENtc2pVc1lrLTFtY0V1OG05ZkxBUjNqUF9nVkIwUUlWc1pFamJiVWQtSmF0QUJGYllHREx4YjNaaDVwNHZycGJsTXpBUllMZFYteEppc3RmaGEtYWlSb2tRaWp5dWZ0NmI1bDN5NjdiajExNTU5a3dPYlB1SjNsM1hmMmRVWVlHUlpyWk84QllwS0s3UTNVaGEyVWpkY3BrdDVKZGxyUXFaUXJKODl1bnI0WTlLYVE2emk5dG5UOHNrWW5CejVjX1Y5eEJ3VjZPSlF3UXhzNVJPLWdGclp5eXcySUlBdmJMUXk2bThyckRiTlFiWDhiR3Azb3RpLTJvMVZaVDhkdElQQmlpX2pwNERtQ2NTQWV3Z1IxZEpmbzNhNnN1dUhBbkpnTGR2SGlUM2l0Mlc5UUlCR25nLWR5TmNyeXR5MWNRcHFCR0VBenJGek9iZ1NvLWt2UlNlMVhQU0pENkw2NExaVnAtc09YXzVkZ0dsZERFV3BqZ3Q5YU5aeVVIMFZ5dGZjT21QX2M3QWpZSzl2UVpzSi1SVFBQNS1QdEMzZVNiQXFsWTJEZDJlQXBVdzRXR0l4Q3JvUjZHTThnNHlmT0tsRzFyUzdORnowWnUxclRDOERrdnMwYUpjMFdTMVFyem4zZ2ZGT3FwcW9yRndyMWpCU1Z2UjZMb0hNZUFwdlFZZmlSaWlieHY4UjZDX0huZUlVNXJXMWVoU19pVUJBOTVqTk1UbW1FcDA3SlllWVJ2R1BQNm1EeXZ0aE50ZTVlSVNoeTg2UXAwT2ZfNXFwSDR1MzlOZEtLTEFlaVdKS1dKVFBWcHotcWZLU1RFVzVKcWtQZmlCOW5ESkxVamtxUFZaYWhOb05GVWpPTGdZVUJaTWVBWGhZeEhxR0tPYy00WEdtMGxCaEh3UWRtOC16blBWcE1uWUNmZFBrZEJMSDRFMUtfakpRYnBRZzUzU2tBMnRiUHpKT2tselY2czIxT25Jd2hSTE9iT2s2bUU1b3Z2UHJTNVV3U0tzYTVsOWNpSDBTd0FzQWJBbnpEa0hwWGJtR2hPNGdLUUF2c1hTMW1pTWJyc2tCQUtMd2NwY1FNcGx6UUpkZ1E4QlRGWHRDVVVnb3puVmxtR1hTRk9WaEZ3bk9zUkNCc2ZCTklxVlFoZVROOThfb2FndFBNLTZiU1JPNXNSdjhBSTNfbUpDUkJ5Z0dnRUNfYXFHU2dLV2s2NTl3bUJtSGtfdEJzWTdGSnRua1R1WFRrSmJ1ZU1wMnZsX01EalNfaDhSNGR4Z1p4TWwydmx0T0VCYWFScXlMUW5oXzRjaXJHcUFqaGdsZm5PSDNlc2VqY3RyRE9pcHdsZjRZaVVyWFhvRTNnRk5KWURGel9mQWlXT1FXQ29yNWlSQ3RWdzBES3JSZXh4bVZGZGg0d0NjemNSZmdab3FUTWdpUFJJSGVWSFVQMURybXkyQWp6WjJjNVc3S3NMNEV4Nk8zRFZtY09kclFZcWt0d2NfNDlnMUxQdl81Q1ZCTUFGYlg0OE0wUkZ2dkljakI4dFNvWE83OG81Nnp1UDB0SG14bmc0T0puY0I5b2lTMkltS3UtZUVPdFp3djZDVTM5V1BFaTBpbzRud0hrQlRYTThOSUtWNERnX2JzX2M3MllKN3dtMWRkWGN2MkVXUUpBRkxsTVdSbmdNdlZ4b1dFMm9UaWpPVmF0OHdKdEFkZ0Q2eTNfcXdXSUktS3FwamFCc19LdUNsbUthb0otaWo3QzU0MjZUUWhSS1o5bWVXS3dyVldFSUZ1UE04cS1yNF9sR1FkSFBCYkdpalFlQzNtRnJMTm1mX2loSjFwN191ZFROSlVBRnFVbUU0SkE3blZyTTR5VXdDaUVKcml1ZlFmcDNJUkd0OExzTTgtNHp2eDRsTzg1UVJHc3VkRjdtMlRuU0wtbGQzRVd3Vl9qZElRaE1MaHktUUM3YlVpdURMZUsyYzdCdnJKUTM4RHBmaE5NOEQtbTFja3Y2eW13bEJGTTNOb25oZmg0NWlXd2h5ZGlhWm54TGNVbmJuVTU5ZXRGT2hVajlqNTVMdUxHQkI0dG4wTGJ2eUp4WV9MenBSTnpXMDdOSVFWekpkV2pRekZ2X0tBUVdRMjNhVHdWVk50OF9MbUhrRjBaTk1RTDhHZnRyNUJDdXdQRlhzUFJWVjVWM3pGMk9PcF9DdVBBanhFRzdXSF94N25ETnIzemZrakRzZ291SzhZRUJXLVF2WS03QTBpZU5DZmFKcmZJUkItRFRpSjlaVGkwbGlCX3JfTEJJZzFHZEZrZzBkcEFDcEF0YlNVby1kMUotR3U2dTB5ZnIyM2NlTGg4VExqa0d4c3hEcFBQOGtPNlVRY1lRY1BzZUpOQUhoVmFDRm5uWFo5akR6MFBBWFR1Ym1MLXVSSmRxUmJoeURHbklfZW1IeS1OUUJkWjJLMEZVX25YN2pfYkNGNHlmeDg4M2ltblJ6X3N4WG5nb1hQQzdHMEI2bkVFVzU2cUZHN0pNYnVXV01saDFMa05rdzZIT3JFaVlvOExqZGNhQ1h5R2k5Z2l5eFUxZHVPU21OSi1XNDdub0N0M0dXU09KaktGWWNnM3NCWFpnX2lnT1pEV0p6eXppR3ZLMzhXbkZGMkdxaWg1NWo3M2daVWdHenAtbnhfNmpMTGlGQkpDQTJkOW9TaURwZFkwMUF1blVLUDhQSE9wcVFsczJYVjkxTGtHclgxdHhiRkdNaTZJTGVwUjFvbDVqcTZEaHRlMF9KZ2FmN3Z3SFA1QlBMSF84b2Rfdm1fSkJCSm5xbHJiTnZSOFFmOXc5Q1BFdGhxUzBBQjlYd3I3Nk81XzNHNmJZT25iSVU2S3JIb0tJbUxQWXAzVHNzdERxOWM2V3c1Vmo1OW55Q1lKdVNlbTkzZWhDMnNlQkNTYkRablIwZGNVdFVfLTVYdFhFNmFMVy1FSktGU0dBbHdUSTBsREVfd1ZOY0NHd2xBMndNVXNyblB1NllHTDZvRDBBNzdLRWNmdFZGN2hmTk1fSzdDWDd3QndTaW5FNnVKbGdHUEVfQ2hxUWpXV3dVdzFiTXVZdlc1V0l4WWN6dE9IUnN0eEhabWE1S1UwckVublM0Q0xPdWpPSEFoNDVmaThYcENqWTV6R2Jjd1IwbExXbGU2UXdTSHNWRzFpUE1sVjR0QTJrWGU5SlN6LTkwTkZ1d3VVbFUxb2NtNFBwTUt4R0l4ZzNLa24zYk0tU2hKc3hUcnJWaDUtRjlMS1RsZEp0U3NOOWN2M1Fxa2MtM1hVdF9OU2dOSkpTa3dSWHpCaHNBQkZtNDBDa3V2blhNZHBsTnlyNTVmcEZjU0Fxd0cyQjh2clJEWTdHbXFCZWNZejdka0xLdjdNdllFRWp4Vl80cHVDQl83REx5VXNoZGhvYW90cVo2VUxXNDVYLWtwQ1BtbGdoQjZXNXZhaV9zM2RUdFNhMkV0Zmp0MHJtZEx1OXBYTGpSVVFOa3VSX05UNjRFSHFYVXN2c3hLN2EteWFiVlJjaUtYWWNSOEFGNlZrM3pHNE5HaTBlcFQ5WTR5UFBsNmdoaTQ1TDVWUkliWlpONGFnY3hpak5yekJmbGFCSW5TSnNsYnQ3RUVKUTZOOEdZTkw5ZU13dnBpc0g4eENlcWIxdzFSbUxSQm1zQVh2X1B3QkdOejhLUGp0QWwtSnYxNXlETzBqdHM1Q21JNkN3Y0g2Y280RFhIc2hEZ0RnMWR6R2hoTnVIS0V2MmUyZ0F4dmhBeVRuNFlsOWhpUWJfRlBvOXlIazlEaHRLZkZJZEpLQWh2bTRKdzZyU3haS3Fvb0xLdGZ2bGJNRV9qRU1WZHg1UVlEeXJjcUdwN1gxLTNERkRfZkgtS0lFM21sTEJUSUhsenIzTWpFUnJhUjZMQ2ZaUUpDdUxKeE91LTBmWnd3SEd4Y1AyTXlpcTZWcTZwcTJXR1ZGVlVoYmxubHFnb0VraGhBU1FPMlBwbjY4YkwzRWYwLWZnUDkwNktmQkdVUURMLTNuOXNjTG5jRlZyM1pIbXJpR21hcDA1eTlGTDcyRExqZWtJWnJSeWVjMFh5NW1vNXcwSFNiazBNZDB1aDRFR2xSM0ZTaUJnX1NJRVJOY1oxUHNPbldUZ1lZN0tsbUlhVzZEMVlmWWJGSEtHV0JZUzU1RFd5YW1BVFFjVlVGdElHSHhwdkVEemJlUzJoQzA1Q25ZaUhGUHpoLVNLYUhGNU9CU3FYUXU4aUZpOF9pZVpVSjVBcEJZYjZNTnlrRGlLLTZ2eVpHVWxtYW5lN2ZyancxejNpVUlrRW9MY0dmY2tmYlBWNVcwX2s5dWp3dU9sV09FLU9obUJKRm4tVU9LZTZQWlhyUnVaS2xkSVBjSkNfSmh6RjNhM3p1akdYQnFKWm40SGpGRGl0NGVCbnBubHViV1BFYWZpcHNEcjFjSjljZlNBbU9sdXYtTHc0b1d6V05wZHhOdzQtZFh5RFhXYXNpc25sS09sdlJQUTlaSU9WREZMRWFCUVdCMWFYM2RZUW9hdzdWZFVRRll0LXE2X0kxbEhaYzh3SE10SUZyM0hLZlJpR2xPZDQ1QVpBeUlfcTlWSVBfdjlYbXlqVFgyaFJRNWhzSlZPa3R3alMzVmtVM3loZGEyVGdfVFNKbi1ybFNtTm4yTlQtbmZ2RkVtRW1qbmh4UHhLOTFNa3dDcTYzRUJEam4tYmxSOXRCb28yMUx6c0JJQUN4SW9QY2xxN3YwUmM3UzNQb3Fxek1DODdYSWZ3bVdFMDdha1R1d3M2eTNQbU1jMnc1ekJzeHJzWDZyN2FXaGZ4Z3QwZFk4S3lWbmFfM2RRNWhOVldqUXNlOURkVmVfbHFpRVNZSnNMSXlDVTczc3ZBV3hGTnFiXzZWOVhaWFc5ZXlQb3hDanA1Um15RVhSem82SEllOEctUi1tTTBGajdUdjNXY3Nyb3QwbHM5c25oX21zQ2xiWl9DUXFEaWstYnBlYkJwc0FSZTlDVTFIcXdDcDhNaF9Gc2tBV3d6dUs0dVBJbTNoR0tBRUxnNE5jYmJqTHJKY05wcjVZV05DTWxsejAyc0FqNXF6RC1sX0dWSzM5Y09qVzd0TjAtTi1QbzBEUkFFVUlWU0dWZ0FwcTJvVm5Yekl0NjBGdTlhU3E4MzIxRktRQ2g4RmcyMUJRcjZCSzN2LVdhV0RyOEg3dVhWWHl3M0t6RDl6dFEya3dBM1BVVFNhY3pMbjBpVU15ZWtTSjNJLTFHTVAwUU81Yk9iZk5tM2NnZlN4ZVdYNVBYcml2aHJuOWhtR1J4T0I4RXlJeDB3dUhwaF9xa0JGNjVtVnlkbWZZMlR1X21leGpuRXlFSVB1d2RxejRpTnB3WUlUd0t5U3VrbFZoaHJDSm96TDBTR2U3R2l5UjdmMWJoTUxMNDZHb1hPZnFxS3Q3UlhPLV9XOEJUbEF5YlJESEZVRmVXUzNZNUhINDBXZnlGQmgwMTN1VzYxSzRJZDRZZ2JhN0t0aTJ4V18xMmFWZ0tadFU0dFhtYlVqRTRmbUhIeG9vc3ItRWhJYU14ZmlPVTN6TG4tXzg3bTAyQ0VHYW1idk51'

        # Act
        result = index.lambda_handler(event, context)

        # Assert
        self.assertEqual(200, result['statusCode'])
        index.s3_client.put_object.assert_called_once()

    def test_lambda_returns_a_400_error_code_when_the_event_body_is_none(self):
        # Arrange
        event = self.get_lambda_event('12.123.123.123')
        context = None

        event['body'] = None

        # Act
        result = index.lambda_handler(event, context)

        # Assert
        self.assertEqual(400, result['statusCode'])
        index.s3_client.put_object.assert_not_called()

    @patch('functions.identity.RetrieveFacebookTokens.index.crypto.boto3')
    def test_lambda_returns_a_403_error_code_when_the_encrypted_tokens_were_already_retrieved(self, mock_gamekithelpers_crypto_boto3):
        # Arrange
        event = self.get_lambda_event('12.123.123.123')
        context = None

        mock_gamekithelpers_crypto_boto3.client('kms').decrypt.return_value = self.get_kms_decrypted_event_body()
        index.s3_client.get_object()['Body'].read().decode.return_value = 'Retrieved'

        # Act
        result = index.lambda_handler(event, context)

        # Assert
        self.assertEqual(403, result['statusCode'])
        index.s3_client.put_object.assert_not_called()

    def test_lambda_returns_a_400_error_code_when_encrypted_source_ip_does_not_match_request(self):
        # Arrange
        event = self.get_lambda_event('12.123.123.124')
        context = None

        event['body'] = None

        # Act
        result = index.lambda_handler(event, context)

        # Assert
        self.assertEqual(400, result['statusCode'])
        index.s3_client.put_object.assert_not_called()

    @patch('functions.identity.RetrieveFacebookTokens.index.crypto.decrypt_blob')
    def test_lambda_raises_an_attribute_error_when_decrypt_blob_returns_none(self, mock_decrypt_blob):
        # Arrange
        event = self.get_lambda_event('12.123.123.123')
        context = None
        mock_decrypt_blob.return_value = (False, None)

        # Act/Assert
        with self.assertRaises(AttributeError):
            index.lambda_handler(event, context)

        index.s3_client.put_object.assert_not_called()

    @staticmethod
    def get_lambda_event(source_ip):
        return {
            'resource': '/identity/fbtokens',
            'path': '/identity/fbtokens',
            'httpMethod': 'POST',
            'headers': {
            'Accept': '*/*',
                'cache-control': 'no-cache',
                'CloudFront-Forwarded-Proto': 'https',
                'CloudFront-Is-Desktop-Viewer': 'true',
                'CloudFront-Is-Mobile-Viewer': 'false',
                'CloudFront-Is-SmartTV-Viewer': 'false',
                'CloudFront-Is-Tablet-Viewer': 'false',
                'CloudFront-Viewer-Country': 'US',
                'content-type': 'application/json',
                'Host': '123abcdefg.execute-api.us-west-2.amazonaws.com',
                'pragma': 'no-cache',
                'User-Agent': 'Amazon CloudFront',
                'Via': '2.0 7eb3b782ab09047ce0d11ee03763894c.cloudfront.net (CloudFront)',
                'X-Amz-Cf-Id': 'cZnJhUoOPznLqZJJs_dG29zkPeJnXOjYFlgm62hKNWwRT6wDWMxd1Q==',
                'X-Amzn-Trace-Id': 'Root=1-60eca6ae-522354b13571eef26d31fced',
                'X-Forwarded-For': '12.123.123.123, 56.567.567.567',
                'X-Forwarded-Port': '443',
                'X-Forwarded-Proto': 'https'
            },
            'multiValueHeaders': {
                'Accept': ['*/*'],
                'cache-control': ['no-cache'],
                'CloudFront-Forwarded-Proto': ['https'],
                'CloudFront-Is-Desktop-Viewer': ['true'],
                'CloudFront-Is-Mobile-Viewer': ['false'],
                'CloudFront-Is-SmartTV-Viewer': ['false'],
                'CloudFront-Is-Tablet-Viewer': ['false'],
                'CloudFront-Viewer-Country': ['US'],
                'content-type': ['application/json'],
                'Host': ['123abcdefg.execute-api.us-west-2.amazonaws.com'],
                'pragma': ['no-cache'],
                'User-Agent': ['Amazon CloudFront'],
                'Via': ['2.0 7eb3b782ab09047ce0d11ee03763894c.cloudfront.net (CloudFront)'],
                'X-Amz-Cf-Id': ['cZnJhUoOPznLqZJJs_dG29zkPeJnXOjYFlgm62hKNWwRT6wDWMxd1Q=='],
                'X-Amzn-Trace-Id': ['Root=1-60eca6ae-522354b13571eef26d31fced'],
                'X-Forwarded-For': ['12.123.123.123, 56.567.567.567'],
                'X-Forwarded-Port': ['443'],
                'X-Forwarded-Proto': ['https']
            },
            'queryStringParameters': None,
                'multiValueQueryStringParameters': None,
                'pathParameters': None,
                'stageVariables': None,
                'requestContext': {
                'resourceId': 'jywlns',
                'resourcePath': '/identity/fbtokens',
                'httpMethod': 'POST',
                'extendedRequestId': 'CX77TEKaPHcFfSw=',
                'requestTime': '12/Jul/2021:20:31:42 +0000',
                'path': '/dev/identity/fbtokens',
                'accountId': '123456789012',
                'protocol': 'HTTP/1.1',
                'stage': 'dev',
                'domainPrefix': '123abcdefg',
                'requestTimeEpoch': 1626121902570,
                'requestId': 'adb5fd01-38a6-4463-8b40-98fe238ae819',
                'identity': {
                    'cognitoIdentityPoolId': None,
                    'accountId': None,
                    'cognitoIdentityId': None,
                    'caller': None,
                    'sourceIp': source_ip,
                    'principalOrgId': None,
                    'accessKey': None,
                    'cognitoAuthenticationType': None,
                    'cognitoAuthenticationProvider': None,
                    'userArn': None,
                    'userAgent': 'Amazon CloudFront',
                    'user': None
                },
                'domainName': '123abcdefg.execute-api.us-west-2.amazonaws.com',
                'apiId': '123abcdefg'
            },
            'body': 'AAAAuAECAwB4gjEk2Zmmx/bj19DfBb4LR+Ewtl4LkhZ89uZAIwWOdqIB1QzexgpLD3CMHP4X2VTSfAAAAH4wfAYJKoZIhvcNAQcGoG8wbQIBADBoBgkqhkiG9w0BBwEwHgYJYIZIAWUDBAEuMBEEDPVYMSAo/vu9vGL44QIBEIA7/QXAQNz9USlSGezQ7Q9OhTQfagINyTbB4CdmAuwXi5k+Coce6EGtX03CSWIQixJmunecj1iPr4tMcQVnQUFBQUFCZzdPVk1lellCb0VOTFBOQ18waWdaQ2t0TUNYNWpPLVhhU0RMWVlHQ0wzMGg0TThmOXpKdUxHek40RHcyMTM2UHlMcUFkMHoyNEs5YUVWWTdnZHM4LXlhbFdVT2lVMkU3R3VtT2x4b3gwaFROS0lSYmt5WDg5REt2cnFoOW5NN3FiYkRfT1RfcjIyWjE0OVdIdExjYjhVVWZzSUVvLXR5TERmVXEyWjlqQTRwUzRESzhVbUZFSnJ4cDZuVC0yNkRhNTAtaXM=',
            'isBase64Encoded': False
        }

    @staticmethod
    def get_kms_decrypted_event_body():
        """
        Get the response of calling kms.decrypt() on the event['body'].
        """
        return {
            'KeyId': 'arn:aws:kms:us-west-2:123456789012:key/325fea05-7af2-4573-bf75-01b61ed473c1',
            'Plaintext': b'\x86\xd1\xa18~3\xe2\xdc\x8eSp\xa0b \xbe\xa6\xf6\xd8C^\x00b\xe6\xbf\xc8EC\xe3\xdf0\xb2\xcc',
            'EncryptionAlgorithm': 'SYMMETRIC_DEFAULT',
            'ResponseMetadata': {
                'RequestId': '02597e38-79e5-4b6b-adcf-3435478b0a98',
                'HTTPStatusCode': 200,
                'HTTPHeaders': {
                    'x-amzn-requestid': '02597e38-79e5-4b6b-adcf-3435478b0a98',
                    'cache-control': 'no-cache, no-store, must-revalidate, private',
                    'expires': '0',
                    'pragma': 'no-cache',
                    'date': 'Mon, 12 Jul 2021 20:31:45 GMT',
                    'content-type': 'application/x-amz-json-1.1',
                    'content-length': '188'
                },
                'RetryAttempts': 0
            }
        }

    @staticmethod
    def get_kms_decrypted_s3_tokens():
        """
        Get the response of calling kms.decrypt() on the encrypted tokens that were fetched from S3.
        """
        return {
            'KeyId': 'arn:aws:kms:us-west-2:123456789012:key/325fea05-7af2-4573-bf75-01b61ed473c1',
            'Plaintext': b'\x8dq\xfa\xddz\x81\xd3\xbe(\x97\xf6O\x18\xf2\xcfn\x9e\xe9r\xf4c;\x9f\xcb\xe7\xdd\x17c\xbe\xe2-\xaa',
            'EncryptionAlgorithm': 'SYMMETRIC_DEFAULT',
            'ResponseMetadata': {
                'RequestId': 'fd36a995-c6c7-4817-a880-6dba272729aa',
                'HTTPStatusCode': 200,
                'HTTPHeaders': {
                    'x-amzn-requestid': 'fd36a995-c6c7-4817-a880-6dba272729aa',
                    'cache-control': 'no-cache, no-store, must-revalidate, private',
                    'expires': '0',
                    'pragma': 'no-cache',
                    'date': 'Mon, 12 Jul 2021 20:31:49 GMT',
                    'content-type': 'application/x-amz-json-1.1',
                    'content-length': '188'
                },
                'RetryAttempts': 0
            }
        }
