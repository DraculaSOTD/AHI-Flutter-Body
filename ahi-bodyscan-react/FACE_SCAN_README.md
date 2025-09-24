# VastMindz Face Scanning Integration

This React app includes a complete implementation of the VastMindz Web SDK for face scanning and health metrics analysis.

## Features

- **Real-time Face Scanning**: 60-second face scans using the device camera
- **Health Metrics**: Heart rate, HRV, blood pressure, respiratory rate, SpO2, stress levels
- **Visual Feedback**: Face positioning guidance and real-time metrics display
- **Results Display**: Comprehensive health metrics with insights and recommendations

## Setup

1. **Install Dependencies**
   ```bash
   npm install
   ```

2. **Configure Authentication**
   - Copy `.env.example` to `.env`
   - Add your VastMindz auth token (already configured: `2b87039d-d352-4526-b245-4dbefc5cf636`)

3. **Run the Application**
   ```bash
   npm start
   ```

## Face Scan Flow

1. **Home Screen** → Select "New Face Scan"
2. **User Profile** → Enter age, gender, height, weight
3. **Preparation** → Review positioning instructions
4. **Camera Scan** → 60-second scan with real-time feedback
5. **Results** → View comprehensive health metrics

## Technical Implementation

### SDK Integration (`/src/services/vastmindz/`)
- `VastMindzSDK.ts` - Main SDK wrapper with TypeScript support
- `VastMindzTypes.ts` - Type definitions and configuration

### Key Components
- `FaceScanCameraScreen.tsx` - Camera capture and real-time scanning
- `FaceScanResultsScreen.tsx` - Results display and health insights
- `ScanMetrics.tsx` - Real-time metrics visualization
- `FaceMeshOverlay.tsx` - Face tracking visualization

### Health Metrics Captured
- Heart Rate (BPM)
- Heart Rate Variability (ms)
- Blood Pressure (systolic/diastolic)
- Respiratory Rate (breaths/min)
- Oxygen Saturation (SpO2 %)
- Stress Level (low/normal/moderate/high)

## SDK Configuration

The SDK is configured with:
- WebSocket URL: `wss://vm-production.xyz/vp/bgr_signal_socket`
- Camera Resolution: 640x480
- Scan Duration: 60 seconds
- Real-time callbacks for metrics and positioning

## Testing

To test the face scan:
1. Navigate to http://localhost:3000
2. Click "New Face Scan"
3. Enter user profile information
4. Follow on-screen positioning instructions
5. Complete the 60-second scan
6. Review results

## Notes

- Requires camera permissions
- Best results in good lighting conditions
- User should remain still during scanning
- All processing is done via VastMindz cloud API