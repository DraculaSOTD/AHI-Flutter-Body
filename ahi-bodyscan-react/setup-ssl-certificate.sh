#!/bin/bash

echo "=== SSL Certificate Setup for ahi.datapulseai.co ==="
echo
echo "This script will help you obtain a Let's Encrypt SSL certificate"
echo "for your domain using certbot."
echo
echo "Prerequisites:"
echo "  - Domain DNS must be pointing to this server (✓ verified)"
echo "  - Ports 80 and 443 must be accessible (✓ verified)"
echo "  - You must run this script with sudo"
echo
echo "To proceed, run:"
echo "  sudo certbot --nginx -d ahi.datapulseai.co"
echo
echo "Follow the prompts to:"
echo "  1. Enter your email address"
echo "  2. Agree to terms of service"
echo "  3. Choose whether to share email with EFF"
echo "  4. Let certbot configure nginx automatically"
echo
echo "After completion, your site will be accessible at:"
echo "  https://ahi.datapulseai.co"
echo
echo "Certificate will auto-renew every 90 days."