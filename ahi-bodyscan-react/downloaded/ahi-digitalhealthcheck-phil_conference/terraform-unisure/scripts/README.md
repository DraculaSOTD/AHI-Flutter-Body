# AHI BHA Lite | UniSure | Create Link

The following script provides example code on how to generate the secure [pre-signed URL](https://docs.aws.amazon.com/AmazonCloudFront/latest/DeveloperGuide/private-content-signed-urls.html) to the BHA Lite deployment for UniSure. This is intended as a quick and simple first step to integration of the deployment, without determining the more complex auth/OAuth/OIDC details, yet remain satisfactory for security needs.

## Preparation

1. [Python](https://www.python.org) version 3.8 or above.
2. Python package [cryptography](https://pypi.org/project/cryptography/).

## Use

Simply run `create_url.py` and the script will output the signed URL to the BHA Lite start page. The script has the relevant sign key and policy information embedded within, and can be replicated in your language of choice.
