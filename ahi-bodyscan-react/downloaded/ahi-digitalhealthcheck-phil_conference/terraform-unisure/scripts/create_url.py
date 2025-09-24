#!/usr/bin/env python3

import base64
import datetime
import json
import uuid

from cryptography.hazmat.backends import default_backend
from cryptography.hazmat.primitives import hashes
from cryptography.hazmat.primitives import serialization
from cryptography.hazmat.primitives.asymmetric import padding

CF_SIGNED_URL_KEY_PAIR_ID = 'KR1BLM3JR9E5O'
CF_SIGNED_URL_PRIVATE_KEY = b'-----BEGIN RSA PRIVATE KEY-----\nMIIEowIBAAKCAQEA1OI2gXWbqQYcT8r5c2XJmjLrduFHaoMiPzv/VdJGKUfe/sYA\nU7BBxt9KvZyseApjBfva8S9A0AOuGWpgLhm66+a/45RjFhuS1UpmRfO1x+DWw6OB\nqkrhF0h4N+ktIiIugfPvJrgnHNUUUbgN1y+3d7ah+J/a3M1ztX2zmVbk4mAqQSU1\nT5CYrMAwj/vhiEs0H5TG3VQVnq2rY5g0MlEieZGoLNmr3xbzfsR1qfid3dUEjewn\njSUxhfH061Wp3i4IR5g0OYMJczd2O4Paf4WnQz4kEG1STjZuoL//PHXxDzVBH2OO\nriXVUWiWYMILDGPp4QxjKvFeGuUlTfu5rtwduwIDAQABAoIBAAtWKJToUGvF2Yge\nRP5yMzQ09fkUshrn9NDtPwGw+u+D1ko2/Rl3z2jpxy8lwsF5vCA7c0NYwtjQ+Vp9\nIbYSxLN/TESwSD6LJVyaY6vh+JI2VPmCIYuSBT6aK1y+3BpvO47MELY9JEgSyMw5\nQvNspRaI8UMuMBeGEV1e3kuSJwou/+bTuCzNiRYoMOKjLiAsZAZ3gLY4g+jd3YZg\nV58cpklfIoXc1den3rafIpRbGVHZZWrdschNPNny7eL1vJBiMcr6MxpBsnEkKsB1\nuKZ66GzeX4kMJzEF/nSkFJbLL5FfpFpSqeLFFbqp26hl68VCC7glmVAON4E5ru8F\n0s8+3tECgYEA+Oolt3p/1otyTuoAQC7U2mJOt6QCjB2eqfWBjJCnv/a9oOu4cO/i\nMOwEVcEQgd1KQPsDDzfsjBszfyR1Gdq98ncRqIBX1xmhfXAiIVE71aDbnVROngqi\nezGps7oggAdMJBH0wNQjyRScglvvcWd1LWwQq2hhoUug+6oboEA5UTcCgYEA2vGB\ni5WQQMJK3vJpTz39mQ2OTGuMWzDT2tQ0b6FZLm3kUgAvxo+46gA0FIEQLwETi/QY\nI8ayLg8RzO18/CBIdkKpmjawgArCuYTIMEpXZWHsQKcEbSwqNwSvzgMoOtsTrY8x\nLDlxERjigB8dHSbtMopONKGWXxssF2Wd8p+oqZ0CgYEA7yRmICT89S78H30JyKA1\nt6qDXkOgfuVmLnvEfzvrLq7r+NL/XOHbOo1GjOzz4xCr6ko+4jwkc2cezqRELRbP\nL0feGzlTK0FguZcl80SMa+4jkMAS2Bv3hsxr8lR8LQTVzkHCGBC4YNQAme2h2h4/\noegVC9k5EzIJ9D+2UvvhFXkCgYBL/gUjyK6/+2/3QBZTPcX2yKC6X53gzOrLiloE\n4PDOT7Au0gfDGIFrHtT0xPM0DWCWdAuo5z9TA0H/afK2pBHikjLB5veOCa4arD67\nnCIHe1x/+DhuGCUy4XqPu8LL4txKomfD3NAkyglbxPDa/HbZ0u1gETHbIthD6q5O\n1yG4dQKBgEES8QO63XGfvBhmu8qVT2iKvkuwEIJPTX4x0809WaZWHMvsBRLdOofM\ngOWNYX5FSB3TQrgU9Y7Id2cNfGZjDIrK/r29PCMMo4zxZi49Rq0G1SLj3U8aXGxG\nIN01sE5rRQmhFNU2EMgQNq4/szULjZuvrCb9IFliUghekSHz/4r5\n-----END RSA PRIVATE KEY-----'


def generate_cloudfront_signature(message: bytes, private_key: bytes):
    private_key_signer = serialization.load_pem_private_key(
        private_key,
        password=None,
        backend=default_backend()
    )
    return private_key_signer.sign(message, padding.PKCS1v15(), hashes.SHA1())


def make_cloudfront_policy(resource: str, expire_epoch_time: int):
    policy = {
        'Statement': [{
            'Resource': resource,
            'Condition': {
                'DateLessThan': {
                    'AWS:EpochTime': expire_epoch_time
                }
            }
        }]
    }
    return json.dumps(policy).replace(" ", "")


def url_base64_encode(data: bytes):
    return base64.b64encode(data).replace(b'+', b'-').replace(b'=', b'_').replace(b'/', b'~').decode('utf-8')


def url_base64_decode(data: bytes):
    return base64.b64encode(data).replace(b'-', b'+').replace(b'_', b'=').replace(b'~', b'/').decode('utf-8')


def generate_cloudfront_signed_url(url: str, expire_seconds: int):
    expire_epoch_time = (datetime.datetime.now() + datetime.timedelta(seconds=expire_seconds)).timestamp()
    expire_epoch_time = int(expire_epoch_time)
    policy = make_cloudfront_policy(url, expire_epoch_time)
    signature = generate_cloudfront_signature(policy.encode('utf-8'), CF_SIGNED_URL_PRIVATE_KEY)

    signed_url = f"{url}?" \
                 f"Policy={url_base64_encode(policy.encode('utf-8'))}&" \
                 f"Signature={url_base64_encode(signature)}&" \
                 f"Key-Pair-Id={CF_SIGNED_URL_KEY_PAIR_ID}"

    print(signed_url)

generate_cloudfront_signed_url('https://usbha.ahi.zone/', 60)
print(".")
generate_cloudfront_signed_url(f"https://usbha.ahi.zone/questions/{uuid.uuid4()}", 60)

