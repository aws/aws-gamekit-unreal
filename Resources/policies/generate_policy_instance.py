# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0
  
import sys
import argparse

def main(account_id):
    #input file
    with open('GameKitDeveloperPolicy_Template.json', 'r') as file:
        filedata = file.read()
  
    # Replace the target string
    filedata = filedata.replace('<YOUR_ACCOUNT_ID>', account_id)
  
    with open(f"GameKitDeveloperPolicy_{account_id}.json", 'w') as file:
        file.write(filedata)

    return f"GameKitDeveloperPolicy_{account_id}.json"
  
if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Create a GameKitDeveloperPolicy file using your AWS Account ID.')
    parser.add_argument('account_id', type=str, help='Your account ID.')
    args = parser.parse_args()

    main(args.account_id)