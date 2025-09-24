#!/usr/bin/env python3

import base64
import datetime
import json

from cryptography.hazmat.backends import default_backend
from cryptography.hazmat.primitives import hashes
from cryptography.hazmat.primitives import serialization
from cryptography.hazmat.primitives.asymmetric import padding

CF_SIGNED_URL_KEY_PAIR_ID = 'K9E7SBFTI2P05'
CF_SIGNED_URL_PRIVATE_KEY = b'-----BEGIN RSA PRIVATE KEY-----\nMIIEpAIBAAKCAQEAu2CbCiz9N/wK9qVfn5w6vzhBY6Y7PCjqDtPMliPnAffA3v6g\nerR5YSmTQRthtsjQ5uRAE+kqdmy2ddfTLELqH4Z+dhIvra867XT98HiT+TGsmhtC\nKqLOybU2upjn9nKA5DcXxC5yGvsGaSIMrJ6YvMsA8vLO4I2CbkXxi9cBTkA7NwZL\nqFE7ooxQIhJ5m92nbi1V+6ColsxOrF8cK5bJzrYhSvtrC7s99KF61QNCtYiyOCXu\nhWX8W5b1LLShB/hPStdiuQ5zb3ryeyKNQ3LPrjR/y5D+sOeymOh9PfFvSV3tCWMy\nQJb4Hgw3LdxIpx2ek4XXYh0WvUZT0QLOe5GDSwIDAQABAoIBADdcZJJOFoaK9iZH\nWAhNXWIP2en6PDgbEoRM+X5Gn0MKkzaPXpTCa7ZUVUiAMY+Sp34AZBo94qjYl9Qb\nby39mzSzRPvXhbS4OB3eWgTJa1bN4WQqt5LnRVd9Le1Vrfs3e3Nx/5ib7GSiVBON\n1tbNiO0GwYIGUS8xQB8alTIu1RjsCadOfV2SSlQxAqAGwHbBijuo9lDIScdCtIDH\nkRrzL0AESQ1UdszxKA4dI4V14s4tcGMrTNO3TnqwS/MVkOO++U2P3R9Ff2yrbIY/\n/KPAjPdxShX57Ob7vyGqJUiH3cJZ3zs1zktikVFGxK/iLF7GcmdTPeSc/gzSXa/s\nplCOSEkCgYEA6Q6Qw8RyjJoQ0e2aXIZ48b9W91aAzIG25XF49jD9AbZ4bt2tVVBT\nEgJs6Jlq2MSkdhhG/rkV67hUsQp6CnXvUhRhtF7wSF4poeKIrdTDLKnY3+i4MBO3\nlk0GBOS6nYvfD1d5tsGbXHg9/pkmheNa8Ac7N+SDBgMXkKpfvsWlFT8CgYEAzdLW\nYTDwrc9388DRvwI/y+sOj0aLxquPSUSug/lapfSGFOBVvPk3UdxSZx/F1oZkdXFv\nlrbTo4LKcrunHtYL1LxjHA8RFVfaqosinrf0cWy/jkkLuR4jTDsM79u0T2Cb6xLZ\nPX/zJoLIIGd0v9SNvgGo6bc77CI9iWkbr/y6UvUCgYBX0IsvmLfozqyDzVmaf3jW\n7lfgfpGtfTrjHWyMp0zN9mlOrOyA9vE2Gf/bbONbpkypyuAFXswps50F0ILHqkRw\n0wk1o4g6XHCndOo3RbH9BEqZo+CzDNriIUehVGYUsv1rlHRiLnVR1GTXp/FiBZbg\nVnkzyECuZT/d3LSS93oKlQKBgQDGvqcZ1HsjKqnmarNeYNqfmstFWFaSMYk7s6Fc\neISqqVJriC/iOTfMyRWc1JZhzwXkTfP1sXbtddbtMPxwDzn/YyXEq1vQozPX6GA2\nqOTWPa8+wpQaXdqxuneEiT80QtJsDJhPBWUSM8jxWVToA2QXLF0hyTiIjIulSyRc\nLIrynQKBgQCx1QGSIODwVcQ9cphAlR10ruyffAzUnaVW+uZQvo+/hoaQ7OhAMURi\n/aHEXZO9Vs6B3SlmhPPv4MqS4LdCTKv7nLyRskdFD5BLTpUdPr5o3dQ+tpm0qqKy\nl1KVVN3Y5oCSXdySQExFnfobh14QJRcJUE2hCSRdgb8PMh2GtKe2iQ==\n-----END RSA PRIVATE KEY-----\n'


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
    expire_epoch_time = 1756359734
    policy = make_cloudfront_policy(url, expire_epoch_time)
    signature = generate_cloudfront_signature(policy.encode('utf-8'), CF_SIGNED_URL_PRIVATE_KEY)

    signed_url = f"{url}?" \
                 f"Policy={url_base64_encode(policy.encode('utf-8'))}&" \
                 f"Signature={url_base64_encode(signature)}&" \
                 f"Key-Pair-Id={CF_SIGNED_URL_KEY_PAIR_ID}"
    
    print(expire_epoch_time)
    print(signed_url)

generate_cloudfront_signed_url('https://cfbha.ahi.zone/', 1200000)

