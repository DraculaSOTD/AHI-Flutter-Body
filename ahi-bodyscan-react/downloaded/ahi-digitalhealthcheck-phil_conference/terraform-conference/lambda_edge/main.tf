provider "aws" {
  region = "us-east-1"
}

# resource "null_resource" "load_deps" {
#   provisioner "local-exec" {
#     command     = <<EOF
#     poetry pip install --upgrade pip
#     poetry export --without-hashes -f requirements.txt -o requirements.txt
#     poetry run pip install -r requirements.txt --target packages
#     # poetry run pip install pydantic-core --platform manylinux2014_x86_64 -t packages --only-binary=:all:
#     pip install --platform manylinux2014_x86_64 --target=packages --implementation cp --python-version 3.12 --only-binary=:all: --upgrade pydantic-core
#     EOF
#     working_dir = var.source_path
#   }

#   triggers = {
#     always_run = "${timestamp()}"
#   }
# }

# resource "null_resource" "create_zip" {
#   provisioner "local-exec" {
#     command = <<EOF
#     cd ${var.source_path}
#     zip -r lambda.zip lambda_function.py
#     cd packages
#     zip -r ../lambda.zip .
#     EOF
#   }

#   depends_on = [
#     null_resource.export_requirements
#   ]
# }

# data "archive_file" "lambda_zip" {
#   type        = "zip"
#   source_dir  = var.source_path
#   output_path = "${var.dist_path}/function.zip"
#   excludes    = ["**/.git/**", "**/node_modules/**", "**/__pycache__/**", "**/.idea/**", "**/test/**", "**/cloudfront_proxy.py", "**/assets/**"]

#   depends_on = [
#     null_resource.create_zip
#   ]
# }

data "archive_file" "zip_source" {
  type        = "zip"
  source_dir  = var.source_path
  output_path = "${var.dist_path}/function.zip"
  excludes    = ["**/.git/**", "**/node_modules/**", "**/__pycache__/**", "**/.idea/**", "**/test/**", "**/cloudfront_proxy.py", "**/assets/**", "**/libs/**"]
}

// AWS IAM role resource
resource "aws_iam_role" "lambda_role" {
  name               = "${var.function_name}_execution_role"
  assume_role_policy = data.aws_iam_policy_document.assume_role_policy.json

  inline_policy {
    name   = "lambda_policy"
    policy = data.aws_iam_policy_document.inline_policy.json
  }
}


resource "aws_lambda_function" "lambda_function" {
  filename         = "${var.dist_path}/function.zip"
  function_name    = var.function_name
  handler          = var.handler
  runtime          = var.runtime
  memory_size      = 512
  timeout          = 30
  role             = aws_iam_role.lambda_role.arn
  source_code_hash = filebase64sha256("${var.dist_path}/function.zip")
  publish          = true
}

resource "aws_lambda_permission" "allow_cloudfront" {
  statement_id  = "AllowExecutionFromCloudFront"
  action        = "lambda:InvokeFunction"
  function_name = aws_lambda_function.lambda_function.function_name
  principal     = "edgelambda.amazonaws.com"
  qualifier     = aws_lambda_function.lambda_function.version
}

