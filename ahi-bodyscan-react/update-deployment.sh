#!/bin/bash

# Update deployment after rebuilding the React app
# Run with: bash update-deployment.sh (will prompt for sudo when needed)

echo "=== Updating AHI BodyScan Deployment ==="
echo

# Check if build directory exists
if [ ! -d "build" ]; then
    echo "Error: No build directory found!"
    echo "Please run 'npm run build' first"
    exit 1
fi

# Run deployment with sudo
echo "Deploying new build to web directory..."
echo "(You may be prompted for your sudo password)"
echo

sudo bash deploy-to-web.sh

if [ $? -eq 0 ]; then
    echo
    echo "✓ Deployment updated successfully!"
    echo "Your changes are now live at:"
    echo "  - https://ahi.datapulseai.co (external access)"
    echo "  - https://192.168.10.117 (local access)"
    
    # Reload nginx to ensure changes take effect
    echo
    echo "Reloading nginx..."
    sudo systemctl reload nginx
else
    echo "✗ Deployment failed!"
    exit 1
fi