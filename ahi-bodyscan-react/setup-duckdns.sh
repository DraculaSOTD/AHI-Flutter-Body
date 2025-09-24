#!/bin/bash

# DuckDNS Setup Script for AHI BodyScan
# This script configures DuckDNS for external access to your app

echo "=== DuckDNS Setup for AHI BodyScan ==="
echo
echo "This script will help you set up DuckDNS for external access."
echo "You'll need to create a free account at https://www.duckdns.org"
echo
echo "Press Enter to continue..."
read

# Check if running as root
if [ "$EUID" -ne 0 ]; then 
    echo "Please run as root (use sudo)"
    exit 1
fi

# Get user input
echo "=== DuckDNS Configuration ==="
echo
read -p "Enter your DuckDNS subdomain (e.g., 'myapp' for myapp.duckdns.org): " SUBDOMAIN
read -p "Enter your DuckDNS token (from https://www.duckdns.org): " TOKEN

# Validate inputs
if [ -z "$SUBDOMAIN" ] || [ -z "$TOKEN" ]; then
    echo "Error: Subdomain and token are required!"
    exit 1
fi

DOMAIN="${SUBDOMAIN}.duckdns.org"

# Create DuckDNS directory
echo
echo "1. Creating DuckDNS directory..."
mkdir -p /home/calvin/duckdns
chown calvin:calvin /home/calvin/duckdns

# Create update script
echo "2. Creating DuckDNS update script..."
cat > /home/calvin/duckdns/duck.sh << EOF
#!/bin/bash
# DuckDNS update script
echo url="https://www.duckdns.org/update?domains=${SUBDOMAIN}&token=${TOKEN}&ip=" | curl -k -o /home/calvin/duckdns/duck.log -K -
EOF

chmod 700 /home/calvin/duckdns/duck.sh
chown calvin:calvin /home/calvin/duckdns/duck.sh

# Test the update
echo "3. Testing DuckDNS update..."
sudo -u calvin /home/calvin/duckdns/duck.sh

if grep -q "OK" /home/calvin/duckdns/duck.log; then
    echo "✓ DuckDNS update successful!"
else
    echo "✗ DuckDNS update failed. Please check your subdomain and token."
    exit 1
fi

# Set up cron job for automatic updates
echo "4. Setting up automatic updates..."
(crontab -u calvin -l 2>/dev/null; echo "*/5 * * * * /home/calvin/duckdns/duck.sh >/dev/null 2>&1") | crontab -u calvin -

# Create systemd service for reliability
echo "5. Creating systemd service..."
cat > /etc/systemd/system/duckdns.service << EOF
[Unit]
Description=DuckDNS update service
After=network.target

[Service]
Type=oneshot
User=calvin
ExecStart=/home/calvin/duckdns/duck.sh

[Install]
WantedBy=multi-user.target
EOF

cat > /etc/systemd/system/duckdns.timer << EOF
[Unit]
Description=Run DuckDNS update every 5 minutes
Requires=duckdns.service

[Timer]
OnBootSec=5min
OnUnitActiveSec=5min

[Install]
WantedBy=timers.target
EOF

systemctl daemon-reload
systemctl enable duckdns.timer
systemctl start duckdns.timer

# Save configuration
echo "6. Saving configuration..."
cat > /home/calvin/duckdns/config << EOF
SUBDOMAIN=${SUBDOMAIN}
DOMAIN=${DOMAIN}
TOKEN=${TOKEN}
EOF
chmod 600 /home/calvin/duckdns/config
chown calvin:calvin /home/calvin/duckdns/config

echo
echo "=== DuckDNS Setup Complete! ==="
echo
echo "Your DuckDNS domain: ${DOMAIN}"
echo "Current IP will be updated every 5 minutes"
echo
echo "Next steps:"
echo "1. Configure your router to forward ports 80 and 443 to this server"
echo "2. Run setup-external-access.sh to configure nginx and SSL"
echo
echo "To check DuckDNS status:"
echo "  cat /home/calvin/duckdns/duck.log"
echo "  systemctl status duckdns.timer"