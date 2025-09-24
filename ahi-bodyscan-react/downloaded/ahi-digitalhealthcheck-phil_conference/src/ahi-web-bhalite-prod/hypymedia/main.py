# hypymedia/main.py
from hypymedia.html_list import tags
from datetime import datetime, timedelta

from botocore.signers import CloudFrontSigner
from cryptography.hazmat.backends import default_backend
from cryptography.hazmat.primitives import hashes, serialization
from cryptography.hazmat.primitives.asymmetric import padding


CF_SIGNED_URL_KEY_PAIR_ID = 'K9E7SBFTI2P05'
CF_SIGNED_URL_PRIVATE_KEY = b'-----BEGIN RSA PRIVATE KEY-----\nMIIEpAIBAAKCAQEAu2CbCiz9N/wK9qVfn5w6vzhBY6Y7PCjqDtPMliPnAffA3v6g\nerR5YSmTQRthtsjQ5uRAE+kqdmy2ddfTLELqH4Z+dhIvra867XT98HiT+TGsmhtC\nKqLOybU2upjn9nKA5DcXxC5yGvsGaSIMrJ6YvMsA8vLO4I2CbkXxi9cBTkA7NwZL\nqFE7ooxQIhJ5m92nbi1V+6ColsxOrF8cK5bJzrYhSvtrC7s99KF61QNCtYiyOCXu\nhWX8W5b1LLShB/hPStdiuQ5zb3ryeyKNQ3LPrjR/y5D+sOeymOh9PfFvSV3tCWMy\nQJb4Hgw3LdxIpx2ek4XXYh0WvUZT0QLOe5GDSwIDAQABAoIBADdcZJJOFoaK9iZH\nWAhNXWIP2en6PDgbEoRM+X5Gn0MKkzaPXpTCa7ZUVUiAMY+Sp34AZBo94qjYl9Qb\nby39mzSzRPvXhbS4OB3eWgTJa1bN4WQqt5LnRVd9Le1Vrfs3e3Nx/5ib7GSiVBON\n1tbNiO0GwYIGUS8xQB8alTIu1RjsCadOfV2SSlQxAqAGwHbBijuo9lDIScdCtIDH\nkRrzL0AESQ1UdszxKA4dI4V14s4tcGMrTNO3TnqwS/MVkOO++U2P3R9Ff2yrbIY/\n/KPAjPdxShX57Ob7vyGqJUiH3cJZ3zs1zktikVFGxK/iLF7GcmdTPeSc/gzSXa/s\nplCOSEkCgYEA6Q6Qw8RyjJoQ0e2aXIZ48b9W91aAzIG25XF49jD9AbZ4bt2tVVBT\nEgJs6Jlq2MSkdhhG/rkV67hUsQp6CnXvUhRhtF7wSF4poeKIrdTDLKnY3+i4MBO3\nlk0GBOS6nYvfD1d5tsGbXHg9/pkmheNa8Ac7N+SDBgMXkKpfvsWlFT8CgYEAzdLW\nYTDwrc9388DRvwI/y+sOj0aLxquPSUSug/lapfSGFOBVvPk3UdxSZx/F1oZkdXFv\nlrbTo4LKcrunHtYL1LxjHA8RFVfaqosinrf0cWy/jkkLuR4jTDsM79u0T2Cb6xLZ\nPX/zJoLIIGd0v9SNvgGo6bc77CI9iWkbr/y6UvUCgYBX0IsvmLfozqyDzVmaf3jW\n7lfgfpGtfTrjHWyMp0zN9mlOrOyA9vE2Gf/bbONbpkypyuAFXswps50F0ILHqkRw\n0wk1o4g6XHCndOo3RbH9BEqZo+CzDNriIUehVGYUsv1rlHRiLnVR1GTXp/FiBZbg\nVnkzyECuZT/d3LSS93oKlQKBgQDGvqcZ1HsjKqnmarNeYNqfmstFWFaSMYk7s6Fc\neISqqVJriC/iOTfMyRWc1JZhzwXkTfP1sXbtddbtMPxwDzn/YyXEq1vQozPX6GA2\nqOTWPa8+wpQaXdqxuneEiT80QtJsDJhPBWUSM8jxWVToA2QXLF0hyTiIjIulSyRc\nLIrynQKBgQCx1QGSIODwVcQ9cphAlR10ruyffAzUnaVW+uZQvo+/hoaQ7OhAMURi\n/aHEXZO9Vs6B3SlmhPPv4MqS4LdCTKv7nLyRskdFD5BLTpUdPr5o3dQ+tpm0qqKy\nl1KVVN3Y5oCSXdySQExFnfobh14QJRcJUE2hCSRdgb8PMh2GtKe2iQ==\n-----END RSA PRIVATE KEY-----\n'
ROOT_URL = "https://cfbha.ahi.zone"


def rsa_signer(message):
    cloudfront_pk = serialization.load_pem_private_key(CF_SIGNED_URL_PRIVATE_KEY, password=None, backend=default_backend())
    return cloudfront_pk.sign(message, padding.PKCS1v15(), hashes.SHA1())

def generate_cloudfront_signed_url(url: str, expire_seconds: int):
    cf_signer = CloudFrontSigner(CF_SIGNED_URL_KEY_PAIR_ID, rsa_signer)
    expire_date = datetime.now() + timedelta(minutes=14)
    return cf_signer.generate_presigned_url(url, date_less_than=expire_date)


def element(tag_name, end_tag=True) -> str:
    """
    This function takes in a tag name and an optional end tag flag.
    It returns a function that takes in any number of arguments.
    The first argument can be a dictionary of attributes.
    All other arguments are reduced to string outputs 
    and treated as content for the HTML tag.
    """
    def attributes_to_string(attributes: dict) -> str:
        """
        This function takes in a dictionary of attributes and converts it to a string.
        """
        return (
            " ".join(
                f'{k}="{v}"' if v is not True else k
                for k, v in (attributes or {}).items()
            )
            if attributes
            else ""
        )

    def inner(*args):
        # If the first argument is a dictionary, 
        # it is treated as attributes for the HTML tag
        attributes = args[0] if args and isinstance(args[0], dict) else None
        # All other arguments are treated as content for the HTML tag
        content_args = args if not attributes else args[1:]
        # All content arguments are reduced to string outputs
        content = "".join(map(str, content_args))
        
        # Hack in the pre-signed URL
        if attributes is not None:
            if "a" == tag_name:
                attributes['href'] = generate_cloudfront_signed_url(attributes['href'], 60)
            if "form" == tag_name:
                attributes['action'] = generate_cloudfront_signed_url(attributes['action'], 60)
            if "script" == tag_name and "src" in attributes and "assets" in attributes['src']:
                attributes['src'] = generate_cloudfront_signed_url(ROOT_URL + attributes['src'], 60)
            if "link" == tag_name and "href" in attributes and "assets" in attributes['href']:
                attributes['href'] = generate_cloudfront_signed_url(ROOT_URL + attributes['href'], 60)
            if "img" == tag_name and "src" in attributes and "assets" in attributes['src']:
                attributes['src'] = generate_cloudfront_signed_url(ROOT_URL + attributes['src'], 60)


        # Convert the attributes dictionary to a string
        attributes_str = attributes_to_string(attributes)

        # If end_tag is True, include the end tag in the output
        end_tag_str = f"</{tag_name}>" if end_tag else ""

        # Return the final HTML tag string
        return f'<{tag_name}{" " + attributes_str if attributes_str else ""}>{content}{end_tag_str or ""}'  # noqa: E501

    return inner


for tag in tags:
    function_name = f"{tag[0]}"
    tag_name = tag[0][:-1]
    end_tag = tag[1]
    
    globals()[function_name] = element(tag_name, end_tag)
