# AWS GameKit Cloud Resources
This folder contains the cloud resources which get deployed to your AWS account when you deploy a feature.

Here you will find these folders:
* `functions/` - Contains all the Lambda functions.
* `functionsTests/` - Unit tests for the Lambda functions.
* `cloudFormation/` - CloudFormation templates which specify the AWS resources to deploy for each feature.
* `configOutputs/` - Placeholder config files which will be copied & filled in when their corresponding feature is deployed.

## Lambda Functions
The Lambda functions are written in Python 3.7.

Learn more about building Lambda functions with Python here: https://docs.aws.amazon.com/lambda/latest/dg/lambda-python.html

### Unit Tests
The unit tests use Python's standard test framework: [unittest](https://docs.python.org/3/library/unittest.html)

#### Prerequisites:
1. Install [Python 3.7](https://www.python.org/downloads/) (the version used in the Lambda functions).
1. Create a virtual environment named `venv` in the current directory: `python -m venv venv`
1. Activate the venv:
   1. Go to: https://docs.python.org/3.7/library/venv.html#creating-virtual-environments
   1. Search for: `Command to activate virtual environment`
   1. Run the command for your Platform/Shell.
1. Install dependencies in the venv: `pip install -r requirements.txt`
1. Install `gamekithelpers` from the local filesystem in the venv: `pip install layers\main\CommonLambdaLayer`

#### Note Before Running Tests:
In order to ensure AWS resources are being mocked properly, developers should temporarily remove their default AWS credentials.
In the case default credentials exist and something is being mocked incorrectly boto will automatically use the default AWS credentials.
Which can lead to unexpected behavior and calls to AWS.

#### Run the Tests:
1. Open up a shell/terminal.
1. Activate the `venv` (see prerequisites).
1. Run tests as follows:
```shell
# Run all unit tests:
python -m unittest

# Run tests in one file:
python -m unittest functionsTests.test_identity.test_CognitoPreSignUp.test_index

# Run a single unit test:
python -m unittest functionsTests.test_identity.test_CognitoPreSignUp.test_index.TestIndex.test_new_user_can_be_registered_by_email_successfully
```

For a comprehensive guide on Python unit testing, please see the official unittest docs: https://docs.python.org/3/library/unittest.html#command-line-interface
