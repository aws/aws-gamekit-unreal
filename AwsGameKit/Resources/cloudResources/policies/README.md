# Create an IAM user
- If you have an admin access key and secret key you can use `create_IAM_user.py` to create an IAM user in your account with all permissions needed to use Aws GameKit.
- It can also be used to update permissions for an existing user.

## Instructions 
- Install dependencies `pip install -r requirements.txt`
- Run `python create_IAM_user.py [AWS IAM USERNAME] [AWS ADMIN ACCESS KEY] [AWS ADMIN SECRET KEY]`
- The access key and secret access key for a new user will be output to a [username]_credentials.txt file in this directory.

# Creating Developer Policy Document
- If you wish to only create the policy document `generate_policy_instance.py` can be used.
- The json document produced grants permission to deploy and manage all AWS GameKit features in your account when uploaded to AWS IAM.

## Instructions
- Retrieve your 12 digit AWS Account ID after creating an AWS Account.
- Run `python generate_policy_instance.py [YOUR ACCOUNT ID]` from the command line in this directory.
- A file named `GameKitDeveloperPolicy_[your id]` will be produced in this directory, ready to upload.