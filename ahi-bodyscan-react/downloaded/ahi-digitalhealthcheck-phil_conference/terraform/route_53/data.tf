data "aws_acm_certificate" "cert" {
  domain      = "biometrichealth.tech"
  statuses    = ["ISSUED"]
  most_recent = true
}

data "aws_route53_zone" "zone" {
  zone_id = "Z01912363F5IAKTBY6LOP"
}
