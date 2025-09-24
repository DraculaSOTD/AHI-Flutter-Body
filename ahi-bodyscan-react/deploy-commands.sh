#!/bin/bash

echo "Deployment Commands - Run with sudo:"
echo ""
echo "1. Deploy the build:"
echo "   sudo ./deploy-to-web.sh"
echo ""
echo "2. Reload nginx to apply proxy configuration:"
echo "   sudo systemctl reload nginx"
echo ""
echo "After running these commands, the Lambda function should work in production."