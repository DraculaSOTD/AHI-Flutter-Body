# AHI BodyScan External Access Guide

This guide explains how to make your AHI BodyScan app accessible from anywhere on the internet using free DuckDNS and Let's Encrypt SSL.

## Quick Start

1. **Configure your router** for port forwarding (see [Router Guide](ROUTER_PORT_FORWARDING_GUIDE.md))
2. **Run the setup script**:
   ```bash
   sudo bash setup-external-access.sh
   ```
3. **Access your app** at `https://yourdomain.duckdns.org`

## Detailed Setup Process

### Prerequisites

- ✅ AHI BodyScan app built (`npm run build` completed)
- ✅ Nginx installed and running
- ✅ Root/sudo access
- ✅ Router admin access for port forwarding
- ✅ Ports 80 and 443 available

### Step 1: Router Configuration

**Critical**: Your router must forward external traffic to your server.

1. Access your router admin panel (usually `192.168.10.1`)
2. Forward these ports to `192.168.10.117`:
   - Port 80 (HTTP)
   - Port 443 (HTTPS)
3. Set a static IP for your server (recommended)

See [ROUTER_PORT_FORWARDING_GUIDE.md](ROUTER_PORT_FORWARDING_GUIDE.md) for detailed instructions.

### Step 2: DuckDNS Setup

DuckDNS provides free dynamic DNS service:

1. Create account at https://www.duckdns.org
2. Choose a subdomain (e.g., `myahi`)
3. Get your token from the DuckDNS dashboard
4. Run:
   ```bash
   sudo bash setup-duckdns.sh
   ```

### Step 3: SSL Certificate Setup

Let's Encrypt provides free SSL certificates:

```bash
sudo bash setup-letsencrypt.sh
```

This will:
- Install certbot
- Configure nginx for your domain
- Obtain SSL certificate
- Set up auto-renewal

### Step 4: Complete Setup

Run the main setup script:

```bash
sudo bash setup-external-access.sh
```

## Architecture Overview

```
Internet → Router → Nginx → React App
    ↓         ↓        ↓         ↓
DuckDNS   Port 80   SSL/TLS   /srv/http/
          Port 443  Certbot   ahi-bodyscan
```

## File Structure

```
/home/calvin/Websites/AHI-new/ahi-bodyscan-react/
├── setup-external-access.sh      # Main setup orchestrator
├── setup-duckdns.sh             # DuckDNS configuration
├── setup-letsencrypt.sh         # SSL certificate setup
├── nginx-external.conf          # Production nginx config
├── deploy-to-web.sh             # Deployment script
└── update-external-deployment.sh # Quick update script

/home/calvin/duckdns/
├── duck.sh                      # DuckDNS update script
├── config                       # DuckDNS configuration
└── duck.log                     # Update log

/srv/http/ahi-bodyscan/          # Deployed app files
```

## Security Features

### 1. **SSL/TLS Encryption**
- Let's Encrypt certificates
- Strong cipher suites
- TLS 1.2/1.3 only
- OCSP stapling

### 2. **HTTP Security Headers**
- Strict-Transport-Security (HSTS)
- X-Frame-Options
- X-Content-Type-Options
- Content-Security-Policy
- Referrer-Policy

### 3. **Rate Limiting**
- General: 10 requests/second
- API endpoints: 30 requests/second
- Connection limits per IP

### 4. **Fail2ban Protection**
- Automatic IP banning for suspicious activity
- Protection against brute force
- Bad bot blocking

### 5. **Nginx Hardening**
- Hidden version numbers
- Restricted file access
- Optimized buffer sizes

## Maintenance

### Daily Tasks
- Check access logs for unusual activity
- Monitor disk space

### Weekly Tasks
- Review fail2ban bans
- Check certificate expiry
- Update system packages

### Monthly Tasks
- Review and update security rules
- Test backup restoration
- Performance optimization

## Common Commands

### Check Status
```bash
# Service status
sudo systemctl status nginx
sudo systemctl status duckdns.timer

# SSL certificate
sudo certbot certificates

# Fail2ban status
sudo fail2ban-client status
sudo fail2ban-client status nginx-limit-req

# DuckDNS update
cat /home/calvin/duckdns/duck.log
```

### View Logs
```bash
# Access logs
sudo tail -f /var/log/nginx/ahi-bodyscan-access.log

# Error logs
sudo tail -f /var/log/nginx/ahi-bodyscan-error.log

# Monitor log
sudo tail -f /var/log/ahi-bodyscan-monitor.log
```

### Update App
```bash
# After making changes to React app
cd /home/calvin/Websites/AHI-new/ahi-bodyscan-react
npm run build
sudo bash update-external-deployment.sh
```

## Troubleshooting

### App Not Accessible Externally

1. **Check DuckDNS**:
   ```bash
   cat /home/calvin/duckdns/duck.log  # Should show "OK"
   nslookup yourdomain.duckdns.org    # Should return your IP
   ```

2. **Check port forwarding**:
   - Test from external network
   - Use online port checker
   - Verify router settings

3. **Check nginx**:
   ```bash
   sudo nginx -t                       # Test configuration
   sudo systemctl status nginx         # Check if running
   sudo ss -tln | grep -E ":80|:443" # Check listening ports
   ```

4. **Check firewall**:
   ```bash
   sudo ufw status                     # Should show ports allowed
   ```

### SSL Certificate Issues

1. **Certificate not renewing**:
   ```bash
   sudo certbot renew --dry-run        # Test renewal
   sudo certbot renew --force-renewal  # Force renewal
   ```

2. **Wrong domain**:
   - Check nginx server_name matches DuckDNS domain
   - Regenerate certificate with correct domain

### Performance Issues

1. **Slow loading**:
   - Check nginx compression settings
   - Verify static asset caching
   - Monitor server resources

2. **High CPU/Memory**:
   - Review nginx worker processes
   - Check for DDoS attacks in logs
   - Consider Cloudflare proxy

## Advanced Configuration

### Using Cloudflare (Optional)

For additional security and performance:

1. Sign up at cloudflare.com
2. Add your DuckDNS domain
3. Update DuckDNS to Cloudflare's IP
4. Enable proxy (orange cloud)
5. Configure SSL mode to "Full (strict)"

Benefits:
- DDoS protection
- Global CDN
- Additional security rules
- Analytics

### Custom Domain

To use your own domain instead of DuckDNS:

1. Update DNS A record to your external IP
2. Modify nginx server_name
3. Regenerate SSL certificate
4. Update monitoring scripts

## Backup and Recovery

### Backup Important Files
```bash
# Create backup directory
mkdir -p ~/ahi-backup

# Backup configurations
cp -r /home/calvin/duckdns ~/ahi-backup/
cp /etc/nginx/nginx.conf ~/ahi-backup/
cp -r /etc/letsencrypt ~/ahi-backup/

# Backup app
tar -czf ~/ahi-backup/ahi-app-$(date +%Y%m%d).tar.gz \
  /srv/http/ahi-bodyscan
```

### Restore from Backup
```bash
# Restore nginx config
sudo cp ~/ahi-backup/nginx.conf /etc/nginx/

# Restore certificates
sudo cp -r ~/ahi-backup/letsencrypt /etc/

# Restore app
sudo tar -xzf ~/ahi-backup/ahi-app-*.tar.gz -C /
```

## Support

For issues:
1. Check logs first
2. Review this documentation
3. Verify router and network settings
4. Test each component separately

Remember: Security is an ongoing process. Keep your system updated and monitor logs regularly!