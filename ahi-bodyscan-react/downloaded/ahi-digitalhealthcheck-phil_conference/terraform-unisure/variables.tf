variable "origin_bucket_domain_name" {
  description = "Origin bucket domain name"
  type        = string
  default     = "usbha"
}

variable "origin_tld" {
  description = "Origin bucket top-level domain"
  type        = string
  default     = ".ahi.zone"
}

variable "function_name" {
  description = "Lambda@Edge function name"
  type        = string
  default     = "ahi-web-bhalite-us-prod"
}

variable "local_path" {
  description = "The local path to sync to S3"
  type        = string
  default     = "../src/ahi-web-bhalite-prod"
}

variable "acm_cert_arn" {
  description = "The AWS Certificate for CloudFront ARN"
  type        = string
  default   = "arn:aws:acm:us-east-1:014498624062:certificate/70c20029-a54a-4a83-a680-39b5fc6ab81a"
}
