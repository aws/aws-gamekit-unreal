# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

import argparse
import sys

import boto3
import botocore
import json
import logging

import generate_policy_instance

GROUP_NAME = "GameKitDevGroup"

parser = argparse.ArgumentParser(description='This script will get or create an IAM user account as well as a pair of '
                                             'access keys if the user is a newly created. Then it will attach all relevant policies to '
                                             f'{GROUP_NAME} group, and add the user to that group.')
parser.add_argument("username", type=str, help='IAM Username you wish to attach GameKit policies to.')
parser.add_argument('access_key', type=str, help='AWS access key. Admin permissions or create_user, create_policy, and attach_policy permissions.')
parser.add_argument('secret_key', type=str, help='AWS secret key. Admin permissions or create_user, create_policy, and attach_policy permissions.')
args = parser.parse_args()

logger = logging.getLogger(__name__)

def create_user(client, username):
    """
    Creates a user. By default, a user has no permissions or access keys.

    :param client: A boto3 IAM client
    :param username: The name of the user.
    :return: The newly created user.
    """
    try:
        client.create_user(UserName=username)
        logger.info(f"Created user {username}.")
    except botocore.exceptions.ClientError:
        logger.exception(f"Couldn't create user {username}.")
        raise
    else:
        return

def create_policies(client, policy_list, account_id):
    """
    Creates a policy that contains a single statement.

    :param client: A boto3 IAM client
    :param policy_string_list: A list of generated policy in string format
    :return: The list of newly created policy.
    """
    arn_base = f"arn:aws:iam::{account_id}:policy/"
    created_policies = []
    try:
        # Attach all the new policies
        policy_number = 1
        for policy in policy_list:
            p_name = f"GameKitDeveloperPolicy-{policy_number}"

            try:
                client.delete_policy(PolicyArn=f'{arn_base}{p_name}')
            except client.exceptions.ClientError:
                pass

            p = client.create_policy(
                    PolicyName=p_name,
                    PolicyDocument=policy
                )
            created_policies.append(p)
            policy_number += 1
        logger.info(f"Created GameKit-dev policy.")
    except client.exceptions.ClientError:
        logger.exception(f"Couldn't create GameKit-dev policy.")
        raise
    else:
        return created_policies


def split_policy(policy_contents):
    """
    Split a policy into multiple policies.

    :param policy_contents: the policy that needs to be split into pieces
    """

    POLICY_MAX_LENGTH = 5000;
    try:
        policy_list = []
        version = policy_contents["Version"]
        current_policy = {"Version": version, "Statement": []}
        for statement in policy_contents["Statement"]:
            current_length = len(json.dumps(current_policy["Statement"]))
            statement_length = len(json.dumps(statement))
            if current_length + statement_length > POLICY_MAX_LENGTH:
                policy_list.append(json.dumps(current_policy))
                current_policy = {"Version": version, "Statement": [statement]}
            else:
                current_policy["Statement"].append(statement)

        policy_list.append(json.dumps(current_policy))

        return policy_list
    except botocore.exceptions.ClientError:
        logger.exception(f"Couldn't create GameKit-dev policy.")
        raise


def attach_policies(client, policy_list):
    """
    Attaches a policy to a group which the user was assigned to.

    :param group: The username.
    :param policy_list: The Amazon Resource Name (ARN) of the policy.
    """
    try:
        for policy in policy_list:
            client.attach_group_policy(GroupName=GROUP_NAME, PolicyArn=policy['Policy']['Arn'])
    except client.exceptions.ClientError:
        logger.exception(f"Couldn't attach policy to group {GROUP_NAME}.")
        raise


def create_key(client, username):
    """
    Creates an access key for the specified user. Each user can have a
    maximum of two keys.

    :param client: A boto3 IAM client
    :param username: The name of the user.
    :return: The created access key.
    """
    try:
        key_pair = client.create_access_key(UserName=username).get("AccessKey")
        client.update_access_key(AccessKeyId=key_pair["AccessKeyId"], Status='Active', UserName=username)
        logger.info(
            f"\n\nCreated access key pair for {username}. \naws_access_key_id: {key_pair['AccessKeyId']}"
            f"\naws_secret_access_key: {key_pair['SecretAccessKey']}\n\n")
    except botocore.exceptions.ClientError:
        logger.exception(f"Couldn't create access key pair for {username}.")
        raise
    else:
        return key_pair


def main():
    logging.basicConfig(level=logging.INFO, format='%(levelname)s: %(message)s')

    iam_client = boto3.client(
        'iam',
        aws_access_key_id=args.access_key,
        aws_secret_access_key=args.secret_key
    )
    sts_client = boto3.client(
        "sts",
        aws_access_key_id=args.access_key,
        aws_secret_access_key=args.secret_key
    )
    account_id = sts_client.get_caller_identity()["Account"]

    # create a user account
    try:
        iam_client.get_user(UserName=args.username)
        print(f"Existing IAM user: {args.username}")
    except iam_client.exceptions.NoSuchEntityException:
        if args.username is None:
            print("Please provide your username")
            sys.exit(1)
        else:
            create_user(iam_client, args.username)
            print(f"Created IAM users: {args.username}")
            create_key(iam_client, args.username)

    except iam_client.exceptions.ServiceFailureException:
        logger.exception(f"Couldn't create a user account for {account_id}.")
        raise

    # generate policy file
    policy_filename = generate_policy_instance.main(account_id)
    with open(policy_filename, 'r') as file:
        policy_contents = json.loads(file.read())

    # split policy statements (policy too large to upload in one policy)
    policy_statements = split_policy(policy_contents)

    # create policies
    policy_list = create_policies(iam_client, policy_statements, account_id)
    print(f"Created GameKit-dev policies from {policy_filename}")

    # create gamekit group if it doesn't exist
    try:
        iam_client.create_group(GroupName=GROUP_NAME)
    except iam_client.exceptions.EntityAlreadyExistsException:
        pass

    iam_client.add_user_to_group(UserName=args.username, GroupName=GROUP_NAME)

    # attach policies to group
    attach_policies(iam_client, policy_list)
    print(f"Attached GameKitDev policies to group {GROUP_NAME}, and added user {args.username} to the group.")

if __name__ == "__main__":
    main()
