# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

# To successfully run, attach the policy in this directory to the IAM user whose credentials are being used.

import argparse
import time

import boto3
import botocore
from getpass import getpass
from numpy import base_repr

POLICY_TIP = "\nIf permission error make sure to attach the policy in this directory to the IAM user whose credentials are being used.\n"

def cf_stack_delete_success_states():
    return ['DELETE_COMPLETE']

def cf_stack_delete_failed_states():
    return ['CREATE_COMPLETE',
            'ROLLBACK_COMPLETE',
            'ROLLBACK_FAILED',
            'DELETE_FAILED',
            'UPDATE_COMPLETE',
            'UPDATE_ROLLBACK_COMPLETE',
            'UPDATE_ROLLBACK_FAILED',
            'IMPORT_ROLLBACK_FAILED']

def cf_stack_delete_states():
    return cf_stack_delete_success_states() + cf_stack_delete_failed_states()

def region_code_mapping():
    return {
        'us-east-1': 'usea1',
        'us-east-2': 'usea2',
        'us-west-1': 'uswe1',
        'us-west-2': 'uswe2',
        'af-south-1': 'afso1',
        'ap-east-1': 'apea1',
        'ap-south-1': 'apso1',
        'ap-northeast-3': 'apne3',
        'ap-northeast-2': 'apne2',
        'ap-southeast-1': 'apse1',
        'ap-southeast-2': 'apse2',
        'ap-northeast-1': 'apne1',
        'ca-central-1': 'cace1',
        'eu-central-1': 'euce1',
        'eu-west-1': 'euwe1',
        'eu-west-2': 'euwe2',
        'eu-south-1': 'euso1',
        'eu-west-3': 'euwe3',
        'eu-north-1': 'euno1',
        'me-south-1': 'meso1',
        'sa-east-1': 'saea1'
    }

class bcolors:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKCYAN = '\033[96m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'


class GameKitCleaner:
    def __init__(self, gamename, environment, region, keyid, secret):
        self.gamename = gamename
        self.environment = environment
        self.region = region
        self.keyid = keyid
        self.secret = secret

    def main_stack_name(self):
        return f"gamekit-{self.environment}-{self.gamename}-main"

    def delete_stack(self):
        cfclient = boto3.client('cloudformation', aws_access_key_id=self.keyid, aws_secret_access_key=self.secret)
        print(f"Deleting Stack {self.main_stack_name()}...")
        try:
            cfclient.describe_stacks(StackName=self.main_stack_name())
        except botocore.exceptions.ClientError:
            print(POLICY_TIP)
            print(f"\n{bcolors.FAIL}Stack '{self.main_stack_name()}' does not exist.{bcolors.ENDC}")
            return

        cfclient.delete_stack(StackName=self.main_stack_name())

        last_stack_event = ""
        while last_stack_event not in cf_stack_delete_states():
            events = cfclient.describe_stack_events(StackName=self.main_stack_name())
            last_stack_event = events['StackEvents'][0]['ResourceStatus']
            status_reason = events['StackEvents'][0].get('ResourceStatusReason', '-')
            print(f"   {last_stack_event} : {status_reason}                                                                         ", end="\r")
            time.sleep(3)

        print(f"   Done. Last Status={last_stack_event}                                                                             ", end="\r")
        print()
        return last_stack_event

    def delete_parameters(self):
        ssmclient = boto3.client('ssm', aws_access_key_id=self.keyid, aws_secret_access_key=self.secret)
        parameters = ssmclient.describe_parameters(ParameterFilters=[{'Key': 'Name', 'Option': 'Contains', 'Values': [f"{self.gamename}_{self.environment}"]}], MaxResults=50)
        for param in parameters['Parameters']:
            print(f"Deleting Parameter {param['Name']}... ", end="")
            try:
                ssmclient.delete_parameter(Name=param['Name'])
                print("Done.")
            except botocore.exceptions.ClientError as err:
                print(POLICY_TIP)
                print(err)

    def delete_secrets(self):
        secretsclient = boto3.client('secretsmanager', aws_access_key_id=self.keyid, aws_secret_access_key=self.secret)
        secrets = secretsclient.list_secrets(Filters=[{'Key': 'name', 'Values': [f"gamekit_{self.environment}_{self.gamename}_"]}])
        for secret in secrets['SecretList']:
            print(f"Deleting Secret {secret['Name']}... ", end="")
            try:
                secretsclient.delete_secret(SecretId=secret['ARN'], ForceDeleteWithoutRecovery=True)
                print("Done.")
            except botocore.exceptions.ClientError as err:
                print(POLICY_TIP)
                print(err)

    def delete_log_groups(self):
        logsclient = boto3.client('logs')

        # Only left over log group for the main stack
        log_group_name = f"/aws/lambda/gamekit_{self.environment}_{self.gamename}_main_RemoveLambdaLayersOnDelete"
        print(f"Deleting Log Group {log_group_name}... ", end="")
        try:
            response = logsclient.delete_log_group(logGroupName=log_group_name)
            print("Done.")
        except botocore.exceptions.ClientError as err:
            print(POLICY_TIP)
            print(err)

    def delete_bootstrap_bucket(self):
        stsclient = boto3.client('sts')
        identity = stsclient.get_caller_identity()

        s3client = boto3.client('s3')
        bootstrap_bucket_name = f"do-not-delete-gamekit-{self.environment}-{region_code_mapping()[self.region]}-{base_repr(int(identity['Account']), 36).lower()}-{self.gamename}"
        print(f"Deleting objects in S3 Bucket {bootstrap_bucket_name}... ")
        objects = s3client.list_objects_v2(Bucket=bootstrap_bucket_name)
        for obj in objects['Contents']:
            print(f"Deleting Object {obj['Key']}... ", end="")
            try:
                s3client.delete_object(Bucket=bootstrap_bucket_name, Key=obj['Key'])
                print("Done.")
            except botocore.exceptions.ClientError as err:
                print(POLICY_TIP)
                print(err)

        print(f"Deleting S3 Bucket {bootstrap_bucket_name}... ", end="")
        try:
            s3client.delete_bucket(Bucket=bootstrap_bucket_name)
            print("Done.")
        except botocore.exceptions.ClientError as err:
            print(POLICY_TIP)
            print(err)

def main(*args):
    gamekit_cleaner = GameKitCleaner(*args)
    event = gamekit_cleaner.delete_stack()
    if event in cf_stack_delete_failed_states():
       print(f"\n{bcolors.FAIL}Unable to delete Stack '{gamekit_cleaner.main_stack_name()}'. Aborting.{bcolors.ENDC}")
       exit(1)
    gamekit_cleaner.delete_parameters()
    gamekit_cleaner.delete_secrets()
    gamekit_cleaner.delete_log_groups()
    gamekit_cleaner.delete_bootstrap_bucket()

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description="Cleans up resources from an Aws GameKit deployment which aren't removed in CloudFormation stack deletion.")
    parser.add_argument("--project_alias", help="Project alias used when deploying with Aws GameKit plugin.")
    parser.add_argument("--env", help="Short code for your environment e.g. (dev, stg, prd)")
    parser.add_argument("--region", help="Aws region the stack is deployed in")
    parser.add_argument("--access_key", help="Aws access key with permissions to delete all resources.")
    parser.add_argument("--secret_key", help="Aws secret key with permissions to delete all resources.")
    args = parser.parse_args()

    project_alias = args.project_alias if args.project_alias else input('Game Alias: ')
    env = args.env if args.env else input('Environment: (env code like dev, stg, prd): ')
    region = args.region if args.region else input('AWS Region: ')
    access_key = args.access_key if args.access_key else input('AWS Access Key ID: ')
    secret_key = args.secret_key if args.secret_key else getpass(prompt='AWS Secret Access Key: ')

    main(project_alias, env, region, access_key, secret_key)
    print("\n---\nGameKit Cleanup done.")
