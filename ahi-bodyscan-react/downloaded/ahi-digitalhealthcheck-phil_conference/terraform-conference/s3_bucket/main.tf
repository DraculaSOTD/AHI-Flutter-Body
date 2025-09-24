provider "aws" {
  region = "us-east-1"
}
resource "aws_s3_bucket" "bucket" {
  bucket = var.bucket_name
}

resource "aws_cloudfront_origin_access_control" "oac" {
  name                              = "${var.domain_name}-s3-oac"
  description                       = ""
  origin_access_control_origin_type = "s3"
  signing_behavior                  = "always"
  signing_protocol                  = "sigv4"
}

resource "aws_s3_bucket_policy" "bucket_policy" {
  bucket = aws_s3_bucket.bucket.id
  policy = data.aws_iam_policy_document.s3_policy.json
}

locals {
  mime_types = jsondecode(file("${path.module}/data/mime.json"))
}

resource "aws_s3_object" "ahi_bhalite_files_object" {
  for_each = fileset("${var.source_path}/assets/", "**")
  bucket   = aws_s3_bucket.bucket.id
  key      = "assets/${each.value}"
  source   = "${var.source_path}/assets/${each.value}"
  etag     = filemd5("${var.source_path}/assets/${each.value}")
  content_type = lookup(local.mime_types, regex("\\.[^.]+$", each.value), null)
}

