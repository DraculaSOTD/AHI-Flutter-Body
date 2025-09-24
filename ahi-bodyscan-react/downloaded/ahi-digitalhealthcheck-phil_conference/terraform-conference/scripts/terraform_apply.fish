#!/usr/bin/env fish

set -x AWS_ACCESS_KEY_ID "AKIAZN7WL3TTPUV26BUG"
set -x AWS_SECRET_ACCESS_KEY "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"

cd ..

terraform init
terraform apply
