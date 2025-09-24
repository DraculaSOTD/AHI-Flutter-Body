data "aws_acm_certificate" "cert" {
  domain      = "cfbha.ahi.zone"
  statuses    = ["ISSUED"]
  most_recent = true
}

data "aws_route53_zone" "zone" {
  zone_id = "Z02598562PW1D2WAMERT2"
}
