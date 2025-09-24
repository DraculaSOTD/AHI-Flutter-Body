# System Restart Checklist

This document outlines what happens when your server restarts and what needs to be checked.

## 🟢 Automatic Services (Start on Boot)

### ✅ Nginx Web Server
- **Status**: Enabled (starts automatically)
- **Config**: `/etc/nginx/nginx.conf`
- **Verify**: `sudo systemctl status nginx`

### ✅ Website Access
- **URL**: https://ahi.datapulseai.co
- **Local**: https://192.168.10.117
- **Files**: `/srv/http/ahi-bodyscan`

## 🟡 Manual Setup Required

### ⚠️ SSL Certificate Renewal
Currently, certbot renewal is not automated. To set up:

```bash
# Enable automatic renewal
sudo systemctl enable certbot-renew.timer
sudo systemctl start certbot-renew.timer

# Or create a cron job
sudo crontab -e
# Add: 0 0,12 * * * certbot renew --quiet
```

### ⚠️ Custom Monitoring Scripts
If you have monitoring scripts, they need to be added to systemd or cron.

## 🔍 Post-Restart Verification

### 1. Check Core Services
```bash
# Check nginx is running
sudo systemctl status nginx

# Check if website is accessible
curl -I https://ahi.datapulseai.co

# Check SSL certificate
sudo certbot certificates
```

### 2. Test Website Functionality
1. Open https://ahi.datapulseai.co in browser
2. Test camera access for face scanning
3. Verify all pages load correctly

### 3. Check Logs for Errors
```bash
# Check nginx error log
sudo tail -20 /var/log/nginx/error.log

# Check system journal
sudo journalctl -xe
```

## 🚀 Quick Commands

### If Website is Down
```bash
# Restart nginx
sudo systemctl restart nginx

# Check configuration
sudo nginx -t

# View detailed logs
sudo journalctl -u nginx -n 100
```

### If SSL Certificate Issues
```bash
# Renew certificate manually
sudo certbot renew

# Force renewal
sudo certbot renew --force-renewal
```

## 📋 Complete Restart Procedure

1. **System boots up**
2. **Nginx starts automatically**
3. **Website is immediately available**
4. **You should check**:
   - [ ] Website loads in browser
   - [ ] HTTPS is working (green padlock)
   - [ ] No errors in nginx logs
   - [ ] SSL certificate is valid

## 🔧 Troubleshooting

### Port Forwarding Lost
If external access stops working after router restart:
1. Log into router admin (192.168.10.1)
2. Re-enable port forwarding:
   - Port 80 → 192.168.10.117:80
   - Port 443 → 192.168.10.117:443

### DNS Issues
If domain doesn't resolve:
1. Check Squarespace DNS settings
2. Verify A record: ahi → 102.182.135.156
3. Wait for DNS propagation (up to 48 hours)

### Service Won't Start
```bash
# Check what's blocking the port
sudo ss -tlnp | grep :80
sudo ss -tlnp | grep :443

# Kill conflicting process if needed
sudo kill -9 [PID]

# Restart nginx
sudo systemctl restart nginx
```

## 📝 Notes

- All critical services are configured to auto-start
- Website should be accessible within 30 seconds of boot
- No manual intervention required for basic operation
- SSL renewal should be automated (see setup above)