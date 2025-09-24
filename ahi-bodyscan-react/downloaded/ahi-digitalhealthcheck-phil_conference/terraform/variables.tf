variable "origin_bucket_domain_name" {
  description = "Origin bucket domain name"
  type        = string
  default     = "biometrichealth"
}

variable "origin_tld" {
  description = "Origin bucket top-level domain"
  type        = string
  default     = ".tech"
}

variable "function_name" {
  description = "Lambda@Edge function name"
  type        = string
  default     = "ahi-web-bhalite-prod"
}

variable "local_path" {
  description = "The local path to sync to S3"
  type        = string
  default     = "../src/ahi-web-bhalite-prod"
}
