output "lambda_function_arn" {
  value = module.lambda_edge_function.lambda_function_arn
}

output "lambda_function_version" {
  value = module.lambda_edge_function.lambda_function_version
}

output "cloudfront_distribution_id" {
  value = module.cloudfront_distribution.cloudfront_distribution_id
}

output "s3_bucket_domain_name" {
  value = module.s3_bucket.bucket_domain_name
}

output "account_id" {
  value = data.aws_caller_identity.current.account_id
}
