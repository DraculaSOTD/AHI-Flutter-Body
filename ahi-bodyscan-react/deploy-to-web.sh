#!/bin/bash

# Deploy AHI BodyScan build to web directory
# Run with: sudo bash deploy-to-web.sh

echo "=== Deploying AHI BodyScan to Web Directory ==="
echo

# Check if running as root
if [ "$EUID" -ne 0 ]; then 
    echo "Please run as root (use sudo)"
    exit 1
fi

# Create web directory
echo "1. Creating web directory..."
mkdir -p /srv/http/ahi-bodyscan

# Copy build files
echo "2. Copying build files..."
cp -r /home/calvin/Websites/AHI-new/ahi-bodyscan-react/build/* /srv/http/ahi-bodyscan/

# Set ownership
echo "3. Setting ownership..."
chown -R http:http /srv/http/ahi-bodyscan

# Set permissions
echo "4. Setting permissions..."
find /srv/http/ahi-bodyscan -type d -exec chmod 755 {} \;
find /srv/http/ahi-bodyscan -type f -exec chmod 644 {} \;

# Verify deployment
echo "5. Verifying deployment..."
ls -la /srv/http/ahi-bodyscan/

echo
echo "✓ Deployment complete!"
echo "Files are now in /srv/http/ahi-bodyscan"