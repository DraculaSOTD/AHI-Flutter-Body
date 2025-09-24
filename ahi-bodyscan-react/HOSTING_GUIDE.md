# AHI BodyScan React App - Hosting Guide

## Quick Start

The app has been built and is ready to host. To complete the setup:

1. **Run the setup script with sudo:**
   ```bash
   sudo bash setup-hosting.sh
   ```

2. **Access your app:**
   - Local network: http://192.168.10.117
   - After SSL setup: https://192.168.10.117

## Manual Setup Steps

If you prefer to set up manually:

### 1. Nginx Configuration
```bash
sudo cp nginx-config-ahi-bodyscan /etc/nginx/sites-available/ahi-bodyscan
sudo ln -sf /etc/nginx/sites-available/ahi-bodyscan /etc/nginx/sites-enabled/
sudo rm -f /etc/nginx/sites-enabled/default
sudo nginx -t
sudo systemctl reload nginx
```

### 2. SSL Certificate (Choose one)

#### Option A: Self-signed (for local testing)
```bash
sudo openssl req -x509 -nodes -days 365 -newkey rsa:2048 \
    -keyout /etc/ssl/private/ahi-bodyscan.key \
    -out /etc/ssl/certs/ahi-bodyscan.crt \
    -subj "/C=US/ST=State/L=City/O=AHI/CN=192.168.10.117"
```

#### Option B: Let's Encrypt (requires domain)
```bash
sudo pacman -S certbot certbot-nginx
sudo certbot --nginx -d yourdomain.com
```

### 3. Firewall Setup
```bash
sudo pacman -S ufw
sudo ufw --force enable
sudo ufw default deny incoming
sudo ufw default allow outgoing
sudo ufw allow ssh
sudo ufw allow 80/tcp
sudo ufw allow 443/tcp
```

### 4. Security Hardening
```bash
sudo pacman -S fail2ban
# Configure fail2ban (see setup script for config)
sudo systemctl enable fail2ban
sudo systemctl start fail2ban
```

## Domain Setup Options

### Option 1: Dynamic DNS (Free)
1. Sign up for a free service:
   - DuckDNS: https://www.duckdns.org
   - No-IP: https://www.noip.com
   
2. Configure your router to forward ports 80 and 443 to 192.168.10.117

3. Update nginx config with your domain

4. Get SSL certificate:
   ```bash
   sudo certbot --nginx -d yourdomain.duckdns.org
   ```

### Option 2: Local Network Only
- Access via IP: http://192.168.10.117
- Add to hosts file on client machines:
  ```
  192.168.10.117 ahi-bodyscan.local
  ```

## Security Features Implemented

- ✅ HTTPS with strong TLS configuration
- ✅ Security headers (HSTS, CSP, X-Frame-Options, etc.)
- ✅ Gzip compression
- ✅ Static asset caching
- ✅ Rate limiting
- ✅ Firewall (ufw)
- ✅ Intrusion prevention (fail2ban)
- ✅ Hidden file protection

## Monitoring

Check nginx access logs:
```bash
sudo tail -f /var/log/nginx/access.log
```

Check nginx error logs:
```bash
sudo tail -f /var/log/nginx/error.log
```

Check fail2ban status:
```bash
sudo fail2ban-client status
```

## Updating the App

When you make changes to the React app:

1. Build the new version:
   ```bash
   cd /home/calvin/Websites/AHI-new/ahi-bodyscan-react
   npm run build
   ```

2. The changes will be automatically served by nginx

## Troubleshooting

### App not loading?
- Check nginx status: `sudo systemctl status nginx`
- Check nginx logs: `sudo journalctl -u nginx`
- Verify build directory: `ls -la /home/calvin/Websites/AHI-new/ahi-bodyscan-react/build`

### SSL certificate issues?
- For self-signed: Accept the browser warning
- For Let's Encrypt: Ensure domain points to your IP

### Firewall blocking access?
- Check ufw status: `sudo ufw status`
- Temporarily disable: `sudo ufw disable` (re-enable after testing!)

## Support

For issues with:
- React app: Check the browser console for errors
- Nginx: Check error logs at `/var/log/nginx/error.log`
- SSL: Verify certificate paths in nginx config