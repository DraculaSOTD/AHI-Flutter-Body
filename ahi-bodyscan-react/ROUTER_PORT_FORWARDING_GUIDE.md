# Router Port Forwarding Guide

This guide will help you configure your router to allow external access to your AHI BodyScan app.

## Prerequisites

- Your server's local IP: **192.168.10.117**
- Ports to forward: **80** (HTTP) and **443** (HTTPS)
- Router admin access (username/password)

## Step 1: Access Your Router

1. Open a web browser
2. Navigate to your router's IP address:
   - Common addresses: `192.168.10.1`, `192.168.1.1`, `192.168.0.1`
   - To find your router IP: `ip route | grep default`
3. Log in with your router credentials

## Step 2: Find Port Forwarding Section

Look for one of these menu options:
- **Port Forwarding**
- **Virtual Server**
- **NAT** → **Port Forwarding**
- **Advanced** → **Port Forwarding**
- **Applications & Gaming** → **Port Forwarding**

## Step 3: Configure Port Forwarding Rules

### Rule 1: HTTP (Port 80)
- **Service Name**: AHI-BodyScan-HTTP
- **Protocol**: TCP
- **External Port**: 80
- **Internal IP**: 192.168.10.117
- **Internal Port**: 80
- **Enable**: ✓ Yes

### Rule 2: HTTPS (Port 443)
- **Service Name**: AHI-BodyScan-HTTPS
- **Protocol**: TCP
- **External Port**: 443
- **Internal IP**: 192.168.10.117
- **Internal Port**: 443
- **Enable**: ✓ Yes

## Step 4: Set Static IP (Recommended)

To ensure your server always has the same IP:

1. Find **DHCP Settings** or **LAN Setup**
2. Look for **DHCP Reservation** or **Static DHCP**
3. Add entry:
   - **MAC Address**: Find with `ip addr show | grep ether`
   - **IP Address**: 192.168.10.117
   - **Device Name**: AHI-BodyScan-Server

## Common Router Interfaces

### TP-Link
1. Advanced → NAT Forwarding → Virtual Servers
2. Click "Add"
3. Fill in the port forwarding details
4. Save

### Netgear
1. Dynamic DNS → Port Forwarding
2. Add Custom Service
3. Enter port details
4. Apply

### Linksys
1. Applications & Gaming → Port Range Forward
2. Enter application name and ports
3. Enable and Save Settings

### ASUS
1. WAN → Virtual Server / Port Forwarding
2. Enable Port Forwarding
3. Add profile with port details
4. Apply

### D-Link
1. Advanced → Port Forwarding
2. Create new rule
3. Enter port information
4. Save Settings

## Step 5: Configure Firewall (if applicable)

Some routers have separate firewall settings:

1. Find **Firewall** or **Security** settings
2. Ensure ports 80 and 443 are allowed
3. If there's a DMZ option, do NOT use it (security risk)

## Step 6: Test Your Configuration

After setting up port forwarding:

1. Visit: https://www.yougetsignal.com/tools/open-ports/
2. Enter port 80 and click "Check"
3. Enter port 443 and click "Check"
4. Both should show as "open"

Alternatively, from another network:
```bash
curl -I http://your-external-ip
curl -I https://your.duckdns.org
```

## Troubleshooting

### Ports Still Closed?

1. **Check server firewall**:
   ```bash
   sudo ufw status
   # Should show 80/tcp and 443/tcp as ALLOW
   ```

2. **Verify nginx is running**:
   ```bash
   sudo systemctl status nginx
   sudo ss -tln | grep -E ":80|:443"
   ```

3. **Double-check router settings**:
   - Ensure rules are enabled
   - Check for typos in IP address
   - Some routers require reboot after changes

4. **ISP Blocking**:
   - Some ISPs block port 80/443
   - Try alternative ports (8080, 8443)
   - Contact ISP to unblock

5. **Double NAT**:
   - Check if you have multiple routers
   - May need to configure both

### Security Considerations

- ✅ Only forward necessary ports (80, 443)
- ✅ Keep your server updated
- ✅ Use strong passwords
- ✅ Monitor access logs regularly
- ❌ Never use DMZ for production servers
- ❌ Don't forward unnecessary ports

## Next Steps

Once port forwarding is configured:

1. Run `sudo bash setup-letsencrypt.sh` to get SSL certificate
2. Access your app at `https://yourdomain.duckdns.org`
3. Set up monitoring and backups

## Need Help?

If you're having trouble:
1. Check your router model's manual
2. Search: "[Router Model] port forwarding guide"
3. Contact your ISP for assistance