output "bucket_domain_name" {
  description = "The domain name of the S3 bucket"
  value       = aws_s3_bucket.bucket.bucket_domain_name
}

output "origin_access_control_id" {
  value = aws_cloudfront_origin_access_control.oac.id
}
