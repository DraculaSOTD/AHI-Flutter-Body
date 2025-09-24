variable "domain_name" {
  description = "The domain name for the Route 53 zone"
  type        = string
}

variable "cloudfront_distribution_domain_name" {
  description = "The domain name of the CloudFront distribution"
  type        = string
}


variable "cloudfront_distribution_id" {
  description = "The hosted zone ID of the CloudFront distribution"
  type        = string
}
