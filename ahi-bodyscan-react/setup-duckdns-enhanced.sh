#!/bin/bash

# Enhanced DuckDNS Setup Script for AHI BodyScan
# This script configures DuckDNS with additional security measures

set -e

echo "=== Enhanced DuckDNS Setup for AHI BodyScan ==="
echo
echo "This will configure DuckDNS for ahi.datapulseai.co with security hardening"
echo

# Colors
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m'

# Check if running as root
if [ "$EUID" -ne 0 ]; then 
    echo -e "${RED}Please run as root (use sudo)${NC}"
    exit 1
fi

# Since ahi.datapulseai.co is your custom domain, we'll set up both DuckDNS and custom domain
echo -e "${YELLOW}=== Domain Configuration ===${NC}"
echo
echo "You have a custom domain: ahi.datapulseai.co"
echo "We'll set up:"
echo "1. DuckDNS as a backup domain"
echo "2. Security configurations for your main domain"
echo
read -p "Enter a DuckDNS subdomain for backup access (e.g., 'ahibackup'): " DUCKDNS_SUBDOMAIN
read -p "Enter your DuckDNS token (from https://www.duckdns.org): " DUCKDNS_TOKEN
read -p "What is your external IP address? (check at whatismyip.com): " EXTERNAL_IP

# Validate inputs
if [ -z "$DUCKDNS_SUBDOMAIN" ] || [ -z "$DUCKDNS_TOKEN" ] || [ -z "$EXTERNAL_IP" ]; then
    echo -e "${RED}Error: All fields are required!${NC}"
    exit 1
fi

DUCKDNS_DOMAIN="${DUCKDNS_SUBDOMAIN}.duckdns.org"

echo -e "${YELLOW}1. Creating DuckDNS directory and scripts...${NC}"

# Create DuckDNS directory
mkdir -p /home/calvin/duckdns
chown calvin:calvin /home/calvin/duckdns

# Create DuckDNS update script
cat > /home/calvin/duckdns/duck.sh << EOF
#!/bin/bash
# DuckDNS update script
echo url="https://www.duckdns.org/update?domains=${DUCKDNS_SUBDOMAIN}&token=${DUCKDNS_TOKEN}&ip=" | curl -k -o /home/calvin/duckdns/duck.log -K -
EOF

chmod 700 /home/calvin/duckdns/duck.sh
chown calvin:calvin /home/calvin/duckdns/duck.sh

# Test DuckDNS update
echo -e "${YELLOW}2. Testing DuckDNS update...${NC}"
sudo -u calvin /home/calvin/duckdns/duck.sh

if grep -q "OK" /home/calvin/duckdns/duck.log; then
    echo -e "${GREEN}✓ DuckDNS update successful!${NC}"
else
    echo -e "${RED}✗ DuckDNS update failed. Please check your subdomain and token.${NC}"
    exit 1
fi

# Set up cron job for automatic updates
echo -e "${YELLOW}3. Setting up automatic updates...${NC}"
(crontab -u calvin -l 2>/dev/null | grep -v duck.sh; echo "*/5 * * * * /home/calvin/duckdns/duck.sh >/dev/null 2>&1") | crontab -u calvin -

# Create systemd service
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
cat > /home/calvin/duckdns/config << EOF
DUCKDNS_SUBDOMAIN=${DUCKDNS_SUBDOMAIN}
DUCKDNS_DOMAIN=${DUCKDNS_DOMAIN}
DUCKDNS_TOKEN=${DUCKDNS_TOKEN}
MAIN_DOMAIN=ahi.datapulseai.co
EXTERNAL_IP=${EXTERNAL_IP}
EOF
chmod 600 /home/calvin/duckdns/config
chown calvin:calvin /home/calvin/duckdns/config

echo -e "${YELLOW}4. Creating enhanced nginx configuration for both domains...${NC}"

# Create nginx configuration that supports both domains
cat > /home/calvin/Websites/AHI-new/ahi-bodyscan-react/nginx-multi-domain.conf << 'NGINX_EOF'
# Enhanced Multi-Domain Configuration for AHI BodyScan

user http;
worker_processes auto;
worker_cpu_affinity auto;

include modules.d/*.conf;

events {
    worker_connections 2048;
    use epoll;
    multi_accept on;
}

http {
    include mime.types;
    default_type application/octet-stream;

    # Hide nginx version for security
    server_tokens off;
    
    # Performance and security settings
    sendfile on;
    tcp_nopush on;
    tcp_nodelay on;
    keepalive_timeout 65;
    types_hash_max_size 2048;
    client_max_body_size 10m;
    
    # Buffer settings to prevent overflow attacks
    client_body_buffer_size 1K;
    client_header_buffer_size 1k;
    large_client_header_buffers 2 1k;
    
    # Timeout settings
    client_body_timeout 10;
    client_header_timeout 10;
    send_timeout 10;
    
    # Gzip compression
    gzip on;
    gzip_vary on;
    gzip_min_length 1024;
    gzip_proxied any;
    gzip_comp_level 6;
    gzip_types text/plain text/css text/xml text/javascript application/javascript application/json application/xml+rss;
    
    # Rate limiting zones
    limit_req_zone $binary_remote_addr zone=general:10m rate=10r/s;
    limit_req_zone $binary_remote_addr zone=api:10m rate=30r/s;
    limit_conn_zone $binary_remote_addr zone=addr:10m;
    
    # IP anonymization for privacy
    map $remote_addr $remote_addr_anon {
        ~(?P<ip>\d+\.\d+\.\d+)\. $ip.0;
        ~(?P<ip>[^:]+:[^:]+): $ip::;
        default 0.0.0.0;
    }
    
    # Log format with anonymized IP
    log_format privacy '$remote_addr_anon - $remote_user [$time_local] '
                      '"$request" $status $body_bytes_sent '
                      '"$http_referer" "$http_user_agent"';
    
    # Default server - catch all
    server {
        listen 80 default_server;
        listen [::]:80 default_server;
        server_name _;
        return 444;
    }
    
    # HTTP redirect for all domains
    server {
        listen 80;
        listen [::]:80;
        server_name ahi.datapulseai.co DUCKDNS_DOMAIN_PLACEHOLDER;
        
        # Allow Let's Encrypt challenges
        location /.well-known/acme-challenge/ {
            root /srv/http/ahi-bodyscan;
        }
        
        # Redirect all other traffic to HTTPS
        location / {
            return 301 https://$server_name$request_uri;
        }
    }
    
    # HTTPS server for main domain
    server {
        listen 443 ssl http2;
        listen [::]:443 ssl http2;
        server_name ahi.datapulseai.co;
        
        # SSL certificates (to be configured by Let's Encrypt)
        ssl_certificate /etc/letsencrypt/live/ahi.datapulseai.co/fullchain.pem;
        ssl_certificate_key /etc/letsencrypt/live/ahi.datapulseai.co/privkey.pem;
        
        # Include common SSL and security settings
        include /home/calvin/Websites/AHI-new/ahi-bodyscan-react/nginx-ssl-common.conf;
        
        # Root directory
        root /srv/http/ahi-bodyscan;
        index index.html;
        
        # Logging with privacy
        access_log /var/log/nginx/ahi-bodyscan-access.log privacy;
        error_log /var/log/nginx/ahi-bodyscan-error.log warn;
        
        # Include common location blocks
        include /home/calvin/Websites/AHI-new/ahi-bodyscan-react/nginx-locations-common.conf;
    }
    
    # HTTPS server for DuckDNS backup domain
    server {
        listen 443 ssl http2;
        listen [::]:443 ssl http2;
        server_name DUCKDNS_DOMAIN_PLACEHOLDER;
        
        # SSL certificates (to be configured by Let's Encrypt)
        ssl_certificate /etc/letsencrypt/live/DUCKDNS_DOMAIN_PLACEHOLDER/fullchain.pem;
        ssl_certificate_key /etc/letsencrypt/live/DUCKDNS_DOMAIN_PLACEHOLDER/privkey.pem;
        
        # Include common SSL and security settings
        include /home/calvin/Websites/AHI-new/ahi-bodyscan-react/nginx-ssl-common.conf;
        
        # Root directory
        root /srv/http/ahi-bodyscan;
        index index.html;
        
        # Logging
        access_log /var/log/nginx/duckdns-access.log privacy;
        error_log /var/log/nginx/duckdns-error.log warn;
        
        # Include common location blocks
        include /home/calvin/Websites/AHI-new/ahi-bodyscan-react/nginx-locations-common.conf;
    }
}
NGINX_EOF

# Replace placeholder with actual DuckDNS domain
sed -i "s/DUCKDNS_DOMAIN_PLACEHOLDER/${DUCKDNS_DOMAIN}/g" /home/calvin/Websites/AHI-new/ahi-bodyscan-react/nginx-multi-domain.conf

# Create common SSL configuration
cat > /home/calvin/Websites/AHI-new/ahi-bodyscan-react/nginx-ssl-common.conf << 'SSL_EOF'
# Common SSL Configuration

# Strong SSL protocols
ssl_protocols TLSv1.2 TLSv1.3;
ssl_ciphers ECDHE-ECDSA-AES128-GCM-SHA256:ECDHE-RSA-AES128-GCM-SHA256:ECDHE-ECDSA-AES256-GCM-SHA384:ECDHE-RSA-AES256-GCM-SHA384:ECDHE-ECDSA-CHACHA20-POLY1305:ECDHE-RSA-CHACHA20-POLY1305;
ssl_prefer_server_ciphers off;

# SSL session settings
ssl_session_cache shared:SSL:10m;
ssl_session_timeout 10m;
ssl_session_tickets off;

# OCSP stapling
ssl_stapling on;
ssl_stapling_verify on;
resolver 8.8.8.8 8.8.4.4 valid=300s;
resolver_timeout 5s;

# Security headers
add_header Strict-Transport-Security "max-age=63072000; includeSubDomains; preload" always;
add_header X-Frame-Options "DENY" always;
add_header X-Content-Type-Options "nosniff" always;
add_header X-XSS-Protection "1; mode=block" always;
add_header Referrer-Policy "strict-origin-when-cross-origin" always;
add_header Permissions-Policy "accelerometer=(), camera=(), geolocation=(), gyroscope=(), magnetometer=(), microphone=(), payment=(), usb=()" always;
add_header Content-Security-Policy "default-src 'self'; script-src 'self' 'unsafe-inline' 'unsafe-eval'; style-src 'self' 'unsafe-inline'; img-src 'self' data: blob: https:; font-src 'self' data:; connect-src 'self' wss: https:; media-src 'self' blob:; object-src 'none'; frame-ancestors 'none'; base-uri 'self'; form-action 'self'; upgrade-insecure-requests;" always;
SSL_EOF

# Create common location blocks
cat > /home/calvin/Websites/AHI-new/ahi-bodyscan-react/nginx-locations-common.conf << 'LOC_EOF'
# Common Location Blocks

# Connection limits
limit_conn addr 100;

# Security: block access to sensitive files
location ~ /\.(git|env|htaccess|htpasswd|ini|log|sh|sql|swp|bak|svn|cvs)$ {
    deny all;
    return 404;
}

# Block access to hidden files
location ~ /\. {
    deny all;
    access_log off;
    log_not_found off;
}

# Nginx status endpoint (internal only)
location /nginx_status {
    stub_status on;
    access_log off;
    allow 127.0.0.1;
    allow ::1;
    deny all;
}

# Health check endpoint
location /health {
    access_log off;
    return 200 "OK\n";
    add_header Content-Type text/plain;
}

# Security.txt file (RFC 9116)
location /.well-known/security.txt {
    alias /srv/http/ahi-bodyscan/security.txt;
}

# Robots.txt with anti-doxxing measures
location = /robots.txt {
    return 200 "User-agent: *\nDisallow: /api/\nDisallow: /admin/\nDisallow: /.well-known/\nCrawl-delay: 10\n";
}

# Static assets with caching
location ~* \.(js|css|png|jpg|jpeg|gif|ico|svg|woff|woff2|ttf|eot)$ {
    expires 1y;
    add_header Cache-Control "public, immutable";
    limit_req zone=general burst=100 nodelay;
    access_log off;
}

# API endpoints
location /api/ {
    limit_req zone=api burst=50 nodelay;
    
    # CORS headers
    add_header Access-Control-Allow-Origin $http_origin always;
    add_header Access-Control-Allow-Methods "GET, POST, PUT, DELETE, OPTIONS" always;
    add_header Access-Control-Allow-Headers "Authorization, Content-Type" always;
    add_header Access-Control-Max-Age 86400 always;
    
    if ($request_method = OPTIONS) {
        return 204;
    }
    
    try_files $uri /index.html;
}

# Main app with rate limiting
location / {
    limit_req zone=general burst=20 nodelay;
    try_files $uri $uri/ /index.html;
}
LOC_EOF

echo -e "${YELLOW}5. Creating security.txt file...${NC}"

# Create security.txt for responsible disclosure
mkdir -p /srv/http/ahi-bodyscan
cat > /srv/http/ahi-bodyscan/security.txt << EOF
Contact: security@datapulseai.co
Expires: $(date -d '+1 year' '+%Y-%m-%d')T00:00:00.000Z
Encryption: https://ahi.datapulseai.co/pgp-key.txt
Acknowledgments: https://ahi.datapulseai.co/security-thanks.txt
Preferred-Languages: en
Canonical: https://ahi.datapulseai.co/.well-known/security.txt
Policy: https://ahi.datapulseai.co/security-policy.txt
EOF

echo -e "${YELLOW}6. Creating anti-doxxing measures...${NC}"

# Create IP whitelisting script
cat > /home/calvin/Websites/AHI-new/ahi-bodyscan-react/manage-ip-whitelist.sh << 'WHITELIST_EOF'
#!/bin/bash

# IP Whitelist Management for AHI BodyScan

ACTION=$1
IP=$2
WHITELIST_FILE="/etc/nginx/ip_whitelist.conf"

case $ACTION in
    add)
        if [ -z "$IP" ]; then
            echo "Usage: $0 add <IP>"
            exit 1
        fi
        echo "allow $IP;" >> $WHITELIST_FILE
        echo "Added $IP to whitelist"
        ;;
    remove)
        if [ -z "$IP" ]; then
            echo "Usage: $0 remove <IP>"
            exit 1
        fi
        sed -i "/allow $IP;/d" $WHITELIST_FILE
        echo "Removed $IP from whitelist"
        ;;
    list)
        echo "Current whitelist:"
        cat $WHITELIST_FILE
        ;;
    *)
        echo "Usage: $0 {add|remove|list} [IP]"
        exit 1
        ;;
esac

# Reload nginx if changes were made
if [ "$ACTION" != "list" ]; then
    nginx -t && systemctl reload nginx
fi
WHITELIST_EOF

chmod +x /home/calvin/Websites/AHI-new/ahi-bodyscan-react/manage-ip-whitelist.sh

# Create initial whitelist
touch /etc/nginx/ip_whitelist.conf
echo "# IP Whitelist for AHI BodyScan" > /etc/nginx/ip_whitelist.conf
echo "# Add trusted IPs here" >> /etc/nginx/ip_whitelist.conf

echo -e "${YELLOW}7. Setting up monitoring for security events...${NC}"

# Create security monitoring script
cat > /home/calvin/Websites/AHI-new/ahi-bodyscan-react/monitor-security-enhanced.sh << 'MONITOR_EOF'
#!/bin/bash

# Enhanced Security Monitoring for AHI BodyScan

LOG_DIR="/var/log/nginx"
ALERT_FILE="/var/log/ahi-security-alerts.log"

# Function to send alert (configure email/webhook as needed)
send_alert() {
    local message=$1
    echo "[$(date)] ALERT: $message" >> $ALERT_FILE
    # Add email/webhook notification here
}

# Monitor for suspicious patterns
echo "=== Security Monitor - $(date) ==="

# Check for potential SQL injection attempts
SQL_ATTEMPTS=$(grep -E "(union|select|insert|update|delete|drop|exec|script|javascript)" $LOG_DIR/ahi-bodyscan-access.log | wc -l)
if [ $SQL_ATTEMPTS -gt 10 ]; then
    send_alert "Potential SQL injection attempts detected: $SQL_ATTEMPTS"
fi

# Check for directory traversal attempts
TRAVERSAL_ATTEMPTS=$(grep -E "\.\./|\.\.%2F" $LOG_DIR/ahi-bodyscan-access.log | wc -l)
if [ $TRAVERSAL_ATTEMPTS -gt 5 ]; then
    send_alert "Directory traversal attempts detected: $TRAVERSAL_ATTEMPTS"
fi

# Check for repeated 404s (possible scanning)
TOP_404_IPS=$(awk '$9 == 404 {print $1}' $LOG_DIR/ahi-bodyscan-access.log | sort | uniq -c | sort -rn | head -5)
echo "Top IPs with 404 errors:"
echo "$TOP_404_IPS"

# Check for high request rates
HIGH_RATE_IPS=$(awk '{print $1}' $LOG_DIR/ahi-bodyscan-access.log | sort | uniq -c | sort -rn | awk '$1 > 1000 {print $2}')
if [ ! -z "$HIGH_RATE_IPS" ]; then
    send_alert "High request rate from IPs: $HIGH_RATE_IPS"
fi

# Monitor SSL certificate expiry
CERT_EXPIRY=$(echo | openssl s_client -servername ahi.datapulseai.co -connect ahi.datapulseai.co:443 2>/dev/null | openssl x509 -noout -dates 2>/dev/null | grep notAfter | cut -d= -f2)
if [ ! -z "$CERT_EXPIRY" ]; then
    DAYS_LEFT=$(( ($(date -d "$CERT_EXPIRY" +%s) - $(date +%s)) / 86400 ))
    if [ $DAYS_LEFT -lt 30 ]; then
        send_alert "SSL certificate expires in $DAYS_LEFT days"
    fi
fi

echo "=== End Security Report ==="
MONITOR_EOF

chmod +x /home/calvin/Websites/AHI-new/ahi-bodyscan-react/monitor-security-enhanced.sh

# Create cron job for security monitoring
(crontab -l 2>/dev/null | grep -v monitor-security-enhanced; echo "0 * * * * /home/calvin/Websites/AHI-new/ahi-bodyscan-react/monitor-security-enhanced.sh") | crontab -

echo
echo -e "${GREEN}=== Enhanced DuckDNS & Security Setup Complete! ===${NC}"
echo
echo "Configuration Summary:"
echo "- Main domain: ahi.datapulseai.co"
echo "- Backup domain: ${DUCKDNS_DOMAIN}"
echo "- External IP: ${EXTERNAL_IP}"
echo "- DuckDNS updates: Every 5 minutes"
echo
echo "Security Features Added:"
echo "✓ IP anonymization in logs"
echo "✓ Enhanced rate limiting"
echo "✓ Security.txt for responsible disclosure"
echo "✓ Anti-doxxing robots.txt"
echo "✓ Sensitive file blocking"
echo "✓ Security monitoring script"
echo "✓ IP whitelist management"
echo
echo "Next Steps:"
echo "1. Configure your router to forward ports 80 and 443 to this server"
echo "2. Update DNS for ahi.datapulseai.co to point to ${EXTERNAL_IP}"
echo "3. Run: sudo bash setup-letsencrypt.sh"
echo "4. Apply new nginx config: sudo cp nginx-multi-domain.conf /etc/nginx/nginx.conf"
echo "5. Test and reload: sudo nginx -t && sudo systemctl reload nginx"
echo
echo "Security Monitoring:"
echo "- Check alerts: tail -f /var/log/ahi-security-alerts.log"
echo "- Run manual check: ./monitor-security-enhanced.sh"
echo "- Manage IPs: ./manage-ip-whitelist.sh {add|remove|list} [IP]"