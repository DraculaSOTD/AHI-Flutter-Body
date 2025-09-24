// AWS IAM policy document for assume role policy
data "aws_iam_policy_document" "assume_role_policy" {
  statement {
    actions = ["sts:AssumeRole"]
    effect  = "Allow"

    principals {
      type        = "Service"
      identifiers = ["lambda.amazonaws.com", "edgelambda.amazonaws.com"]
    }
  }
}

// AWS IAM policy document for inline policy
data "aws_iam_policy_document" "inline_policy" {
  statement {
    actions   = ["logs:*", "dynamodb:GetItem", "dynamodb:PutItem", "dynamodb:UpdateItem"]
    effect    = "Allow"
    resources = ["*"]
  }
}
