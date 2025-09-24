variable "lambda_function_arn" {
  description = "ARN of the Lambda function"
  type        = string
}

variable "lambda_function_version" {
  description = "Version of the Lambda function"
  type        = string
}

variable "origin_bucket_domain_name" {
  description = "Origin bucket domain name"
  type        = string
}

variable "origin_id" {
  description = "Origin ID"
  type        = string
}

variable "acm_certificate_arn" {
  description = "The ARN of the ACM certificate"
  type        = string
}

variable "origin_domain_name" {
  description = "Origin domain name"
  type        = string
}

variable "origin_access_control_id" {}

variable "aliases" {
  description = "List of aliases"
  type        = list(string)
  default     = []
}
