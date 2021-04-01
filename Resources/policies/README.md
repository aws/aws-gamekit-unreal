# Create an IAM user

## Requirement
Install Boto3
`pip install -r requirements.txt`

## Instruction
Call create_IAM_user.py script as follow:
`python create_IAM_user.py [AWS USERNAME] [AWS ACCESS KEY] [AWS SECRET KEY]`
The create_IAM_user.py script requires 3 arguments, username, AWS access key, and AWS secret key with Admin permissions. If all information is provided correctly the user will have an account created for them with all the required GameKit permissions attached and the new access and secret key will be shown in stdout.