data "aws_acm_certificate" "cert" {
  domain      = "usbha.ahi.zone"
  statuses    = ["ISSUED"]
  most_recent = true
}

data "aws_route53_zone" "zone" {
  zone_id = "Z01091621G90DK4SJQ9XK"
}
