# VastMindz Face Scan Setup Guide

This guide explains how to set up and use the VastMindz Web SDK for face scanning functionality in the AHI BodyScan React app.

## Prerequisites

1. **VastMindz Authentication Token**
   - An authentication token has been provided for this project
   - This token is required for the SDK to function properly
   - If you need a new token, contact team@vastmindz.com

2. **Browser Requirements**
   - Chrome, Edge, or Safari (latest versions)
   - Camera access permission
   - Stable internet connection for WebSocket communication

## Setup Instructions

### 1. Environment Configuration

1. Copy the example environment file:
   ```bash
   cp .env.example .env
   ```

2. Edit `.env` and add your VastMindz auth token:
   ```
   REACT_APP_VASTMINDZ_AUTH_TOKEN=your_actual_token_here
   ```

### 2. Running the Application

1. Install dependencies (if not already done):
   ```bash
   npm install
   ```

2. Start the development server:
   ```bash
   npm start
   ```

3. Navigate to the face scan feature:
   - Go to Home → New Scan → Face Scan

## How It Works

### SDK Integration

The VastMindz SDK is integrated using the following components:

1. **VastMindzSDK Service** (`src/services/vastmindz/VastMindzSDK.ts`)
   - Manages SDK initialization and lifecycle
   - Handles WebSocket connections
   - Processes real-time health data

2. **Face Scan Components**
   - `FaceScanCameraScreen`: Main scanning interface
   - `FaceMeshOverlay`: Real-time face tracking visualization
   - `PositioningGuide`: User positioning feedback
   - `ScanMetrics`: Live health metrics display

### Data Flow

1. User grants camera permission
2. SDK initializes camera and face tracking
3. WebSocket connection established with VastMindz servers
4. Real-time BGR signal data transmitted
5. Health metrics calculated and returned
6. Results displayed to user

## Features

### Real-time Feedback
- Face detection status
- Positioning guidance (too close/far, centered)
- Lighting condition warnings
- Movement detection
- Signal quality indicators

### Health Metrics
- Heart Rate (BPM)
- Heart Rate Variability (HRV)
- Blood Pressure estimation
- Respiratory Rate
- Oxygen Saturation (SpO2)
- Stress Level assessment

### No Fake Data
The implementation strictly uses real data from the VastMindz SDK. There are no simulated or fake health metrics.

## Troubleshooting

### SDK Not Loading
- Check browser console for errors
- Verify internet connectivity
- Ensure the SDK CDN is accessible

### WebSocket Connection Issues
- Verify auth token is correct
- Check firewall/proxy settings
- Ensure WebSocket port 443 is not blocked

### Camera Access Denied
- Check browser permissions
- Ensure HTTPS is being used (required for camera access)
- Try in an incognito/private window

### Poor Signal Quality
- Improve lighting conditions
- Ensure face is properly positioned
- Minimize movement during scan
- Remove glasses or face coverings

## Security Considerations

- Auth tokens should never be committed to version control
- Always use HTTPS in production
- Camera permissions are requested only when needed
- No video data is stored locally
- All health data transmission is encrypted

## Support

For SDK-related issues, contact:
- VastMindz Support: team@vastmindz.com
- GitHub Issues: https://github.com/Vastmindz-Public-Repository/Web-SDK

## Additional Resources

- [VastMindz Web SDK Documentation](https://github.com/Vastmindz-Public-Repository/Web-SDK)
- [WebRTC Camera API](https://developer.mozilla.org/en-US/docs/Web/API/MediaDevices/getUserMedia)
- [WebSocket API](https://developer.mozilla.org/en-US/docs/Web/API/WebSocket)