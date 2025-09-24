#!/bin/bash

# Quick script to apply nginx configuration
# Run with: sudo bash quick-apply-nginx.sh

echo "Applying nginx configuration for AHI BodyScan..."

if [ "$EUID" -ne 0 ]; then 
    echo "Please run as root (use sudo)"
    exit 1
fi

# Backup current config
cp /etc/nginx/nginx.conf /etc/nginx/nginx.conf.backup-$(date +%Y%m%d-%H%M%S)

# Apply new config
cp nginx.conf /etc/nginx/nginx.conf

# Test configuration
nginx -t

if [ $? -eq 0 ]; then
    echo "Configuration is valid. Reloading nginx..."
    systemctl reload nginx
    echo "✓ Done! Your app should now be available at http://192.168.10.117"
else
    echo "Configuration error! Restoring backup..."
    cp /etc/nginx/nginx.conf.backup-* /etc/nginx/nginx.conf
    exit 1
fi