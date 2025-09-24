provider "aws" {
  region = "us-east-1"
}

################################################################################
# Generate the CDN sign key group
################################################################################

resource "tls_private_key" "ahi_bhalite_files_keypair" {
	algorithm = "RSA"
}

resource "aws_cloudfront_public_key" "ahi_bhalite_files_keypair_public" {
	encoded_key = tls_private_key.ahi_bhalite_files_keypair.public_key_pem
}

resource "aws_cloudfront_key_group" "ahi_bhalite_files_keypair_group" {
	items = [aws_cloudfront_public_key.ahi_bhalite_files_keypair_public.id]
	name  = "ahi-bhalite-cf-files-keygroup"
}


################################################################################
# CDN 
################################################################################

resource "aws_cloudfront_distribution" "cloudfront_distribution" {
  enabled = true
  origin {
    domain_name              = var.origin_bucket_domain_name
    origin_id                = var.origin_id
    origin_access_control_id = var.origin_access_control_id
  }
  # Default WAF Web ACL (Enable for WAF protections.)
  # web_acl_id = aws_wafv2_web_acl.spa.arn

  default_cache_behavior {

    target_origin_id = var.origin_id

    allowed_methods        = ["DELETE", "GET", "HEAD", "OPTIONS", "PATCH", "POST", "PUT"]
    cached_methods         = ["GET", "HEAD"]
    viewer_protocol_policy = "redirect-to-https"

    # cache_policy_id = "658327ea-f89d-4fab-a63d-7e88639e58f6" # Managed-CachingOptimized (recommended for S3)
    cache_policy_id          = "4135ea2d-6df8-44a3-9df3-4b5a84be39ad" # Cache Disabled
    origin_request_policy_id = "88a5eaf4-2fd4-4709-b370-b4c650ea3fcf" # Managed-CORS-S3Origin
    compress                 = true
    lambda_function_association {
      event_type   = "origin-request"
      include_body = true
      lambda_arn   = "${var.lambda_function_arn}:${var.lambda_function_version}"
    }

    trusted_key_groups      = [aws_cloudfront_key_group.ahi_bhalite_files_keypair_group.id]
  }

  ordered_cache_behavior {
    path_pattern     = "/assets/*"
    target_origin_id = var.origin_id

    allowed_methods          = ["GET", "HEAD", "OPTIONS"]
    cached_methods           = ["GET", "HEAD"]
    viewer_protocol_policy   = "redirect-to-https"
    cache_policy_id          = "4135ea2d-6df8-44a3-9df3-4b5a84be39ad" # Cache Disabled
    origin_request_policy_id = "88a5eaf4-2fd4-4709-b370-b4c650ea3fcf" # Managed-CORS-S3Origin
    compress                 = true

    trusted_key_groups      = [aws_cloudfront_key_group.ahi_bhalite_files_keypair_group.id]
  }

  restrictions {
    geo_restriction {
      restriction_type = "none"
    }
  }

  aliases = var.aliases

  viewer_certificate {
    acm_certificate_arn      = var.acm_certificate_arn
    ssl_support_method       = "sni-only"
    minimum_protocol_version = "TLSv1.2_2018"
  }
}

resource "aws_cloudfront_origin_access_identity" "oai" {
  comment = "OAI for my S3 bucket"
}

resource "aws_wafv2_web_acl" "spa" {
  name  = "${var.origin_domain_name}-cloudfront-webacl"
  scope = "CLOUDFRONT"

  default_action {
    allow {}
  }

  rule {
    name     = "AWSManagedRulesCommonRuleSet"
    priority = 0

    override_action {
      count {}
    }

    statement {
      managed_rule_group_statement {
        name        = "AWSManagedRulesCommonRuleSet"
        vendor_name = "AWS"
      }
    }

    visibility_config {
      cloudwatch_metrics_enabled = true
      metric_name                = "AWSManagedRulesCommonRuleSet"
      sampled_requests_enabled   = true
    }
  }

  visibility_config {
    cloudwatch_metrics_enabled = true
    metric_name                = "cloudfrontVisibilityConfig"
    sampled_requests_enabled   = true
  }

  tags = {
    Environment = "Production"
    Name        = "cloudfrontWebACL"
  }
}

