variable "origin_bucket_domain_name" {
  description = "Origin bucket domain name"
  type        = string
  default     = "cfbha"
}

variable "origin_tld" {
  description = "Origin bucket top-level domain"
  type        = string
  default     = ".ahi.zone"
}

variable "function_name" {
  description = "Lambda@Edge function name"
  type        = string
  default     = "ahi-web-bhalite-cf-prod"
}

variable "local_path" {
  description = "The local path to sync to S3"
  type        = string
  default     = "../src/ahi-web-bhalite-prod"
}

variable "acm_cert_arn" {
  description = "The AWS Certificate for CloudFront ARN"
  type        = string
  default   = "arn:aws:acm:us-east-1:648519867622:certificate/be2de01a-95c2-4a9f-ae5b-1ff9a83fa678"
}
