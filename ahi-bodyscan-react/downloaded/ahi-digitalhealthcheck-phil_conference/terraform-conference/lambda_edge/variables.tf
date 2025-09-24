variable "function_name" {
  description = "Name of the Lambda function"
  type        = string
}

variable "handler" {
  description = "Lambda function handler"
  type        = string
}

variable "runtime" {
  description = "Runtime environment for the Lambda function"
  type        = string
}

variable "source_path" {
  description = "Path to the Lambda function source code"
  type        = string
}

variable "dist_path" {
  description = "Path to the Lambda function zip file"
  type        = string
}

variable "url" {
  description = "Base URL to root"
  type = string
}
