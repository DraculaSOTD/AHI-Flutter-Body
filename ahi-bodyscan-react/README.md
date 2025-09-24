# AHI BodyScan React App

A comprehensive health scanning application featuring face scanning for vital signs and body scanning for measurements and composition analysis.

**Live Demo**: https://ahi.datapulseai.co

## 🚀 Features

### Face Scanning (VastMindz SDK)
- **60-second scans** using device camera
- **Real-time health metrics**: Heart rate, HRV, blood pressure, respiratory rate, SpO2, stress levels
- **Visual feedback**: Face positioning guidance and live metrics display
- **Comprehensive results**: Health insights and recommendations
- **PDF export**: Generate detailed health reports

### Body Scanning (In Development)
- **3D body measurements**: Chest, waist, hip, thigh, inseam
- **Body composition**: Body fat %, fat-free mass, visceral fat
- **Avatar generation**: 3D mesh visualization
- **Machine learning models**: 54+ MB of embedded CoreML models

## 🛠️ Technology Stack

- **Frontend**: React 19, TypeScript, Material-UI
- **State Management**: Redux Toolkit
- **Routing**: React Router v6
- **Styling**: Styled Components, Emotion
- **Health SDKs**: VastMindz Web SDK (rPPG)
- **ML/AI**: TensorFlow.js, Body-Pix, Pose Detection
- **3D Graphics**: Three.js, React Three Fiber
- **PDF Generation**: jsPDF
- **Build Tool**: Create React App

## 🏁 Getting Started

### Prerequisites
- Node.js 16+ and npm
- Camera access for scanning features
- Modern browser with WebRTC support

### Installation

```bash
# Clone the repository
git clone [repository-url]
cd ahi-bodyscan-react

# Install dependencies
npm install

# Create environment file
cp .env.example .env
# Add your VastMindz auth token (current: 2b87039d-d352-4526-b245-4dbefc5cf636)

# Start development server
npm start
```

The app will open at http://localhost:3000

### Building for Production

```bash
# Create optimized production build
npm run build

# Deploy to server (requires sudo)
sudo bash deploy-to-web.sh
```

## 🌐 Production Hosting

### Live URL
- **Public Access**: https://ahi.datapulseai.co
- **Local Access**: https://192.168.10.117

### Server Configuration
- **Web Server**: Nginx 1.28.0
- **SSL**: Let's Encrypt (expires 2025-10-24)
- **Root Directory**: `/srv/http/ahi-bodyscan`
- **Server**: Arch Linux

### Deployment Process

1. **Build the app**:
   ```bash
   npm run build
   ```

2. **Deploy to production**:
   ```bash
   sudo bash deploy-to-web.sh
   ```

3. **Quick updates** (builds and deploys):
   ```bash
   bash update-deployment.sh
   ```

### Auto-Start Services

After system restart:
- ✅ **Nginx**: Starts automatically
- ✅ **Website**: Immediately available
- ⚠️ **SSL Renewal**: Manual setup needed (see below)

## 🔧 Maintenance

### Update Application
```bash
# Make your changes, then:
bash update-deployment.sh
```

### SSL Certificate
- **Current Certificate**: Valid until 2025-10-24
- **Manual Renewal**: `sudo certbot renew`
- **Check Status**: `sudo certbot certificates`

### Monitoring
- **Access Logs**: `sudo tail -f /var/log/nginx/access.log`
- **Error Logs**: `sudo tail -f /var/log/nginx/error.log`
- **Service Status**: `sudo systemctl status nginx`

### Backup
```bash
# Backup configuration
cp /etc/nginx/nginx.conf ~/nginx.conf.backup

# Backup app
tar -czf ~/ahi-backup-$(date +%Y%m%d).tar.gz /srv/http/ahi-bodyscan
```

## 🗂️ Project Structure

```
ahi-bodyscan-react/
├── build/                    # Production build files
├── public/                   # Static assets
├── src/
│   ├── components/          # Reusable UI components
│   ├── features/           # Feature-specific components
│   │   ├── face-scan/     # Face scanning features
│   │   └── body-scan/     # Body scanning features
│   ├── services/          # Business logic and SDKs
│   │   ├── vastmindz/    # VastMindz SDK integration
│   │   ├── bodyScan/     # Body scan service
│   │   └── pdf/          # PDF generation
│   └── styles/            # Global styles and theme
├── BodyScanML/            # Machine learning models
├── nginx-domain.conf      # Nginx configuration
├── deploy-to-web.sh       # Deployment script
├── update-deployment.sh   # Update script
└── setup-*.sh            # Various setup scripts
```

## 🔒 Security

### HTTPS Configuration
- **SSL/TLS**: Enforced with HSTS
- **Protocols**: TLS 1.2 and 1.3 only
- **Certificate**: Let's Encrypt auto-renewal

### Security Headers
- X-Frame-Options: SAMEORIGIN
- X-Content-Type-Options: nosniff
- X-XSS-Protection: 1; mode=block
- Strict-Transport-Security: max-age=63072000
- Content-Security-Policy: Configured for app requirements

### Rate Limiting
- Nginx rate limiting: 30 requests/second
- Burst handling with delay

## 📱 App Usage

### Face Scan Flow
1. Home Screen → Select "New Face Scan"
2. Enter user profile (age, gender, height, weight)
3. Review positioning instructions
4. Complete 60-second scan
5. View results and export PDF

### Body Scan Flow (Coming Soon)
1. Home Screen → Select "Body Scan"
2. Enter measurements (height, weight, gender)
3. Position for front photo
4. Position for side photo
5. View measurements and 3D avatar

## 🐛 Troubleshooting

### Website Not Accessible
```bash
# Check nginx status
sudo systemctl status nginx

# Test configuration
sudo nginx -t

# Check logs
sudo journalctl -u nginx -n 50

# Verify DNS
curl -I https://ahi.datapulseai.co
```

### Camera Not Working
- Ensure HTTPS is enabled (required for camera access)
- Check browser permissions
- Try different browser if issues persist

### Build Failures
```bash
# Clear cache and reinstall
rm -rf node_modules package-lock.json
npm install
npm run build
```

## 📝 Development Notes

### Available Scripts
- `npm start` - Development server
- `npm run build` - Production build
- `npm test` - Run tests
- `npm run eject` - Eject from CRA (irreversible)

### Environment Variables
- `REACT_APP_VASTMINDZ_AUTH_TOKEN` - VastMindz SDK authentication

### Code Style
- TypeScript for type safety
- Functional components with hooks
- Styled-components for styling
- Redux Toolkit for state management

## 🤝 Contributing

1. Create feature branch
2. Make changes
3. Test thoroughly
4. Submit pull request

## 📄 License

Private repository - All rights reserved

## 🆘 Support

For issues or questions:
- Check logs in `/var/log/nginx/`
- Review error messages in browser console
- Ensure all services are running

---

**Server Admin Notes**: 
- Router ports 80 and 443 forwarded to 192.168.10.117
- DNS managed via Squarespace (A record: ahi → 102.182.135.156)
- SSL certificates managed by certbot