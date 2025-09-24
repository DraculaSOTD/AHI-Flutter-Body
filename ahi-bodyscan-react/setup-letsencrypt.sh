#!/bin/bash

# Let's Encrypt SSL Setup Script for AHI BodyScan
# This script configures SSL certificates for your DuckDNS domain

echo "=== Let's Encrypt SSL Setup for AHI BodyScan ==="
echo

# Check if running as root
if [ "$EUID" -ne 0 ]; then 
    echo "Please run as root (use sudo)"
    exit 1
fi

# Check if DuckDNS is configured
if [ ! -f /home/calvin/duckdns/config ]; then
    echo "Error: DuckDNS not configured. Please run setup-duckdns.sh first."
    exit 1
fi

# Load DuckDNS configuration
source /home/calvin/duckdns/config

echo "Domain: ${DOMAIN}"
echo

# Install certbot if not already installed
echo "1. Installing certbot..."
pacman -S --noconfirm certbot certbot-nginx python-certbot-nginx

# Create strong DH parameters
echo "2. Generating strong Diffie-Hellman parameters (this may take a few minutes)..."
if [ ! -f /etc/ssl/certs/dhparam.pem ]; then
    openssl dhparam -out /etc/ssl/certs/dhparam.pem 2048
fi

# Deploy files to web directory if not already done
echo "3. Ensuring web files are deployed..."
if [ ! -d /srv/http/ahi-bodyscan ]; then
    bash deploy-to-web.sh
fi

# Update nginx configuration with domain
echo "4. Updating nginx configuration..."
cp nginx-external.conf /tmp/nginx-external-configured.conf
sed -i "s/DOMAIN_PLACEHOLDER/${DOMAIN}/g" /tmp/nginx-external-configured.conf

# Temporarily use a basic config for initial cert generation
cat > /tmp/nginx-temp.conf << EOF
#user http;
worker_processes 1;
include modules.d/*.conf;

events {
    worker_connections 1024;
}

http {
    include mime.types;
    default_type application/octet-stream;
    sendfile on;
    keepalive_timeout 65;

    server {
        listen 80;
        server_name ${DOMAIN};
        root /srv/http/ahi-bodyscan;
        
        location /.well-known/acme-challenge/ {
            root /srv/http/ahi-bodyscan;
        }
        
        location / {
            try_files \$uri \$uri/ /index.html;
        }
    }
}
EOF

# Backup current nginx config
cp /etc/nginx/nginx.conf /etc/nginx/nginx.conf.backup-$(date +%Y%m%d-%H%M%S)

# Apply temporary config
cp /tmp/nginx-temp.conf /etc/nginx/nginx.conf
nginx -t && systemctl reload nginx

# Get SSL certificate
echo "5. Obtaining SSL certificate..."
echo
echo "IMPORTANT: Your router must be configured to forward ports 80 and 443 to this server!"
echo "Press Enter to continue when router is configured..."
read

# Get user email for Let's Encrypt
read -p "Enter your email address for Let's Encrypt notifications: " EMAIL

# Obtain certificate
certbot certonly --nginx \
    --non-interactive \
    --agree-tos \
    --email "${EMAIL}" \
    --domains "${DOMAIN}" \
    --redirect \
    --staple-ocsp

if [ $? -eq 0 ]; then
    echo "✓ SSL certificate obtained successfully!"
    
    # Apply full nginx configuration
    echo "6. Applying production nginx configuration..."
    cp /tmp/nginx-external-configured.conf /etc/nginx/nginx.conf
    
    # Test configuration
    nginx -t
    if [ $? -eq 0 ]; then
        systemctl reload nginx
        echo "✓ Nginx configured with SSL!"
    else
        echo "✗ Nginx configuration error. Restoring backup..."
        cp /etc/nginx/nginx.conf.backup-* /etc/nginx/nginx.conf
        systemctl reload nginx
        exit 1
    fi
    
    # Set up auto-renewal
    echo "7. Setting up automatic certificate renewal..."
    systemctl enable certbot-renew.timer
    systemctl start certbot-renew.timer
    
    # Create renewal hook
    mkdir -p /etc/letsencrypt/renewal-hooks/deploy
    cat > /etc/letsencrypt/renewal-hooks/deploy/nginx-reload.sh << 'HOOK'
#!/bin/bash
systemctl reload nginx
HOOK
    chmod +x /etc/letsencrypt/renewal-hooks/deploy/nginx-reload.sh
    
    echo
    echo "=== SSL Setup Complete! ==="
    echo
    echo "Your app is now available at:"
    echo "  https://${DOMAIN}"
    echo
    echo "SSL certificate will auto-renew before expiration."
    echo
    echo "To test renewal:"
    echo "  sudo certbot renew --dry-run"
    echo
    echo "To check certificate status:"
    echo "  sudo certbot certificates"
    
else
    echo "✗ Failed to obtain SSL certificate."
    echo
    echo "Common issues:"
    echo "1. Ports 80/443 not forwarded in router"
    echo "2. DuckDNS domain not pointing to your IP"
    echo "3. Firewall blocking incoming connections"
    echo
    echo "After fixing issues, run this script again."
    
    # Restore nginx config
    cp /etc/nginx/nginx.conf.backup-* /etc/nginx/nginx.conf
    systemctl reload nginx
    exit 1
fi