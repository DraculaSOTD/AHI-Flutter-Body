#!/bin/bash

# Fix permissions by moving build to standard web directory
# Run with: sudo bash fix-permissions.sh

echo "=== Fixing AHI BodyScan Permissions Issue ==="
echo

# Check if running as root
if [ "$EUID" -ne 0 ]; then 
    echo "Please run as root (use sudo)"
    exit 1
fi

# Step 1: Deploy files to web directory
echo "Step 1: Deploying files to /srv/http/ahi-bodyscan..."
bash deploy-to-web.sh

# Step 2: Update nginx configuration
echo
echo "Step 2: Updating nginx configuration..."
cp nginx-ssl.conf /etc/nginx/nginx.conf

# Step 3: Test nginx configuration
echo
echo "Step 3: Testing nginx configuration..."
nginx -t

if [ $? -eq 0 ]; then
    echo "✓ Configuration is valid"
    
    # Step 4: Reload nginx
    echo
    echo "Step 4: Reloading nginx..."
    systemctl reload nginx
    
    echo
    echo "=== Success! ==="
    echo "Your app should now be accessible at https://192.168.10.117"
    echo "(You may see a certificate warning - click 'Advanced' and proceed)"
    echo
    echo "For future deployments after building, run:"
    echo "  sudo bash deploy-to-web.sh"
    echo "  sudo systemctl reload nginx"
else
    echo "✗ Configuration error!"
    exit 1
fi