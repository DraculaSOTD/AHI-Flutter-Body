# AHI BodyScan Setup Summary

## Current Status

✅ **Completed Steps:**
1. React app deployed to `/srv/http/ahi-bodyscan`
2. Nginx configured and running
3. Port forwarding verified (80 and 443 working)
4. DNS configured - `ahi.datapulseai.co` pointing to `102.182.135.156`
5. DNS propagation confirmed
6. Update scripts created

## Next Steps (Manual)

### 1. Apply Nginx Configuration
```bash
sudo cp /home/calvin/Websites/AHI-new/ahi-bodyscan-react/nginx-domain.conf /etc/nginx/nginx.conf
sudo nginx -t
sudo systemctl reload nginx
```

### 2. Obtain SSL Certificate
```bash
sudo certbot --nginx -d ahi.datapulseai.co
```

Follow the prompts to complete SSL setup.

## Access URLs

Once SSL is configured:
- **External**: https://ahi.datapulseai.co
- **Local**: https://192.168.10.117

## Maintenance Scripts

- **Update deployment**: `bash update-deployment.sh`
- **Deploy to web**: `sudo bash deploy-to-web.sh`
- **Fix permissions**: `sudo bash fix-permissions.sh`

## Important Files

- **Nginx config**: `/home/calvin/Websites/AHI-new/ahi-bodyscan-react/nginx-domain.conf`
- **Build files**: `/home/calvin/Websites/AHI-new/ahi-bodyscan-react/build/`
- **Web root**: `/srv/http/ahi-bodyscan/`

## Troubleshooting

If the site isn't accessible:
1. Check nginx status: `sudo systemctl status nginx`
2. Check nginx errors: `sudo journalctl -u nginx -n 50`
3. Verify DNS: `curl -I http://ahi.datapulseai.co`
4. Check firewall: `sudo iptables -L -n | grep -E "80|443"`

## Security Notes

- SSL certificate will auto-renew via certbot
- Security headers are configured in nginx
- Rate limiting is enabled
- HSTS is enabled for secure connections