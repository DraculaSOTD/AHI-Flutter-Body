# Deployment Instructions

## 1. Deploy the React Build

Run the following command with sudo:
```bash
sudo ./deploy-to-web.sh
```

## 2. Update Nginx Configuration for Lambda Support

The nginx configuration needs to be updated to proxy Lambda requests. 

### Option A: Copy the prepared config file
```bash
sudo cp /tmp/ahi-bodyscan-nginx-update /etc/nginx/sites-available/ahi-bodyscan
```

### Option B: Manually add the Lambda proxy
Add this location block to `/etc/nginx/sites-available/ahi-bodyscan` after the `/api/health` location:

```nginx
# Lambda BHA API proxy
location /bha/ {
    proxy_pass https://o5t5ewcghf.execute-api.ap-southeast-2.amazonaws.com/bha/;
    proxy_http_version 1.1;
    proxy_set_header Host o5t5ewcghf.execute-api.ap-southeast-2.amazonaws.com;
    proxy_set_header X-Real-IP $remote_addr;
    proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
    proxy_set_header X-Forwarded-Proto $scheme;
    
    # Timeouts
    proxy_connect_timeout 30s;
    proxy_send_timeout 30s;
    proxy_read_timeout 30s;
    
    # Buffer settings
    proxy_buffering on;
    proxy_buffer_size 4k;
    proxy_buffers 8 4k;
    proxy_busy_buffers_size 8k;
}
```

## 3. Test and Reload Nginx

After updating the configuration:
```bash
# Test the configuration
sudo nginx -t

# If test passes, reload nginx
sudo systemctl reload nginx
```

## What Was Fixed

1. **Face Mesh Overlay**: Fixed coordinate transformation to properly scale the green face mesh overlay based on video display size with `object-fit: cover`

2. **Lambda Integration**: Added nginx proxy configuration to forward `/bha/` requests to AWS Lambda endpoint, fixing the issue where Lambda worked in development but not production

## Verification

After deployment:
1. Visit https://ahi.datapulseai.co
2. Test the face scan feature - the green mesh should properly align with faces
3. Complete a scan - the Lambda health assessment should now work properly