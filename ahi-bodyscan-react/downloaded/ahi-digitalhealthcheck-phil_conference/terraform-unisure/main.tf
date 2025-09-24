provider "aws" {
  region = "us-east-1"
}

module "s3_bucket" {
  source                      = "./s3_bucket"
  bucket_name                 = "${var.origin_bucket_domain_name}${var.origin_tld}.prod"
  domain_name                 = var.origin_bucket_domain_name
  cloudfront_distribution_arn = module.cloudfront_distribution.cloudfront_distribution_id
  source_path = "../src/ahi-web-bhalite-prod"
}

module "lambda_edge_function" {
  source        = "./lambda_edge"
  function_name = var.function_name
  handler       = "index.handler"
  runtime       = "python3.11"
  source_path   = "../src/ahi-web-bhalite-prod"
  dist_path     = "../dist/ahi-web-bhalite-prod"

  url = "https://${var.origin_bucket_domain_name}${var.origin_tld}"
}

module "cloudfront_distribution" {
  source                    = "./cloudfront"
  origin_access_control_id  = module.s3_bucket.origin_access_control_id
  lambda_function_arn       = module.lambda_edge_function.lambda_function_arn
  lambda_function_version   = module.lambda_edge_function.lambda_function_version
  origin_bucket_domain_name = module.s3_bucket.bucket_domain_name
  origin_id                 = "s3-origin"
  acm_certificate_arn       = var.acm_cert_arn
  origin_domain_name        = var.origin_bucket_domain_name
  aliases                   = ["${var.origin_bucket_domain_name}${var.origin_tld}"]
}

module "route_53" {
  source                              = "./route_53"
  domain_name                         = "${var.origin_bucket_domain_name}${var.origin_tld}"
  cloudfront_distribution_id          = module.cloudfront_distribution.cloudfront_distribution_id
  cloudfront_distribution_domain_name = module.cloudfront_distribution.cloudfront_distribution_domain_name
}

module "dynamodb_table" {
  source     = "./dynamodb"
  table_name = "ahi-bha-lite-web-session-db"
}

