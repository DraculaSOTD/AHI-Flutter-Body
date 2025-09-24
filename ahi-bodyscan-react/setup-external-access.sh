#!/bin/bash

# Complete External Access Setup for AHI BodyScan
# This script orchestrates the entire setup process

set -e  # Exit on any error

echo "=== AHI BodyScan External Access Setup ==="
echo
echo "This script will set up:"
echo "  ✓ DuckDNS dynamic DNS"
echo "  ✓ Nginx with domain configuration"
echo "  ✓ Let's Encrypt SSL certificates"
echo "  ✓ Security hardening"
echo "  ✓ Monitoring and logging"
echo
echo "Prerequisites:"
echo "  - Root access (sudo)"
echo "  - Router configured for port forwarding (80, 443)"
echo "  - DuckDNS account (free at https://www.duckdns.org)"
echo
echo "Press Enter to continue or Ctrl+C to cancel..."
read

# Check if running as root
if [ "$EUID" -ne 0 ]; then 
    echo "Please run as root (use sudo)"
    exit 1
fi

# Function to check command success
check_status() {
    if [ $? -eq 0 ]; then
        echo "✓ $1"
    else
        echo "✗ $1 failed!"
        exit 1
    fi
}

# Function to display section headers
section() {
    echo
    echo "=== $1 ==="
    echo
}

# Check if app is built
if [ ! -d "/home/calvin/Websites/AHI-new/ahi-bodyscan-react/build" ]; then
    echo "Error: App not built. Please run 'npm run build' first."
    exit 1
fi

# Step 1: Deploy to web directory
section "Step 1: Deploying App to Web Directory"
bash deploy-to-web.sh
check_status "App deployment"

# Step 2: Set up DuckDNS
section "Step 2: Setting up DuckDNS"
if [ -f /home/calvin/duckdns/config ]; then
    echo "DuckDNS already configured. Loading existing configuration..."
    source /home/calvin/duckdns/config
    echo "Domain: ${DOMAIN}"
else
    bash setup-duckdns.sh
    check_status "DuckDNS setup"
    source /home/calvin/duckdns/config
fi

# Step 3: Verify external connectivity
section "Step 3: Verifying External Connectivity"
echo "Checking if your external IP can be reached..."
EXTERNAL_IP=$(curl -s ifconfig.me)
echo "Your external IP: ${EXTERNAL_IP}"
echo
echo "Please ensure your router is configured for port forwarding:"
echo "  - Port 80 → 192.168.10.117:80"
echo "  - Port 443 → 192.168.10.117:443"
echo
echo "Refer to ROUTER_PORT_FORWARDING_GUIDE.md for instructions."
echo "Press Enter when router is configured..."
read

# Test port 80 accessibility
echo "Testing port 80..."
timeout 5 nc -zv ${EXTERNAL_IP} 80 2>&1 | grep -q "succeeded" && echo "✓ Port 80 is accessible" || echo "⚠ Port 80 might not be accessible externally"

# Step 4: Install dependencies
section "Step 4: Installing Dependencies"
pacman -S --noconfirm certbot certbot-nginx python-certbot-nginx
check_status "Package installation"

# Step 5: Set up SSL with Let's Encrypt
section "Step 5: Setting up SSL Certificates"
bash setup-letsencrypt.sh
check_status "SSL setup"

# Step 6: Security hardening
section "Step 6: Security Hardening"

# Configure fail2ban if not already done
if ! systemctl is-active --quiet fail2ban; then
    echo "Configuring fail2ban..."
    pacman -S --noconfirm fail2ban
    
    # Create nginx jail
    cat > /etc/fail2ban/jail.d/nginx.conf << 'EOF'
[nginx-http-auth]
enabled = true
filter = nginx-http-auth
port = http,https
logpath = /var/log/nginx/error.log

[nginx-noscript]
enabled = true
port = http,https
filter = nginx-noscript
logpath = /var/log/nginx/access.log
maxretry = 6

[nginx-badbots]
enabled = true
port = http,https
filter = nginx-badbots
logpath = /var/log/nginx/access.log
maxretry = 2

[nginx-noproxy]
enabled = true
port = http,https
filter = nginx-noproxy
logpath = /var/log/nginx/access.log
maxretry = 2

[nginx-limit-req]
enabled = true
filter = nginx-limit-req
port = http,https
logpath = /var/log/nginx/error.log
findtime = 600
maxretry = 10
bantime = 600
EOF
    
    systemctl enable fail2ban
    systemctl start fail2ban
    check_status "Fail2ban configuration"
fi

# Step 7: Set up monitoring
section "Step 7: Setting up Monitoring"

# Create monitoring script
cat > /home/calvin/ahi-bodyscan-monitor.sh << 'EOF'
#!/bin/bash
# Simple monitoring script for AHI BodyScan

LOG_FILE="/var/log/ahi-bodyscan-monitor.log"
DOMAIN=$(cat /home/calvin/duckdns/config | grep DOMAIN | cut -d'=' -f2)

# Check HTTPS availability
if curl -sf https://${DOMAIN}/health > /dev/null 2>&1; then
    echo "[$(date)] ✓ HTTPS check passed" >> $LOG_FILE
else
    echo "[$(date)] ✗ HTTPS check failed" >> $LOG_FILE
    # Could add email notification here
fi

# Check certificate expiry
CERT_EXPIRY=$(echo | openssl s_client -servername ${DOMAIN} -connect ${DOMAIN}:443 2>/dev/null | openssl x509 -noout -enddate | cut -d'=' -f2)
EXPIRY_EPOCH=$(date -d "${CERT_EXPIRY}" +%s)
CURRENT_EPOCH=$(date +%s)
DAYS_LEFT=$(( ($EXPIRY_EPOCH - $CURRENT_EPOCH) / 86400 ))

if [ $DAYS_LEFT -lt 10 ]; then
    echo "[$(date)] ⚠ Certificate expires in ${DAYS_LEFT} days!" >> $LOG_FILE
fi

# Check disk space
DISK_USAGE=$(df -h /srv/http/ahi-bodyscan | awk 'NR==2 {print $5}' | sed 's/%//')
if [ $DISK_USAGE -gt 80 ]; then
    echo "[$(date)] ⚠ Disk usage is ${DISK_USAGE}%" >> $LOG_FILE
fi
EOF

chmod +x /home/calvin/ahi-bodyscan-monitor.sh

# Add to crontab
(crontab -l 2>/dev/null | grep -v "ahi-bodyscan-monitor"; echo "0 * * * * /home/calvin/ahi-bodyscan-monitor.sh") | crontab -

# Step 8: Create update script
section "Step 8: Creating Update Script"

cat > /home/calvin/Websites/AHI-new/ahi-bodyscan-react/update-external-deployment.sh << 'EOF'
#!/bin/bash
# Quick update script for AHI BodyScan after making changes

echo "=== Updating AHI BodyScan Deployment ==="

# Check if running as root
if [ "$EUID" -ne 0 ]; then 
    echo "Please run as root (use sudo)"
    exit 1
fi

# Build the app
echo "1. Building app..."
cd /home/calvin/Websites/AHI-new/ahi-bodyscan-react
sudo -u calvin npm run build

# Deploy to web directory
echo "2. Deploying to web directory..."
bash deploy-to-web.sh

# Reload nginx
echo "3. Reloading nginx..."
systemctl reload nginx

echo "✓ Update complete!"
EOF

chmod +x /home/calvin/Websites/AHI-new/ahi-bodyscan-react/update-external-deployment.sh

# Final summary
section "Setup Complete!"

source /home/calvin/duckdns/config

echo "Your AHI BodyScan app is now accessible at:"
echo
echo "  🌐 https://${DOMAIN}"
echo
echo "Important information:"
echo "  - SSL certificate: Auto-renews via Let's Encrypt"
echo "  - DuckDNS updates: Every 5 minutes"
echo "  - Logs: /var/log/nginx/ahi-bodyscan-*.log"
echo "  - Monitor: /var/log/ahi-bodyscan-monitor.log"
echo
echo "Useful commands:"
echo "  - Update app: sudo bash update-external-deployment.sh"
echo "  - Check SSL: sudo certbot certificates"
echo "  - View logs: sudo tail -f /var/log/nginx/ahi-bodyscan-access.log"
echo "  - Ban status: sudo fail2ban-client status"
echo
echo "Security notes:"
echo "  ✓ HTTPS enforced with HSTS"
echo "  ✓ Rate limiting enabled"
echo "  ✓ Fail2ban protecting against attacks"
echo "  ✓ Security headers configured"
echo
echo "Next steps:"
echo "1. Test access from external network"
echo "2. Set up regular backups"
echo "3. Consider Cloudflare for additional protection"
echo "4. Monitor logs regularly"