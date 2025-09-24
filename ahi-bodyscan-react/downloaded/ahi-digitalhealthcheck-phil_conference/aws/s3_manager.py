import hashlib
import logging
import mimetypes
import time
from pathlib import Path

import boto3

# Initialize logger
logging.basicConfig(level=logging.INFO)


def get_s3_key(file, directory_path):
    return "/assets/" + str(file.relative_to(directory_path)).replace("\\", "/")


def is_valid_file(file):
    return file.is_file() and not file.name == ".DS_Store"


def upload_directory_to_s3(bucket_name, directory_path, s3_client):
    path = Path(directory_path)
    for file in path.rglob("*"):
        if is_valid_file(file):
            s3_key = get_s3_key(file, directory_path)

            local_etag = f'"{hashlib.md5(file.read_bytes()).hexdigest()}"'
            try:
                response = s3_client.head_object(Bucket=bucket_name, Key=s3_key)
                s3_etag = response.get("ETag")
            except Exception as e:
                logging.error(e)
                s3_etag = None

            if s3_etag != local_etag:
                content_type, _ = mimetypes.guess_type(str(file))
                content_type = content_type or "application/octet-stream"
                s3_client.upload_file(
                    str(file),
                    bucket_name,
                    s3_key,
                    ExtraArgs={"ContentType": content_type},
                )
                logging.info(
                    f"Uploaded {file} to {bucket_name}/{s3_key} with content type {content_type}"
                )
            else:
                logging.info(f"{file} is already up-to-date")


def delete_unmatched_s3_objects(bucket_name, directory_path, s3_client):
    s3_objects = s3_client.list_objects(Bucket=bucket_name)
    s3_keys = [obj["Key"] for obj in s3_objects.get("Contents", [])]

    local_files = [
        get_s3_key(file, directory_path)
        for file in Path(directory_path).rglob("*")
        if is_valid_file(file)
    ]

    unmatched_s3_keys = set(s3_keys) - set(local_files)
    for s3_key in unmatched_s3_keys:
        s3_client.delete_object(Bucket=bucket_name, Key=s3_key)
        logging.info(f"Deleted {s3_key} from {bucket_name}")


def create_invalidation(distribution_id, paths, cloudfront_client):
    invalidation = cloudfront_client.create_invalidation(
        DistributionId=distribution_id,
        InvalidationBatch={
            "Paths": {"Quantity": len(paths), "Items": paths},
            "CallerReference": str(hash(frozenset(paths))),
        },
    )
    invalidation_id = invalidation["Invalidation"]["Id"]
    print(f"Created invalidation for {paths} in {distribution_id}")

    # Check the status of the invalidation until it's completed
    while True:
        invalidation = cloudfront_client.get_invalidation(
            DistributionId=distribution_id, Id=invalidation_id
        )
        status = invalidation["Invalidation"]["Status"]
        if status == "Completed":
            print(f"Invalidation {invalidation_id} is completed")
            break
        else:
            print(f"Invalidation {invalidation_id} is still in progress")
            time.sleep(10)  # Wait for 10 seconds before checking the status again


if __name__ == "__main__":
    # Initialize clients
    session = boto3.Session(profile_name="default")
    s3_client = boto3.client("s3", region_name="us-east-1")
    cloudfront_client = boto3.client("cloudfront", region_name="us-east-1")

    # Specify your bucket name, directory path and CloudFront distribution ID
    bucket_name = "biometrichealth.tech.prod"
    directory_path = "./src/ahi-web-bhalite-prod/assets/"
    distribution_id = "E2IB4WFPC9JC1Z"  # WARNING This could change

    # Upload directory to S3
    delete_unmatched_s3_objects(bucket_name, directory_path, s3_client)
    upload_directory_to_s3(bucket_name, directory_path, s3_client)

    # Create invalidation in CloudFront
    # Here, we're invalidating everything ('/*'), but you can specify the paths you want to invalidate
    create_invalidation(distribution_id, ["/*"], cloudfront_client)
