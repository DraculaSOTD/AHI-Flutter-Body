# AHI BodyScan Security & Monitoring Implementation Guide

## Overview

This guide provides comprehensive protection for your website ahi.datapulseai.co against hackers, doxxing, and other security threats while implementing resource monitoring.

## Current Security Status

### ✅ Completed
1. **Basic Monitoring Setup**
   - Created monitoring scripts and dashboard
   - System status monitoring script
   - Custom metrics API
   - HTML monitoring dashboard

2. **Security Configuration Files**
   - Enhanced nginx configurations
   - Fail2ban rules
   - Security monitoring scripts
   - Backup automation

### ⚠️ Pending Implementation
1. SSL certificates for ahi.datapulseai.co
2. DuckDNS backup domain configuration
3. Fail2ban activation
4. ModSecurity WAF installation
5. Full monitoring system deployment

## Immediate Actions Required

### 1. Install Basic Monitoring Tools
```bash
sudo pacman -S htop iotop nethogs vnstat
```

### 2. Run the Security Setup
```bash
cd /home/calvin/Websites/AHI-new/ahi-bodyscan-react
sudo bash setup-security-monitoring.sh
```

### 3. Configure DuckDNS Backup Domain
```bash
sudo bash setup-duckdns-enhanced.sh
```

## Security Measures Against Hacking & Doxxing

### 1. **Network Security**
- **DDoS Protection**: Rate limiting (10 req/sec general, 30 req/sec API)
- **Connection Limits**: Max 100 connections per IP
- **Timeout Settings**: 10 second timeouts to prevent slowloris attacks
- **IP Blocking**: Fail2ban for automated threat response

### 2. **Web Application Security**
- **Headers**: HSTS, CSP, X-Frame-Options, etc.
- **Input Validation**: Request method filtering
- **File Access**: Blocked access to sensitive files (.git, .env, etc.)
- **Bot Protection**: Bad bot user-agent blocking

### 3. **Privacy & Anti-Doxxing**
- **IP Anonymization**: Logs store anonymized IPs
- **Server Info Hidden**: No version disclosure
- **Robots.txt**: Restricts crawler access
- **Security.txt**: Responsible disclosure contact

### 4. **SSL/TLS Security**
- **Strong Protocols**: TLS 1.2/1.3 only
- **Modern Ciphers**: ECDHE with forward secrecy
- **OCSP Stapling**: Certificate validation
- **HSTS Preloading**: Force HTTPS

### 5. **Access Control**
- **IP Whitelisting**: For admin areas
- **Geographic Restrictions**: Can limit by country
- **Rate Limiting**: Different zones for different endpoints
- **Authentication**: Basic auth for monitoring dashboard

## Resource Monitoring Options

### Option 1: Command Line Tools (Immediate)
```bash
# CPU & Memory
htop

# Disk I/O
sudo iotop

# Network usage
sudo nethogs

# Quick system check
./monitoring/system-status.sh
```

### Option 2: Web Dashboard (Easy Setup)
```bash
# Start the metrics API
cd monitoring
python3 metrics-api.py &

# Open dashboard in browser
firefox monitoring/dashboard.html
```

### Option 3: Glances (Recommended)
```bash
# Install
sudo pacman -S glances python-bottle

# Run web interface
glances -w --bind 127.0.0.1 --port 61208

# Access at http://localhost:61208
```

### Option 4: Netdata (Professional)
```bash
# One-line installer
bash <(curl -Ss https://get.netdata.cloud/kickstart.sh) --stable-channel --disable-telemetry

# Access at https://localhost:19999
```

## Security Monitoring Commands

### Check Security Status
```bash
# Run security monitor
./monitor-security-enhanced.sh

# View security alerts
tail -f /var/log/ahi-security-alerts.log

# Check nginx access patterns
./monitor-security.sh
```

### Manage IP Access
```bash
# Whitelist trusted IP
./manage-ip-whitelist.sh add 192.168.1.100

# Remove IP
./manage-ip-whitelist.sh remove 192.168.1.100

# List whitelisted IPs
./manage-ip-whitelist.sh list
```

### Monitor Failed Attempts
```bash
# Check 403/404 errors
grep -E "403|404" /var/log/nginx/ahi-bodyscan-access.log | tail -20

# Top attacking IPs
awk '{print $1}' /var/log/nginx/ahi-bodyscan-access.log | sort | uniq -c | sort -rn | head -10
```

## Automated Security Features

### 1. **Fail2ban Rules** (When Activated)
- Blocks IPs after 5 rate limit violations
- Bans scanners after 30 404s in 5 minutes
- Blocks bad bots on first detection
- Auto-unban after configured time

### 2. **Security Monitoring Cron**
- Hourly security checks
- SQL injection detection
- Directory traversal detection
- High request rate alerts
- SSL expiry warnings

### 3. **Backup Automation**
```bash
# Set up daily encrypted backups
crontab -e
# Add: 0 2 * * * /home/calvin/Websites/AHI-new/ahi-bodyscan-react/backup-ahi-bodyscan.sh
```

## Complete Setup Checklist

- [ ] Install monitoring tools (htop, iotop, etc.)
- [ ] Configure DuckDNS backup domain
- [ ] Set up SSL certificates
- [ ] Apply enhanced nginx configuration
- [ ] Activate fail2ban
- [ ] Configure firewall rules
- [ ] Set up automated backups
- [ ] Test monitoring dashboard
- [ ] Configure alert notifications
- [ ] Document emergency procedures

## Emergency Response

### If Under Attack
1. **Enable emergency rate limiting**:
   ```nginx
   limit_req_zone $binary_remote_addr zone=emergency:10m rate=1r/s;
   ```

2. **Block attacking IPs**:
   ```bash
   sudo iptables -A INPUT -s ATTACKER_IP -j DROP
   ```

3. **Enable Cloudflare (if configured)**:
   - Set security level to "I'm Under Attack"
   - Enable rate limiting rules

### Recovery Procedures
1. Review logs to identify attack patterns
2. Update security rules based on attack
3. Restore from encrypted backups if needed
4. Report serious incidents to authorities

## Maintenance Schedule

### Daily
- Check monitoring dashboard
- Review security alerts

### Weekly
- Analyze traffic patterns
- Update IP blacklists
- Check SSL certificate status

### Monthly
- Review and update security rules
- Test backup restoration
- Update system packages
- Security audit

## Support Resources

- **Nginx Docs**: https://nginx.org/en/docs/
- **Fail2ban Wiki**: https://github.com/fail2ban/fail2ban/wiki
- **OWASP**: https://owasp.org/
- **Let's Encrypt**: https://letsencrypt.org/docs/

Remember: Security is a continuous process. Stay vigilant and keep your systems updated!