#!/bin/bash

# AHI BodyScan React App Hosting Setup Script for Arch Linux
# Run with: sudo bash setup-hosting-arch.sh

echo "=== AHI BodyScan Hosting Setup for Arch Linux ==="
echo

# Check if running as root
if [ "$EUID" -ne 0 ]; then 
    echo "Please run as root (use sudo)"
    exit 1
fi

# 1. Apply nginx configuration
echo "1. Applying Nginx configuration..."
cp /etc/nginx/nginx.conf /etc/nginx/nginx.conf.backup-$(date +%Y%m%d-%H%M%S)
cp nginx.conf /etc/nginx/nginx.conf

# Test nginx configuration
nginx -t
if [ $? -eq 0 ]; then
    echo "✓ Nginx configuration is valid"
    systemctl reload nginx
    echo "✓ Nginx reloaded"
else
    echo "✗ Nginx configuration error!"
    echo "Restoring backup..."
    cp /etc/nginx/nginx.conf.backup-* /etc/nginx/nginx.conf
    exit 1
fi

echo
echo "=== Basic Setup Complete! ==="
echo
echo "Your AHI BodyScan app is now available at:"
echo "  http://192.168.10.117"
echo
echo "To continue with SSL and security setup, press Enter..."
read

# 2. Install certbot for SSL
echo
echo "2. Installing certbot for SSL certificates..."
pacman -S --noconfirm certbot certbot-nginx

# 3. Install firewall
echo
echo "3. Installing and configuring firewall..."
pacman -S --noconfirm ufw

# Configure firewall
ufw --force reset
ufw default deny incoming
ufw default allow outgoing
ufw allow ssh
ufw allow 80/tcp
ufw allow 443/tcp
ufw --force enable
echo "✓ Firewall configured"

# 4. Create self-signed certificate for local testing
echo
echo "4. Creating self-signed certificate for local testing..."
mkdir -p /etc/ssl/private
openssl req -x509 -nodes -days 365 -newkey rsa:2048 \
    -keyout /etc/ssl/private/ahi-bodyscan.key \
    -out /etc/ssl/certs/ahi-bodyscan.crt \
    -subj "/C=US/ST=State/L=City/O=AHI/CN=192.168.10.117"

# Create nginx config with SSL
cat > nginx-ssl.conf << 'EOF'
#user http;
worker_processes  1;

# Load all installed modules
include modules.d/*.conf;

events {
    worker_connections  1024;
}

http {
    include       mime.types;
    default_type  application/octet-stream;

    sendfile        on;
    keepalive_timeout  65;

    # Enable gzip compression globally
    gzip  on;
    gzip_vary on;
    gzip_min_length 1024;
    gzip_types text/plain text/css text/xml text/javascript application/javascript application/xml+rss application/json;

    # Rate limiting
    limit_req_zone $binary_remote_addr zone=one:10m rate=30r/s;

    # Redirect HTTP to HTTPS
    server {
        listen       80;
        listen       [::]:80;
        server_name  192.168.10.117 localhost;
        return 301 https://$server_name$request_uri;
    }

    # HTTPS server
    server {
        listen       443 ssl http2;
        listen       [::]:443 ssl http2;
        server_name  192.168.10.117 localhost;

        ssl_certificate     /etc/ssl/certs/ahi-bodyscan.crt;
        ssl_certificate_key /etc/ssl/private/ahi-bodyscan.key;

        # Strong SSL configuration
        ssl_protocols TLSv1.2 TLSv1.3;
        ssl_ciphers ECDHE-RSA-AES128-GCM-SHA256:ECDHE-ECDSA-AES128-GCM-SHA256:ECDHE-RSA-AES256-GCM-SHA384:ECDHE-ECDSA-AES256-GCM-SHA384;
        ssl_prefer_server_ciphers off;

        # HSTS
        add_header Strict-Transport-Security "max-age=63072000; includeSubDomains; preload" always;

        root   /home/calvin/Websites/AHI-new/ahi-bodyscan-react/build;
        index  index.html index.htm;

        # Security headers
        add_header X-Frame-Options "SAMEORIGIN" always;
        add_header X-Content-Type-Options "nosniff" always;
        add_header X-XSS-Protection "1; mode=block" always;
        add_header Referrer-Policy "no-referrer-when-downgrade" always;
        add_header Permissions-Policy "camera=self, microphone=(), geolocation=(), payment=()" always;
        add_header Content-Security-Policy "default-src 'self'; script-src 'self' 'unsafe-inline' 'unsafe-eval'; style-src 'self' 'unsafe-inline'; img-src 'self' data: blob:; font-src 'self' data:; connect-src 'self' wss: https:; media-src 'self' blob:;" always;

        # Cache static assets
        location ~* \.(js|css|png|jpg|jpeg|gif|ico|svg|woff|woff2|ttf|eot)$ {
            expires 1y;
            add_header Cache-Control "public, immutable";
        }

        # SPA fallback
        location / {
            try_files $uri $uri/ /index.html;
        }

        # Disable access to hidden files
        location ~ /\. {
            deny all;
        }

        # Rate limiting
        limit_req zone=one burst=20 nodelay;
    }
}
EOF

# 5. Install fail2ban
echo
echo "5. Installing fail2ban..."
pacman -S --noconfirm fail2ban

# Configure fail2ban for nginx
mkdir -p /etc/fail2ban
cat > /etc/fail2ban/jail.local << 'EOF'
[DEFAULT]
bantime = 3600
findtime = 600
maxretry = 5

[nginx-limit-req]
enabled = true
filter = nginx-limit-req
port = http,https
logpath = /var/log/nginx/error.log

[nginx-http-auth]
enabled = true
filter = nginx-http-auth
port = http,https
logpath = /var/log/nginx/error.log
EOF

systemctl enable fail2ban
systemctl start fail2ban

echo
echo "=== Setup Complete! ==="
echo
echo "Your AHI BodyScan app is now available at:"
echo "  HTTP:  http://192.168.10.117"
echo "  HTTPS: https://192.168.10.117 (self-signed cert warning will appear)"
echo
echo "To enable HTTPS:"
echo "  sudo cp nginx-ssl.conf /etc/nginx/nginx.conf"
echo "  sudo systemctl reload nginx"
echo
echo "For production with a domain name:"
echo "1. Get a domain (or use free dynamic DNS like DuckDNS)"
echo "2. Update server_name in nginx.conf"
echo "3. Run: sudo certbot --nginx -d yourdomain.com"