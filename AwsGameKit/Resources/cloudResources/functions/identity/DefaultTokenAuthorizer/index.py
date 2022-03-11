# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

"""
Purpose

This is a custom lambda authorizer.
Adapted from https://github.com/awslabs/aws-apigateway-lambda-authorizer-blueprints/blob/master/blueprints/python/api-gateway-authorizer-python.py
"""

import logging
import os
import re
from . import token_verifier
from distutils.util import strtobool

logger = logging.getLogger()
logger.setLevel(logging.INFO)


def lambda_handler(event, context):
    """
    This is the lambda function handler.
    """

    # Validate the incoming token
    raw_token = event['authorizationToken']
    verify_expiration = bool(strtobool(os.environ.get("VERIFY_EXPIRATION", 'true')))
    token_arr = raw_token.split(" ")
    token = token_arr[len(token_arr) - 1]
    claims, ok = token_verifier.verify(token, 'AWSCURRENT', verify_expiration)

    if not ok:
        logger.info(f"Cannot verify token using AWSCURRENT. Trying AWSPREVIOUS...")
        claims, ok = token_verifier.verify(token, 'AWSPREVIOUS', verify_expiration)

        if not ok:
            logger.error("Error getting secret")
            raise Exception('Unauthorized')

    player_id_claim_field = os.environ.get('USER_IDENTIFIER_CLAIM_FIELD')
    player_id = claims[player_id_claim_field]
    if player_id is None:
        logger.error(f"Claim '{player_id_claim_field}' not found in token.")
        raise Exception('Unauthorized')

    # Parse methodArn, which is of the format:
    #   arn:aws:execute-api:<REGION>:<ACCOUNTID>:<RESTAPI_ID>/<STAGE>/<METHOD>/<METHOD_PATH>
    #   Example: arn:aws:execute-api:us-west-2:123456789012:a1b2c3d4e5/dev/POST/pets/toys
    method_arn = event['methodArn'].split(':')
    apig_method_path = method_arn[5].split('/')
    aws_account_id = method_arn[4]

    # Generate policy
    policy = AuthPolicy(player_id, aws_account_id)
    policy.restApiId = apig_method_path[0]
    policy.region = method_arn[3]
    policy.stage = apig_method_path[1]

    # Add endpoints to policy
    endpoints = os.environ['ENDPOINTS_ALLOWED'].split(',')
    for endpoint in endpoints:
        policy.allowMethod(HttpVerb.ALL, endpoint.strip())

    auth_response = policy.build()

    # Set player_id in context
    context = {
        'custom:thirdparty_player_id': player_id,  # Available as $context.authorizer.custom:thirdparty_player_id
    }
    auth_response['context'] = context

    return auth_response

class HttpVerb:
    GET = "GET"
    POST = "POST"
    PUT = "PUT"
    PATCH = "PATCH"
    HEAD = "HEAD"
    DELETE = "DELETE"
    OPTIONS = "OPTIONS"
    ALL = "*"


class AuthPolicy(object):
    # The policy version used for the evaluation. This should always be '2012-10-17'
    version = "2012-10-17"

    # The regular expression used to validate resource paths for the policy
    pathRegex = "^[/.a-zA-Z0-9-\*]+$"

    def __init__(self, principal, awsAccountId):
        self.awsAccountId = awsAccountId
        self.principalId = principal
        self.allowMethods = []
        self.denyMethods = []

    def _addMethod(self, effect, verb, resource, conditions):
        """Adds a method to the internal lists of allowed or denied methods. Each object in
        the internal list contains a resource ARN and a condition statement. The condition
        statement can be null."""
        if verb != "*" and not hasattr(HttpVerb, verb):
            raise NameError("Invalid HTTP verb " + verb + ". Allowed verbs in HttpVerb class")
        resourcePattern = re.compile(self.pathRegex)
        if not resourcePattern.match(resource):
            raise NameError("Invalid resource path: " + resource + ". Path should match " + self.pathRegex)

        if resource[:1] == "/":
            resource = resource[1:]

        resourceArn = ("arn:aws:execute-api:" +
                       self.region + ":" +
                       self.awsAccountId + ":" +
                       self.restApiId + "/" +
                       self.stage + "/" +
                       verb + "/" +
                       resource)

        if effect.lower() == "allow":
            self.allowMethods.append({
                'resourceArn': resourceArn,
                'conditions': conditions
            })
        elif effect.lower() == "deny":
            self.denyMethods.append({
                'resourceArn': resourceArn,
                'conditions': conditions
            })

    def _getEmptyStatement(self, effect):
        """Returns an empty statement object prepopulated with the correct action and the
        desired effect."""
        statement = {
            'Action': 'execute-api:Invoke',
            'Effect': effect[:1].upper() + effect[1:].lower(),
            'Resource': []
        }

        return statement

    def _getStatementForEffect(self, effect, methods):
        """This function loops over an array of objects containing a resourceArn and
        conditions statement and generates the array of statements for the policy."""
        statements = []

        if len(methods) > 0:
            statement = self._getEmptyStatement(effect)

            for curMethod in methods:
                if curMethod['conditions'] is None or len(curMethod['conditions']) == 0:
                    statement['Resource'].append(curMethod['resourceArn'])
                else:
                    conditionalStatement = self._getEmptyStatement(effect)
                    conditionalStatement['Resource'].append(curMethod['resourceArn'])
                    conditionalStatement['Condition'] = curMethod['conditions']
                    statements.append(conditionalStatement)

            statements.append(statement)

        return statements

    def allowAllMethods(self):
        """Adds a '*' allow to the policy to authorize access to all methods of an API"""
        self._addMethod("Allow", HttpVerb.ALL, "*", [])

    def denyAllMethods(self):
        """Adds a '*' allow to the policy to deny access to all methods of an API"""
        self._addMethod("Deny", HttpVerb.ALL, "*", [])

    def allowMethod(self, verb, resource):
        """Adds an API Gateway method (Http verb + Resource path) to the list of allowed
        methods for the policy"""
        self._addMethod("Allow", verb, resource, [])

    def denyMethod(self, verb, resource):
        """Adds an API Gateway method (Http verb + Resource path) to the list of denied
        methods for the policy"""
        self._addMethod("Deny", verb, resource, [])

    def allowMethodWithConditions(self, verb, resource, conditions):
        """Adds an API Gateway method (Http verb + Resource path) to the list of allowed
        methods and includes a condition for the policy statement. More on AWS policy
        conditions here: http://docs.aws.amazon.com/IAM/latest/UserGuide/reference_policies_elements.html#Condition"""
        self._addMethod("Allow", verb, resource, conditions)

    def denyMethodWithConditions(self, verb, resource, conditions):
        """Adds an API Gateway method (Http verb + Resource path) to the list of denied
        methods and includes a condition for the policy statement. More on AWS policy
        conditions here: http://docs.aws.amazon.com/IAM/latest/UserGuide/reference_policies_elements.html#Condition"""
        self._addMethod("Deny", verb, resource, conditions)

    def build(self):
        """Generates the policy document based on the internal lists of allowed and denied
        conditions. This will generate a policy with two main statements for the effect:
        one statement for Allow and one statement for Deny.
        Methods that includes conditions will have their own statement in the policy."""
        if ((self.allowMethods is None or len(self.allowMethods) == 0) and
                (self.denyMethods is None or len(self.denyMethods) == 0)):
            raise NameError("No statements defined for the policy")

        policy = {
            'principalId': self.principalId,
            'policyDocument': {
                'Version': self.version,
                'Statement': []
            }
        }

        policy['policyDocument']['Statement'].extend(self._getStatementForEffect("Allow", self.allowMethods))
        policy['policyDocument']['Statement'].extend(self._getStatementForEffect("Deny", self.denyMethods))

        return policy
