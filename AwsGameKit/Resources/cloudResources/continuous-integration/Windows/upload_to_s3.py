import argparse
import logging
import pathlib
import re
import shutil
import sys
import tempfile

import boto3

def get_version(version_path):
	with open(version_path, 'r') as v:
		version = v.read().strip()

	version_regex = "v(\d+)\.(\d+)\.(\d+)"
	match = re.match(version_regex, version)

	if match:
		return version
	logging.error(f"Does not match version pattern vX.X.X .. got {version} instead.")
	sys.exit(2)

if __name__ == "__main__":
	parser = argparse.ArgumentParser(description="Builds Aws GameKit Sdk and publishes it to github and uploads to s3.")
	parser.add_argument("--s3_bucket", required=True, help="Bucket where artifact should be output to.")
	args = parser.parse_args()

	logging.basicConfig(level=logging.INFO)
	script_path = pathlib.Path(__file__).absolute()
	repository_root = script_path.parents[2]

	version = get_version(script_path.parents[1] / ".version")

	tmp_dir = tempfile.TemporaryDirectory().name
	shutil.copytree(repository_root, pathlib.Path(tmp_dir), dirs_exist_ok=True)
	logging.info("Zipping up artifact ...")
	shutil.make_archive(tmp_dir, "zip", tmp_dir)
	logging.info("Artifact zipped.")
	s3_client = boto3.client('s3', region_name='us-west-2')
	s3_client.put_object(Bucket=args.s3_bucket, Key=version + ".zip", Body=open(f'{tmp_dir}.zip', 'rb'))