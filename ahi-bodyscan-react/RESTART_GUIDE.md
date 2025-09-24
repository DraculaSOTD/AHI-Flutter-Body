# Server Restart Guide

This guide explains what happens when your server restarts and how to ensure everything is working.

## 🚀 What Happens Automatically

When your server restarts, the following services start automatically:

1. **Nginx Web Server**
   - Starts within 10-15 seconds of boot
   - Serves your website immediately
   - No manual intervention needed

2. **Network Services**
   - Network interfaces come up automatically
   - Static IP configuration is preserved

## 🌐 Your Website After Restart

### Immediate Availability
- **External Access**: https://ahi.datapulseai.co
- **Local Access**: https://192.168.10.117
- **Status**: Should be accessible within 30 seconds of boot

### What's Preserved
- ✅ SSL certificates (valid until 2025-10-24)
- ✅ Website files in `/srv/http/ahi-bodyscan`
- ✅ Nginx configuration
- ✅ Port forwarding (unless router also restarted)

## 🔍 Quick Verification Steps

After restart, run these commands to verify everything is working:

```bash
# 1. Check if nginx is running
sudo systemctl status nginx

# 2. Test website locally
curl -I https://localhost

# 3. Test external access
curl -I https://ahi.datapulseai.co

# 4. Check for any errors
sudo journalctl -u nginx -n 20
```

## ⚠️ Manual Tasks Required

### 1. SSL Certificate Renewal Setup
```bash
# Run once to enable auto-renewal
sudo bash setup-certbot-renewal.sh
```

### 2. If External Access Fails
Check if router was also restarted:
- Verify port forwarding is still active
- Ports 80 and 443 → 192.168.10.117

## 🛠️ Troubleshooting Common Issues

### Website Not Loading
```bash
# Check if nginx is running
sudo systemctl status nginx

# If not running, start it
sudo systemctl start nginx

# Check for configuration errors
sudo nginx -t
```

### SSL Certificate Warnings
```bash
# Check certificate status
sudo certbot certificates

# Manually renew if needed
sudo certbot renew
```

### Port Already in Use
```bash
# Find what's using port 80 or 443
sudo lsof -i :80
sudo lsof -i :443

# Stop conflicting service
sudo systemctl stop [service-name]
```

## 📋 Complete Post-Restart Checklist

- [ ] Server has booted completely
- [ ] Can SSH into server
- [ ] Nginx is running: `sudo systemctl status nginx`
- [ ] Website loads locally: `curl -I https://localhost`
- [ ] Website loads externally: Visit https://ahi.datapulseai.co
- [ ] No errors in logs: `sudo tail -20 /var/log/nginx/error.log`
- [ ] SSL certificate valid: Check browser padlock icon

## 🔄 Restart Commands

### Graceful Restart (Recommended)
```bash
sudo reboot
```

### Service-Only Restart
```bash
# Just restart nginx without rebooting
sudo systemctl restart nginx
```

### Emergency Recovery
```bash
# If system won't boot normally
# Boot into recovery mode and run:
systemctl start nginx.service
systemctl enable nginx.service
```

## 📝 Important Notes

1. **First Boot After Setup**: Everything should work automatically
2. **Router Restart**: You may need to reconfigure port forwarding
3. **DNS Changes**: Take up to 48 hours to propagate
4. **SSL Renewal**: Happens automatically if timer is enabled
5. **Backup**: Keep backups of `/etc/nginx/nginx.conf` and `/srv/http/ahi-bodyscan`

## 🆘 Still Having Issues?

1. Check STARTUP_CHECKLIST.md for detailed verification steps
2. Review README.md for troubleshooting section
3. Check nginx error logs for specific error messages
4. Verify network connectivity and DNS resolution

Remember: The system is designed to be resilient. Most issues can be resolved by simply restarting the nginx service.