#!/bin/bash

# Comprehensive Security & Monitoring Setup for AHI BodyScan
# This script sets up security hardening and monitoring tools

set -e

echo "=== AHI BodyScan Security & Monitoring Setup ==="
echo
echo "This script will set up:"
echo "1. Netdata for real-time monitoring"
echo "2. Enhanced nginx security configuration"
echo "3. Fail2ban for DDoS protection"
echo "4. SSL certificate preparation"
echo "5. Security headers and hardening"
echo
echo "Note: Run this script with sudo"
echo
echo "Press Enter to continue..."
read

# Colors for output
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m'

# Check if running as root
if [ "$EUID" -ne 0 ]; then 
    echo -e "${RED}Please run as root (use sudo)${NC}"
    exit 1
fi

echo -e "${YELLOW}=== Part 1: Enhanced Nginx Security Configuration ===${NC}"

# Create enhanced nginx configuration with security features
cat > /home/calvin/Websites/AHI-new/ahi-bodyscan-react/nginx-secure.conf << 'EOF'
# Enhanced Security Configuration for AHI BodyScan

user http;
worker_processes auto;
worker_cpu_affinity auto;

# Load modules
include modules.d/*.conf;

events {
    worker_connections 2048;
    use epoll;
    multi_accept on;
}

http {
    include mime.types;
    default_type application/octet-stream;

    # Hide nginx version
    server_tokens off;
    
    # Performance optimizations
    sendfile on;
    tcp_nopush on;
    tcp_nodelay on;
    keepalive_timeout 65;
    types_hash_max_size 2048;
    client_max_body_size 10m;
    
    # Buffer overflow protection
    client_body_buffer_size 1K;
    client_header_buffer_size 1k;
    large_client_header_buffers 2 1k;
    
    # Timeout settings for DDoS mitigation
    client_body_timeout 10;
    client_header_timeout 10;
    send_timeout 10;
    
    # Gzip compression
    gzip on;
    gzip_vary on;
    gzip_min_length 1024;
    gzip_proxied any;
    gzip_comp_level 6;
    gzip_types text/plain text/css text/xml text/javascript application/javascript application/json application/xml+rss application/x-font-ttf font/opentype image/svg+xml image/x-icon;
    
    # Rate limiting zones
    limit_req_zone $binary_remote_addr zone=general:10m rate=10r/s;
    limit_req_zone $binary_remote_addr zone=api:10m rate=30r/s;
    limit_req_zone $binary_remote_addr zone=static:10m rate=50r/s;
    limit_conn_zone $binary_remote_addr zone=addr:10m;
    
    # DDoS protection - limit request methods
    if ($request_method !~ ^(GET|HEAD|POST|PUT|DELETE|OPTIONS)$) {
        return 444;
    }
    
    # Security headers map
    map $sent_http_content_type $content_security_policy {
        ~*text/html "default-src 'self'; script-src 'self' 'unsafe-inline' 'unsafe-eval'; style-src 'self' 'unsafe-inline'; img-src 'self' data: blob: https:; font-src 'self' data:; connect-src 'self' wss: https:; media-src 'self' blob:; object-src 'none'; frame-ancestors 'self'; base-uri 'self'; form-action 'self'; upgrade-insecure-requests;";
    }
    
    # Bad bot blocking
    map $http_user_agent $bad_bot {
        default 0;
        ~*(?i)(semrush|ahrefs|majestic|dotbot|megaindex|serpstatbot|seekport|mj12bot|bl\.uk_lddc_bot|blexbot|ahrefsbot) 1;
    }
    
    # Log format with performance metrics
    log_format performance '$remote_addr - $remote_user [$time_local] '
                          '"$request" $status $body_bytes_sent '
                          '"$http_referer" "$http_user_agent" '
                          'rt=$request_time uct="$upstream_connect_time" '
                          'uht="$upstream_header_time" urt="$upstream_response_time"';
    
    # Default server block to catch all undefined requests
    server {
        listen 80 default_server;
        listen [::]:80 default_server;
        server_name _;
        return 444;
    }
    
    # AHI BodyScan React App
    server {
        listen 80;
        listen [::]:80;
        server_name ahi.datapulseai.co;
        
        # Redirect to HTTPS
        return 301 https://$server_name$request_uri;
    }
    
    # HTTPS server (will be fully configured when SSL certs are ready)
    server {
        listen 443 ssl http2;
        listen [::]:443 ssl http2;
        server_name ahi.datapulseai.co;
        
        # Temporary self-signed cert
        ssl_certificate /etc/ssl/certs/ahi-bodyscan.crt;
        ssl_certificate_key /etc/ssl/private/ahi-bodyscan.key;
        
        # Strong SSL configuration
        ssl_protocols TLSv1.2 TLSv1.3;
        ssl_ciphers ECDHE-ECDSA-AES128-GCM-SHA256:ECDHE-RSA-AES128-GCM-SHA256:ECDHE-ECDSA-AES256-GCM-SHA384:ECDHE-RSA-AES256-GCM-SHA384:ECDHE-ECDSA-CHACHA20-POLY1305:ECDHE-RSA-CHACHA20-POLY1305;
        ssl_prefer_server_ciphers off;
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
        add_header X-Frame-Options "SAMEORIGIN" always;
        add_header X-Content-Type-Options "nosniff" always;
        add_header X-XSS-Protection "1; mode=block" always;
        add_header Referrer-Policy "strict-origin-when-cross-origin" always;
        add_header Permissions-Policy "accelerometer=(), camera=(), geolocation=(), gyroscope=(), magnetometer=(), microphone=(), payment=(), usb=()" always;
        add_header Content-Security-Policy $content_security_policy always;
        
        # Block bad bots
        if ($bad_bot) {
            return 403;
        }
        
        # Root directory
        root /srv/http/ahi-bodyscan;
        index index.html;
        
        # Logging
        access_log /var/log/nginx/ahi-bodyscan-access.log performance;
        error_log /var/log/nginx/ahi-bodyscan-error.log warn;
        
        # Connection limits
        limit_conn addr 100;
        
        # Block access to sensitive files
        location ~ /\.(git|env|htaccess|htpasswd|ini|log|sh|sql|swp|bak)$ {
            deny all;
        }
        
        # Block access to hidden files
        location ~ /\. {
            deny all;
            access_log off;
            log_not_found off;
        }
        
        # Nginx status for monitoring
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
        
        # Static assets with aggressive caching
        location ~* \.(js|css|png|jpg|jpeg|gif|ico|svg|woff|woff2|ttf|eot)$ {
            expires 1y;
            add_header Cache-Control "public, immutable";
            limit_req zone=static burst=100 nodelay;
            access_log off;
        }
        
        # API endpoints with higher rate limit
        location /api/ {
            limit_req zone=api burst=50 nodelay;
            
            # CORS headers for API
            add_header Access-Control-Allow-Origin $http_origin always;
            add_header Access-Control-Allow-Methods "GET, POST, PUT, DELETE, OPTIONS" always;
            add_header Access-Control-Allow-Headers "Authorization, Content-Type" always;
            add_header Access-Control-Max-Age 86400 always;
            
            if ($request_method = OPTIONS) {
                return 204;
            }
            
            try_files $uri /index.html;
        }
        
        # General rate limiting
        location / {
            limit_req zone=general burst=20 nodelay;
            try_files $uri $uri/ /index.html;
        }
    }
    
    # Include additional configurations
    include /etc/nginx/conf.d/*.conf;
}
EOF

echo -e "${GREEN}✓ Enhanced nginx configuration created${NC}"

echo -e "${YELLOW}=== Part 2: Creating Security Monitoring Script ===${NC}"

# Create a monitoring script
cat > /home/calvin/Websites/AHI-new/ahi-bodyscan-react/monitor-security.sh << 'EOF'
#!/bin/bash

# Security monitoring script for AHI BodyScan

# Check for suspicious activity in logs
echo "=== Security Monitor Report ==="
echo "Date: $(date)"
echo

echo "1. Failed login attempts:"
grep "401\|403" /var/log/nginx/ahi-bodyscan-access.log | tail -20

echo
echo "2. Potential DDoS patterns:"
awk '{print $1}' /var/log/nginx/ahi-bodyscan-access.log | sort | uniq -c | sort -rn | head -10

echo
echo "3. 5xx errors:"
grep " 5[0-9][0-9] " /var/log/nginx/ahi-bodyscan-access.log | tail -10

echo
echo "4. Resource usage:"
echo "CPU: $(top -bn1 | grep "Cpu(s)" | awk '{print $2}')"
echo "Memory: $(free -h | grep Mem | awk '{print $3 "/" $2}')"
echo "Disk: $(df -h / | tail -1 | awk '{print $3 "/" $2 " (" $5 ")"}')"

echo
echo "5. Active connections:"
ss -tunap | grep -E ":80|:443" | wc -l
EOF

chmod +x /home/calvin/Websites/AHI-new/ahi-bodyscan-react/monitor-security.sh

echo -e "${GREEN}✓ Security monitoring script created${NC}"

echo -e "${YELLOW}=== Part 3: Creating Fail2ban Configuration ===${NC}"

# Create fail2ban directory if it doesn't exist
mkdir -p /etc/fail2ban/jail.d
mkdir -p /etc/fail2ban/filter.d

# Create fail2ban jail for nginx
cat > /etc/fail2ban/jail.d/nginx-ahi-bodyscan.conf << 'EOF'
[nginx-limit-req]
enabled = true
filter = nginx-limit-req
logpath = /var/log/nginx/ahi-bodyscan-error.log
maxretry = 5
findtime = 300
bantime = 7200
action = iptables-multiport[name=nginx-limit-req, port="80,443", protocol=tcp]

[nginx-404]
enabled = true
filter = nginx-404
logpath = /var/log/nginx/ahi-bodyscan-access.log
maxretry = 30
findtime = 300
bantime = 3600
action = iptables-multiport[name=nginx-404, port="80,443", protocol=tcp]

[nginx-noscript]
enabled = true
filter = nginx-noscript
logpath = /var/log/nginx/ahi-bodyscan-access.log
maxretry = 5
findtime = 300
bantime = 3600
action = iptables-multiport[name=nginx-noscript, port="80,443", protocol=tcp]

[nginx-badbots]
enabled = true
filter = nginx-badbots
logpath = /var/log/nginx/ahi-bodyscan-access.log
maxretry = 1
findtime = 300
bantime = 86400
action = iptables-multiport[name=nginx-badbots, port="80,443", protocol=tcp]
EOF

# Create custom filters
cat > /etc/fail2ban/filter.d/nginx-404.conf << 'EOF'
[Definition]
failregex = ^<HOST>.*"(GET|POST|HEAD).*" 404
ignoreregex =
EOF

cat > /etc/fail2ban/filter.d/nginx-noscript.conf << 'EOF'
[Definition]
failregex = ^<HOST>.*"(GET|POST|HEAD).*(\.php|\.asp|\.exe|\.pl|\.cgi|\.scgi).*" 
ignoreregex =
EOF

cat > /etc/fail2ban/filter.d/nginx-badbots.conf << 'EOF'
[Definition]
badbotscustom = semrush|ahrefs|majestic|mj12bot|rogerbot|dotbot
failregex = ^<HOST>.*"(GET|POST|HEAD).*HTTP.*(?:%(badbotscustom)s)
ignoreregex =
EOF

echo -e "${GREEN}✓ Fail2ban configuration created${NC}"

echo -e "${YELLOW}=== Part 4: Creating Automated Backup Script ===${NC}"

# Create backup script
cat > /home/calvin/Websites/AHI-new/ahi-bodyscan-react/backup-ahi-bodyscan.sh << 'EOF'
#!/bin/bash

# Backup script for AHI BodyScan

BACKUP_DIR="/home/calvin/ahi-backups"
DATE=$(date +%Y%m%d_%H%M%S)
BACKUP_NAME="ahi-bodyscan-backup-$DATE"

# Create backup directory
mkdir -p "$BACKUP_DIR"

# Create backup
echo "Creating backup: $BACKUP_NAME"

# Backup application files
tar -czf "$BACKUP_DIR/$BACKUP_NAME-app.tar.gz" \
    /srv/http/ahi-bodyscan \
    /home/calvin/Websites/AHI-new/ahi-bodyscan-react/build

# Backup configurations
tar -czf "$BACKUP_DIR/$BACKUP_NAME-config.tar.gz" \
    /etc/nginx/nginx.conf \
    /etc/fail2ban/jail.d/nginx-ahi-bodyscan.conf \
    /home/calvin/Websites/AHI-new/ahi-bodyscan-react/*.conf

# Encrypt backups
echo "Encrypting backups..."
openssl enc -aes-256-cbc -salt -in "$BACKUP_DIR/$BACKUP_NAME-app.tar.gz" \
    -out "$BACKUP_DIR/$BACKUP_NAME-app.tar.gz.enc" \
    -k "$(cat /dev/urandom | tr -dc 'a-zA-Z0-9' | fold -w 32 | head -n 1)"

# Remove unencrypted files
rm "$BACKUP_DIR/$BACKUP_NAME-app.tar.gz"

# Keep only last 7 days of backups
find "$BACKUP_DIR" -name "ahi-bodyscan-backup-*" -mtime +7 -delete

echo "Backup completed: $BACKUP_DIR/$BACKUP_NAME"
EOF

chmod +x /home/calvin/Websites/AHI-new/ahi-bodyscan-react/backup-ahi-bodyscan.sh

echo -e "${GREEN}✓ Backup script created${NC}"

echo -e "${YELLOW}=== Part 5: Creating Setup Summary ===${NC}"

# Create setup summary
cat > /home/calvin/Websites/AHI-new/ahi-bodyscan-react/SECURITY_MONITORING_SETUP.md << 'EOF'
# AHI BodyScan Security & Monitoring Setup

## Security Features Implemented

### 1. Nginx Hardening
- Server tokens hidden
- Request method filtering
- Buffer overflow protection
- Rate limiting (10r/s general, 30r/s API)
- Bad bot blocking
- Sensitive file access blocking
- Connection limits

### 2. SSL/TLS Security
- TLS 1.2/1.3 only
- Strong cipher suites
- OCSP stapling ready
- Session security

### 3. Security Headers
- Strict-Transport-Security (HSTS)
- X-Frame-Options
- X-Content-Type-Options
- X-XSS-Protection
- Content-Security-Policy
- Referrer-Policy
- Permissions-Policy

### 4. DDoS Protection
- Rate limiting by zone
- Connection limiting
- Timeout settings
- Fail2ban rules (when activated)

### 5. Monitoring Features
- Nginx status endpoint
- Performance logging
- Security monitoring script
- Resource usage tracking

## Next Steps

1. **Install Netdata**:
   ```bash
   sudo bash setup-netdata-monitoring.sh
   ```

2. **Configure SSL Certificates**:
   ```bash
   sudo bash setup-letsencrypt.sh
   ```

3. **Activate Fail2ban**:
   ```bash
   sudo systemctl restart fail2ban
   ```

4. **Apply New Nginx Config**:
   ```bash
   sudo cp nginx-secure.conf /etc/nginx/nginx.conf
   sudo nginx -t
   sudo systemctl reload nginx
   ```

5. **Set Up Automated Backups**:
   ```bash
   # Add to crontab
   0 2 * * * /home/calvin/Websites/AHI-new/ahi-bodyscan-react/backup-ahi-bodyscan.sh
   ```

## Monitoring Commands

- Check security status: `./monitor-security.sh`
- View nginx logs: `tail -f /var/log/nginx/ahi-bodyscan-*.log`
- Check fail2ban: `sudo fail2ban-client status`
- Monitor resources: Access Netdata at https://localhost:19999

## Security Best Practices

1. Regularly update system packages
2. Monitor logs for suspicious activity
3. Keep backups encrypted and off-site
4. Review and update security rules monthly
5. Test disaster recovery procedures
EOF

echo -e "${GREEN}=== Security & Monitoring Setup Script Created ===${NC}"
echo
echo "This script has created all necessary configuration files."
echo "To complete the setup, run:"
echo
echo "1. sudo bash setup-netdata-monitoring.sh"
echo "2. Apply the secure nginx configuration"
echo "3. Configure SSL certificates for your domain"
echo "4. Restart services to apply changes"
echo
echo "See SECURITY_MONITORING_SETUP.md for detailed instructions."