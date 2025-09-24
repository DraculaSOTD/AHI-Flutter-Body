#!/bin/bash

# Function name
FUNCTION_NAME="ahi-web-bhalite-prod"

# List of all AWS regions
REGIONS=$(aws ec2 describe-regions --query "Regions[*].RegionName" --output text)

# Loop through all regions to delete the Lambda function
for REGION in $REGIONS; do
    echo "Deleting function $FUNCTION_NAME in region $REGION"
    aws lambda delete-function --function-name $FUNCTION_NAME --region $REGION
done

# Delete the main Lambda function in the primary region
PRIMARY_REGION="us-east-1"
echo "Deleting main function $FUNCTION_NAME in region $PRIMARY_REGION"
aws lambda delete-function --function-name $FUNCTION_NAME --region $PRIMARY_REGION
