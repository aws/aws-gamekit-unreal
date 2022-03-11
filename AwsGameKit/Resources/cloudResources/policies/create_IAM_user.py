# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

import argparse
import json
import logging
import sys

import boto3
import botocore

import generate_policy_instance

GROUP_NAME = "GameKitDevGroup"

parser = argparse.ArgumentParser(description='This script will create or update an IAM user account, and produce an access key and '
                                             'secret key if the user is newly created. Then it will attach all relevant policies to '
                                             f'{GROUP_NAME} group, and add the user to that group.')
parser.add_argument("username", type=str, help='IAM Username you wish to attach GameKit policies to.')
parser.add_argument('access_key', type=str, help='AWS access key. Admin permissions or create_user, create_policy, and attach_policy permissions.')
parser.add_argument('secret_key', type=str, help='AWS secret key. Admin permissions or create_user, create_policy, and attach_policy permissions.')
args = parser.parse_args()

def create_user(client, username):
    """
    Creates a user. By default, a user has no permissions or access keys.

    :param client: A boto3 IAM client
    :param username: The name of the user.
    :return: The newly created user.
    """
    try:
        client.create_user(UserName=username)
        logging.info(f"Created user: {username}")
    except botocore.exceptions.ClientError:
        logging.exception(f"Couldn't create user {username}.")
        raise
    else:
        return


def create_policies(client, policy_string_list, account_id):
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
        for policy in policy_string_list:
            p_name = f"GameKitDeveloperPolicy-{policy_number}"
            arn = f'{arn_base}{p_name}'

            try:
                # Cleanup old policy
                policy_response = client.get_policy(PolicyArn=arn)
                client.create_policy_version(
                    PolicyArn=arn,
                    PolicyDocument=policy,
                    SetAsDefault=True
                )
                client.delete_policy_version(PolicyArn=arn, VersionId=policy_response['Policy']['DefaultVersionId'])
            except client.exceptions.ClientError:
                client.create_policy(
                    PolicyName=p_name,
                    PolicyDocument=policy,
                )
            created_policies.append(arn)
            policy_number += 1
            logging.info(f"Created {p_name} policy.")
    except client.exceptions.ClientError:
        logging.exception(f"Couldn't create policy.")
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
        logging.exception(f"Couldn't create GameKit-dev policy.")
        raise


def attach_policies(client, policy_list):
    """
    Attaches a policy to a group which the user was assigned to.

    :param group: The username.
    :param policy_list: The Amazon Resource Name (ARN) of the policy.
    """
    try:
        for policy_arn in policy_list:
            client.attach_group_policy(GroupName=GROUP_NAME, PolicyArn=policy_arn)
    except client.exceptions.ClientError:
        logging.exception(f"Couldn't attach policy to group {GROUP_NAME}.")
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

        credentials_file = f'{username}_credentials.txt'
        with open(credentials_file, 'w') as cred:
            cred.write(f"aws_access_key: {key_pair['AccessKeyId']}\naws_secret_access_key: {key_pair['SecretAccessKey']}")
        logging.info(f"Access key and secret key for {username} written to: {credentials_file} in the current directory.")
    except botocore.exceptions.ClientError:
        logging.exception(f"Couldn't create access key pair for {username}.")
        raise
    else:
        return key_pair


def main():
    logging.basicConfig(level=logging.INFO)

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
        logging.info(f"Existing IAM user: {args.username}")
    except iam_client.exceptions.NoSuchEntityException:
        create_user(iam_client, args.username)
        create_key(iam_client, args.username)
    except iam_client.exceptions.ServiceFailureException:
        logging.exception(f"Couldn't create a user account for {account_id}.")
        raise

    # generate policy file
    policy_filename = generate_policy_instance.main(account_id)
    with open(policy_filename, 'r') as file:
        policy_contents = json.loads(file.read())

    # split policy statements (policy is too large to upload in one policy)
    policy_statements = split_policy(policy_contents)

    # create policies
    policy_list = create_policies(iam_client, policy_statements, account_id)

    # create gamekit group if it doesn't exist
    try:
        iam_client.create_group(GroupName=GROUP_NAME)
        logging.info(f"Created IAM group: {GROUP_NAME}")
    except iam_client.exceptions.EntityAlreadyExistsException:
        logging.info(f"Existing IAM group: {GROUP_NAME}")

    iam_client.add_user_to_group(UserName=args.username, GroupName=GROUP_NAME)

    # attach policies to group
    attach_policies(iam_client, policy_list)
    logging.info(f"Attached GameKitDev policies to group {GROUP_NAME}, and added user {args.username} to the group.")

if __name__ == "__main__":
    main()
