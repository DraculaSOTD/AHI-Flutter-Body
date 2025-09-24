#!/bin/bash

# Netdata Monitoring Setup Script for AHI BodyScan
# This script installs and configures Netdata for real-time monitoring

set -e

echo "=== Netdata Monitoring Setup for AHI BodyScan ==="
echo

# Check if running as root
if [ "$EUID" -ne 0 ]; then 
    echo "Please run as root (use sudo)"
    exit 1
fi

# Colors for output
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m'

echo -e "${YELLOW}1. Installing Netdata...${NC}"

# Install Netdata using the official installer
if ! command -v netdata &> /dev/null; then
    # Install dependencies
    pacman -S --noconfirm curl git gcc make autoconf automake pkg-config zlib libuv lz4 openssl judy libmnl libnetfilter_conntrack

    # Install Netdata
    bash <(curl -Ss https://get.netdata.cloud/kickstart.sh) --dont-wait --stable-channel --disable-telemetry
else
    echo -e "${GREEN}Netdata is already installed${NC}"
fi

echo -e "${YELLOW}2. Configuring Netdata for AHI BodyScan...${NC}"

# Create custom configuration directory
mkdir -p /etc/netdata/custom

# Configure Netdata main settings
cat > /etc/netdata/netdata.conf << 'EOF'
# Netdata Configuration for AHI BodyScan

[global]
    run as user = netdata
    web files owner = root
    web files group = netdata
    
    # Performance settings
    update every = 1
    memory mode = dbengine
    page cache size = 32
    dbengine multihost disk space = 256

[web]
    # Bind to localhost only for security
    bind to = 127.0.0.1 ::1
    
    # Enable SSL
    ssl key = /etc/netdata/ssl/key.pem
    ssl certificate = /etc/netdata/ssl/cert.pem
    
    # Security headers
    enable gzip compression = yes
    gzip compression strategy = default
    gzip compression level = 3

[plugins]
    # Enable useful plugins
    apps = yes
    charts.d = yes
    node.d = yes
    python.d = yes
    
[health]
    # Enable health monitoring
    enabled = yes
    default repeat warning = never
    default repeat critical = never
EOF

echo -e "${YELLOW}3. Creating Nginx monitoring configuration...${NC}"

# Configure nginx stub_status
cat > /etc/netdata/python.d/nginx.conf << 'EOF'
# Nginx monitoring configuration

local:
  name: 'local'
  url: 'http://localhost/nginx_status'
EOF

# Add nginx status endpoint to nginx config
cat > /home/calvin/Websites/AHI-new/ahi-bodyscan-react/nginx-status.conf << 'EOF'
# Nginx status endpoint for monitoring
location /nginx_status {
    stub_status on;
    access_log off;
    allow 127.0.0.1;
    allow ::1;
    deny all;
}
EOF

echo -e "${YELLOW}4. Creating custom dashboards for React app monitoring...${NC}"

# Create custom charts configuration
cat > /etc/netdata/custom/ahi-bodyscan.conf << 'EOF'
# Custom monitoring for AHI BodyScan

# Monitor React build directory
[chart.ahi_bodyscan_disk]
type = disk_space
family = ahi_bodyscan
title = AHI BodyScan Disk Usage
units = GB
dimensions = /srv/http/ahi-bodyscan

# Monitor nginx access logs for the app
[chart.ahi_bodyscan_requests]
type = web_log
family = ahi_bodyscan
title = AHI BodyScan Requests
path = /var/log/nginx/ahi-bodyscan-access.log
EOF

echo -e "${YELLOW}5. Setting up alerts...${NC}"

# Create health configuration for alerts
cat > /etc/netdata/health.d/ahi-bodyscan.conf << 'EOF'
# Health alerts for AHI BodyScan

# Alert on high CPU usage
alarm: ahi_bodyscan_cpu_usage
on: system.cpu
lookup: average -1m of user,system,nice,iowait,irq,softirq,steal,guest,guest_nice
units: %
every: 10s
warn: $this > 80
crit: $this > 90
info: CPU usage is high

# Alert on high memory usage
alarm: ahi_bodyscan_memory_usage
on: system.ram
lookup: average -1m of used
calc: $this * 100 / ($this + $available)
units: %
every: 10s
warn: $this > 80
crit: $this > 90
info: Memory usage is high

# Alert on disk space
alarm: ahi_bodyscan_disk_space
on: disk_space./
lookup: average -1m of avail
calc: $this * 100 / ($this + $used)
units: %
every: 1m
warn: $this < 20
crit: $this < 10
info: Disk space is low

# Alert on nginx errors
alarm: ahi_bodyscan_nginx_errors
on: web_log_nginx.response_codes
lookup: sum -1m of 5xx
units: requests
every: 10s
warn: $this > 10
crit: $this > 50
info: High number of nginx 5xx errors
EOF

echo -e "${YELLOW}6. Creating SSL certificates for Netdata...${NC}"

# Create SSL directory
mkdir -p /etc/netdata/ssl

# Generate self-signed certificate for Netdata (will be replaced with proper cert later)
openssl req -x509 -nodes -days 365 -newkey rsa:2048 \
    -keyout /etc/netdata/ssl/key.pem \
    -out /etc/netdata/ssl/cert.pem \
    -subj "/C=US/ST=State/L=City/O=AHI BodyScan/CN=localhost"

# Set proper permissions
chown -R netdata:netdata /etc/netdata/ssl
chmod 700 /etc/netdata/ssl
chmod 600 /etc/netdata/ssl/*

echo -e "${YELLOW}7. Creating systemd service override for security...${NC}"

# Create systemd override for additional security
mkdir -p /etc/systemd/system/netdata.service.d
cat > /etc/systemd/system/netdata.service.d/override.conf << 'EOF'
[Service]
# Security hardening
PrivateTmp=true
ProtectSystem=strict
ProtectHome=true
ReadWritePaths=/var/cache/netdata /var/lib/netdata /var/log/netdata
NoNewPrivileges=true
ProtectKernelTunables=true
ProtectKernelModules=true
ProtectControlGroups=true
LockPersonality=true
RestrictRealtime=true
RestrictSUIDSGID=true
MemoryDenyWriteExecute=true
EOF

echo -e "${YELLOW}8. Creating nginx reverse proxy configuration for Netdata...${NC}"

# Create nginx configuration for Netdata access
cat > /home/calvin/Websites/AHI-new/ahi-bodyscan-react/nginx-netdata.conf << 'EOF'
# Netdata monitoring dashboard - SECURE ACCESS ONLY
server {
    listen 19999 ssl http2;
    server_name localhost;
    
    # SSL configuration (using same certs as main site when available)
    ssl_certificate /etc/ssl/certs/ahi-bodyscan.crt;
    ssl_certificate_key /etc/ssl/private/ahi-bodyscan.key;
    
    # Strong SSL settings
    ssl_protocols TLSv1.2 TLSv1.3;
    ssl_ciphers ECDHE-ECDSA-AES128-GCM-SHA256:ECDHE-RSA-AES128-GCM-SHA256;
    ssl_prefer_server_ciphers off;
    
    # Basic authentication
    auth_basic "AHI BodyScan Monitoring";
    auth_basic_user_file /etc/nginx/.htpasswd-netdata;
    
    # IP whitelist - only allow local network and specific IPs
    allow 127.0.0.1;
    allow ::1;
    allow 192.168.10.0/24;  # Local network
    deny all;
    
    # Security headers
    add_header X-Frame-Options "SAMEORIGIN" always;
    add_header X-Content-Type-Options "nosniff" always;
    add_header X-XSS-Protection "1; mode=block" always;
    
    location / {
        proxy_pass http://127.0.0.1:19999;
        proxy_set_header Host $host;
        proxy_set_header X-Real-IP $remote_addr;
        proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
        proxy_set_header X-Forwarded-Proto $scheme;
        
        # WebSocket support for real-time updates
        proxy_http_version 1.1;
        proxy_set_header Upgrade $http_upgrade;
        proxy_set_header Connection "upgrade";
        
        # Increase timeouts for streaming
        proxy_connect_timeout 60s;
        proxy_send_timeout 60s;
        proxy_read_timeout 60s;
    }
}
EOF

echo -e "${YELLOW}9. Creating monitoring credentials...${NC}"

# Generate random password for monitoring access
MONITOR_PASS=$(openssl rand -base64 32)

# Create htpasswd file
htpasswd -bc /etc/nginx/.htpasswd-netdata admin "$MONITOR_PASS"

# Save credentials
cat > /home/calvin/Websites/AHI-new/ahi-bodyscan-react/monitoring-credentials.txt << EOF
=== AHI BodyScan Monitoring Credentials ===

Netdata Dashboard: https://localhost:19999
Username: admin
Password: $MONITOR_PASS

Keep these credentials secure!
EOF

chmod 600 /home/calvin/Websites/AHI-new/ahi-bodyscan-react/monitoring-credentials.txt
chown calvin:calvin /home/calvin/Websites/AHI-new/ahi-bodyscan-react/monitoring-credentials.txt

echo -e "${YELLOW}10. Starting and enabling Netdata...${NC}"

# Reload systemd and start services
systemctl daemon-reload
systemctl enable netdata
systemctl restart netdata

# Wait for Netdata to start
sleep 5

# Check if Netdata is running
if systemctl is-active --quiet netdata; then
    echo -e "${GREEN}✓ Netdata is running successfully!${NC}"
else
    echo -e "${RED}✗ Netdata failed to start. Check logs: journalctl -u netdata${NC}"
    exit 1
fi

echo
echo -e "${GREEN}=== Netdata Monitoring Setup Complete! ===${NC}"
echo
echo "Monitoring Features Enabled:"
echo "- Real-time CPU, Memory, Disk monitoring"
echo "- Network traffic analysis"
echo "- Nginx performance metrics"
echo "- Process monitoring"
echo "- Custom alerts for high resource usage"
echo "- Secure web dashboard with authentication"
echo
echo "Access the dashboard:"
echo "- URL: https://localhost:19999"
echo "- Credentials: See monitoring-credentials.txt"
echo
echo "Next steps:"
echo "1. Configure your firewall to allow port 19999 from trusted IPs only"
echo "2. Set up email alerts in /etc/netdata/health_alarm_notify.conf"
echo "3. Customize dashboards for your specific needs"
echo
echo "To view real-time metrics from command line:"
echo "  curl -s localhost:19999/api/v1/info | jq ."