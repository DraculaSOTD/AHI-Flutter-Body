import RPPG from 'vastmindz-web-sdk/dist';
import type { RPPGConfig, RPPGOnFrame } from 'vastmindz-web-sdk/dist/lib/RPPG.types';
import type {
  MeasurementMeanData,
  MeasurementProgress,
  MeasurementSignal,
  MeasurementStatus,
  MovingWarning,
  SignalQuality,
  BloodPressure,
  HrvMetrics,
  StressIndex,
} from 'vastmindz-web-sdk/dist/lib/RPPGEvents.types';
// Import enums as values, not types
import {
  StatusCode,
  BloodPressureStatus,
  StressStatus,
} from 'vastmindz-web-sdk/dist/lib/RPPGEvents.types';
import { FaceScanData, RPPGState, FacePosition, PositioningFeedback, ScanProgress } from './RPPGTypes';

// Configuration constants
const VASTMINDZ_AUTH_TOKEN = '2b87039d-d352-4526-b245-4dbefc5cf636';
const VASTMINDZ_WEBSOCKET_URL = 'wss://vm-production.xyz/vp/bgr_signal_socket';

export class RPPGService {
  private rppgInstance: RPPG | null = null;
  private state: RPPGState = RPPGState.UNINITIALIZED;
  private isServerless: boolean = false;
  
  // Callbacks
  private onStateChange?: (state: RPPGState) => void;
  private onDataReceived?: (data: FaceScanData) => void;
  private onProgressUpdate?: (progress: ScanProgress) => void;
  private onPositioningFeedback?: (feedback: PositioningFeedback) => void;
  private onError?: (error: Error) => void;
  private onScanComplete?: (data: FaceScanData) => void;
  
  // Internal state
  private lastFacePosition?: FacePosition;
  private frameCount = 0;
  private startTime = 0;
  private signalQuality = 0;
  private videoElement?: HTMLVideoElement;
  private canvasElement?: HTMLCanvasElement;
  private userData?: { sex: string; age: number; weight: number; height: number };
  
  // Track received metrics
  private metricsReceived = {
    heartRate: false,
    heartRateVariability: false,
    respiratoryRate: false,
    oxygenSaturation: false,
    bloodPressure: false,
    stressLevel: false
  };
  
  // Store latest values for combined reporting
  private latestMetrics = {
    heartRate: 0,
    heartRateVariability: 0,
    respiratoryRate: 0,
    oxygenSaturation: 0,
    bloodPressure: { systolic: 0, diastolic: 0 },
    stressStatus: StressStatus.NO_DATA,
  };
  
  constructor(serverless: boolean = false) {
    this.isServerless = serverless;
  }
  
  // Set callbacks
  setCallbacks(callbacks: {
    onStateChange?: (state: RPPGState) => void;
    onDataReceived?: (data: FaceScanData) => void;
    onProgressUpdate?: (progress: ScanProgress) => void;
    onPositioningFeedback?: (feedback: PositioningFeedback) => void;
    onError?: (error: Error) => void;
    onScanComplete?: (data: FaceScanData) => void;
  }) {
    this.onStateChange = callbacks.onStateChange;
    this.onDataReceived = callbacks.onDataReceived;
    this.onProgressUpdate = callbacks.onProgressUpdate;
    this.onPositioningFeedback = callbacks.onPositioningFeedback;
    this.onError = callbacks.onError;
    this.onScanComplete = callbacks.onScanComplete;
  }
  
  private updateState(newState: RPPGState) {
    this.state = newState;
    this.onStateChange?.(newState);
    console.log(`RPPG: State changed to ${newState}`);
  }
  
  async initialize(): Promise<void> {
    try {
      this.updateState(RPPGState.INITIALIZING);
      
      // Create RPPG instance with all event handlers
      this.rppgInstance = new RPPG({
        serverless: this.isServerless,
        skipSocketWhenNoFace: true,
        
        // Event handlers
        onFrame: this.handleFrame.bind(this),
        onMeasurementMeanData: this.handleMeasurementData.bind(this),
        onMeasurementProgress: this.handleProgress.bind(this),
        onMeasurementSignal: this.handleSignal.bind(this),
        onMeasurementStatus: this.handleStatus.bind(this),
        onMovingWarning: this.handleMovingWarning.bind(this),
        onFaceSizeWarning: this.handleFaceSizeWarning.bind(this),
        onFaceOrientWarning: this.handleFaceOrientWarning.bind(this),
        onInterferenceWarning: this.handleInterferenceWarning.bind(this),
        onUnstableConditionsWarning: this.handleUnstableConditionsWarning.bind(this),
        onSignalQuality: this.handleSignalQuality.bind(this),
        onHrvMetrics: this.handleHrvMetrics.bind(this),
        onStressIndex: this.handleStressIndex.bind(this),
        onBloodPressure: this.handleBloodPressure.bind(this),
      });
      
      this.updateState(RPPGState.INITIALIZED);
      console.log('RPPG: SDK initialized successfully');
    } catch (error) {
      this.updateState(RPPGState.ERROR);
      this.handleError(error);
      throw error;
    }
  }
  
  async setupCamera(videoElement: HTMLVideoElement, canvasElement: HTMLCanvasElement): Promise<void> {
    if (!this.rppgInstance) {
      throw new Error('SDK not initialized. Call initialize() first.');
    }
    
    try {
      this.videoElement = videoElement;
      this.canvasElement = canvasElement;
      
      await this.rppgInstance.initCamera({
        width: 640,
        height: 480,
        videoElement,
        canvasElement,
        useFrontCamera: true,
        onSuccess: (data: any) => {
          console.log('RPPG: Camera initialized', data);
          this.updateState(RPPGState.CAMERA_READY);
        },
        onError: (error: any) => {
          this.handleError(error);
        },
      });
    } catch (error) {
      this.updateState(RPPGState.ERROR);
      throw error;
    }
  }
  
  async setupTracker(): Promise<void> {
    if (!this.rppgInstance) {
      throw new Error('SDK not initialized. Call initialize() first.');
    }
    
    try {
      await this.rppgInstance.initTracker({
        pathToWasmData: 'https://websdk1.blob.core.windows.net/sdk-1-2-3/',
        maxTimeBetweenBlinksSeconds: 60,
      });
      
      this.updateState(RPPGState.TRACKING_READY);
      console.log('RPPG: Tracker initialized');
    } catch (error) {
      this.updateState(RPPGState.ERROR);
      throw error;
    }
  }
  
  async connectSocket(userData: {
    sex: string;
    age: number;
    weight: number;
    height: number;
  }): Promise<void> {
    if (!this.rppgInstance || this.isServerless) {
      return;
    }
    
    this.userData = userData;
    
    try {
      // Build query string with auth token and user data
      const queryParams = new URLSearchParams({
        authToken: VASTMINDZ_AUTH_TOKEN,
        sex: userData.sex,
        age: userData.age.toString(),
        weight: userData.weight.toString(),
        height: userData.height.toString(),
      }).toString();
      
      console.log('RPPG: Connecting WebSocket with user data:', {
        sex: userData.sex,
        age: userData.age,
        weight: userData.weight,
        height: userData.height,
        queryString: queryParams,
        fullUrl: `${VASTMINDZ_WEBSOCKET_URL}?${queryParams}`
      });
      
      await this.rppgInstance.initSocket({
        url: VASTMINDZ_WEBSOCKET_URL,
        authToken: '', // Auth token is now in query params
        query: queryParams,
        onConnect: () => {
          console.log('RPPG: WebSocket connected successfully');
          this.updateState(RPPGState.SOCKET_CONNECTED);
        },
        onClose: (event: any) => {
          console.log('RPPG: WebSocket closed', event);
        },
        onError: (event: any) => {
          console.error('RPPG: WebSocket error', event);
          this.handleError(new Error('WebSocket connection failed'));
        },
      });
    } catch (error) {
      console.error('RPPG: Failed to connect socket', error);
      // Don't throw - allow SDK to work in serverless mode
    }
  }
  
  async startAnalysis(): Promise<void> {
    if (!this.rppgInstance) {
      throw new Error('SDK not initialized');
    }
    
    this.frameCount = 0;
    this.startTime = Date.now();
    
    // Reset metrics tracking
    this.metricsReceived = {
      heartRate: false,
      heartRateVariability: false,
      respiratoryRate: false,
      oxygenSaturation: false,
      bloodPressure: false,
      stressLevel: false
    };
    
    this.rppgInstance.start();
    this.updateState(RPPGState.ANALYSIS_RUNNING);
    console.log('RPPG: Analysis started');
  }
  
  stopAnalysis(): void {
    if (this.rppgInstance) {
      this.rppgInstance.stop();
      this.updateState(RPPGState.TRACKING_READY);
      console.log('RPPG: Analysis stopped');
    }
  }
  
  // Frame processing callback
  private handleFrame(data: RPPGOnFrame): void {
    this.frameCount++;
    
    // Check if face is detected (status 0 = SUCCESS)
    if (data.rppgTrackerData.status === 0 && data.rppgTrackerData.face && data.rppgTrackerData.face.length === 4) {
      // Face data is [x, y, width, height]
      const [x, y, width, height] = data.rppgTrackerData.face;
      
      // Estimate eye positions from face bounding box
      this.lastFacePosition = {
        leftEyeX: x + width * 0.33,
        leftEyeY: y + height * 0.33,
        rightEyeX: x + width * 0.67,
        rightEyeY: y + height * 0.33,
      };
      
      // Calculate face position feedback
      this.calculatePositioningFeedback();
    } else {
      // No face detected
      this.onPositioningFeedback?.({
        isFaceDetected: false,
        isFaceCentered: false,
        isFaceDistanceOk: false,
        isLightingOk: false,
        isStable: false,
        message: 'No face detected. Please position your face in the camera view.',
        severity: 'warning',
      });
    }
  }
  
  // Measurement data callback
  private handleMeasurementData(data: MeasurementMeanData): void {
    console.log('RPPG: Raw measurement data received:', {
      bpm: data.bpm,
      oxygen: data.oxygen,
      rr: data.rr,
      stressStatus: data.stressStatus,
      bloodPressureStatus: data.bloodPressureStatus,
      timestamp: Date.now()
    });
    
    // Update latest metrics
    if (data.bpm > 0) {
      this.latestMetrics.heartRate = data.bpm;
      this.metricsReceived.heartRate = true;
    }
    if (data.oxygen > 0) {
      this.latestMetrics.oxygenSaturation = data.oxygen;
      this.metricsReceived.oxygenSaturation = true;
    }
    if (data.rr > 0) {
      this.latestMetrics.respiratoryRate = data.rr;
      this.metricsReceived.respiratoryRate = true;
    }
    if (data.stressStatus !== StressStatus.NO_DATA) {
      this.latestMetrics.stressStatus = data.stressStatus;
      this.metricsReceived.stressLevel = true;
    }
    
    // Map the data to FaceScanData format
    const faceScanData: FaceScanData = {
      bgrSignal: [],
      timestamp: Date.now(),
      receivedAt: new Date(),
      heartRate: data.bpm,
      heartRateVariability: this.latestMetrics.heartRateVariability,
      oxygenSaturation: data.oxygen ? Math.round(data.oxygen) : undefined,
      respiratoryRate: data.rr ? Math.round(data.rr) : undefined,
      stressIndex: this.parseStressLevel(data.stressStatus),
      bloodPressure: data.bloodPressureStatus === BloodPressureStatus.NO_DATA ? undefined : this.latestMetrics.bloodPressure,
      isFromVastMindz: true,
      hasRealHealthMetrics: true,
    };
    
    // Send data to callback
    this.onDataReceived?.(faceScanData);
    
    console.log('RPPG: Processed scan data:', faceScanData);
    console.log('RPPG: Metrics received status:', this.metricsReceived);
  }
  
  // Signal data callback
  private handleSignal(data: MeasurementSignal): void {
    // Signal data for visualization purposes
    if (data && data.signal) {
      console.log('RPPG: Signal data received', {
        signalLength: data.signal.length,
        timestamp: Date.now()
      });
    }
  }
  
  // Progress callback
  private handleProgress(data: MeasurementProgress): void {
    const timeElapsed = (Date.now() - this.startTime) / 1000;
    
    const progress: ScanProgress = {
      percentage: data.progressPercent,
      timeElapsed,
      timeRemaining: 60 - timeElapsed,
      framesProcessed: this.frameCount,
      signalQuality: this.getSignalQualityLevel(this.signalQuality),
    };
    
    this.onProgressUpdate?.(progress);
  }
  
  // Status callback
  private handleStatus(data: MeasurementStatus): void {
    console.log('RPPG: Status update', data);
    
    // Handle specific status codes
    if (data.statusCode === StatusCode.NOISE_DURING_EXECUTION) {
      console.log('RPPG: Noise detected, SDK will handle recalibration');
    } else if (data.statusCode === StatusCode.RECALIBRATING) {
      console.log('RPPG: SDK is recalibrating');
    }
  }
  
  // Warning callbacks
  private handleMovingWarning(data: MovingWarning): void {
    this.onPositioningFeedback?.({
      isFaceDetected: true,
      isFaceCentered: true,
      isFaceDistanceOk: true,
      isLightingOk: true,
      isStable: false,
      message: 'Please hold still during the scan',
      severity: 'warning',
      movement: 'slight_movement',
    });
  }
  
  private handleFaceSizeWarning(): void {
    this.onPositioningFeedback?.({
      isFaceDetected: true,
      isFaceCentered: true,
      isFaceDistanceOk: false,
      isLightingOk: true,
      isStable: true,
      message: 'Please adjust your distance from the camera',
      severity: 'warning',
      faceSize: 'too_small',
    });
  }
  
  private handleFaceOrientWarning(): void {
    this.onPositioningFeedback?.({
      isFaceDetected: true,
      isFaceCentered: false,
      isFaceDistanceOk: true,
      isLightingOk: true,
      isStable: true,
      message: 'Please face the camera directly',
      severity: 'warning',
    });
  }
  
  private handleInterferenceWarning(): void {
    this.onPositioningFeedback?.({
      isFaceDetected: true,
      isFaceCentered: true,
      isFaceDistanceOk: true,
      isLightingOk: false,
      isStable: true,
      message: 'Lighting conditions are not optimal',
      severity: 'warning',
      lighting: 'uneven',
    });
  }
  
  private handleUnstableConditionsWarning(): void {
    this.onPositioningFeedback?.({
      isFaceDetected: true,
      isFaceCentered: true,
      isFaceDistanceOk: true,
      isLightingOk: false,
      isStable: false,
      message: 'Unstable conditions detected',
      severity: 'warning',
    });
  }
  
  private handleSignalQuality(data: SignalQuality): void {
    this.signalQuality = data.snr;
  }
  
  private handleHrvMetrics(data: HrvMetrics): void {
    console.log('RPPG: HRV metrics received:', {
      sdnn: data.sdnn,
      rmssd: data.rmssd,
      ibi: data.ibi,
      timestamp: Date.now()
    });
    
    // Update latest HRV value
    if (data.sdnn && data.sdnn > 0) {
      this.latestMetrics.heartRateVariability = data.sdnn;
      this.metricsReceived.heartRateVariability = true;
    }
    
    // Send HRV data as a measurement update
    const faceScanData: FaceScanData = {
      bgrSignal: [],
      timestamp: Date.now(),
      receivedAt: new Date(),
      heartRate: this.latestMetrics.heartRate,
      heartRateVariability: data.sdnn,
      oxygenSaturation: this.latestMetrics.oxygenSaturation,
      respiratoryRate: this.latestMetrics.respiratoryRate,
      bloodPressure: this.latestMetrics.bloodPressure,
      stressIndex: this.parseStressLevel(this.latestMetrics.stressStatus),
      isFromVastMindz: true,
      hasRealHealthMetrics: true,
    };
    
    this.onDataReceived?.(faceScanData);
  }
  
  private handleStressIndex(data: StressIndex): void {
    console.log('RPPG: Stress index', data);
  }
  
  private handleBloodPressure(data: BloodPressure): void {
    console.log('RPPG: Blood pressure received:', {
      systolic: data.systolic,
      diastolic: data.diastolic,
      timestamp: Date.now()
    });
    
    // Update latest blood pressure
    if (data.systolic > 0 && data.diastolic > 0) {
      this.latestMetrics.bloodPressure = {
        systolic: data.systolic,
        diastolic: data.diastolic,
      };
      this.metricsReceived.bloodPressure = true;
    }
    
    // Send blood pressure data as a measurement
    const faceScanData: FaceScanData = {
      bgrSignal: [],
      timestamp: Date.now(),
      receivedAt: new Date(),
      heartRate: this.latestMetrics.heartRate,
      heartRateVariability: this.latestMetrics.heartRateVariability,
      oxygenSaturation: this.latestMetrics.oxygenSaturation,
      respiratoryRate: this.latestMetrics.respiratoryRate,
      bloodPressure: {
        systolic: data.systolic,
        diastolic: data.diastolic,
      },
      stressIndex: this.parseStressLevel(this.latestMetrics.stressStatus),
      isFromVastMindz: true,
      hasRealHealthMetrics: true,
    };
    
    this.onDataReceived?.(faceScanData);
  }
  
  private handleError(error: any): void {
    console.error('RPPG: Error', error);
    this.onError?.(error instanceof Error ? error : new Error(String(error)));
  }
  
  // Helper methods
  private calculatePositioningFeedback(): void {
    if (!this.lastFacePosition || !this.canvasElement) {
      return;
    }
    
    const canvasWidth = this.canvasElement.width;
    const canvasHeight = this.canvasElement.height;
    
    // Calculate face center
    const faceCenterX = (this.lastFacePosition.leftEyeX + this.lastFacePosition.rightEyeX) / 2;
    const faceCenterY = (this.lastFacePosition.leftEyeY + this.lastFacePosition.rightEyeY) / 2;
    
    // Calculate face size (distance between eyes)
    const eyeDistance = Math.abs(this.lastFacePosition.rightEyeX - this.lastFacePosition.leftEyeX);
    
    // Determine positioning
    const centerThreshold = 0.2; // 20% from center
    const isCenteredX = Math.abs(faceCenterX - canvasWidth / 2) < canvasWidth * centerThreshold;
    const isCenteredY = Math.abs(faceCenterY - canvasHeight / 2) < canvasHeight * centerThreshold;
    
    // Determine face size (based on eye distance)
    const idealEyeDistance = canvasWidth * 0.2; // 20% of canvas width
    const sizeRatio = eyeDistance / idealEyeDistance;
    
    let faceSize: 'too_small' | 'too_large' | 'ok' = 'ok';
    if (sizeRatio < 0.7) faceSize = 'too_small';
    else if (sizeRatio > 1.3) faceSize = 'too_large';
    
    let facePosition: PositioningFeedback['facePosition'] = 'centered';
    if (!isCenteredX) {
      facePosition = faceCenterX < canvasWidth / 2 ? 'too_left' : 'too_right';
    } else if (!isCenteredY) {
      facePosition = faceCenterY < canvasHeight / 2 ? 'too_high' : 'too_low';
    }
    
    const feedback: PositioningFeedback = {
      isFaceDetected: true,
      isFaceCentered: isCenteredX && isCenteredY,
      isFaceDistanceOk: faceSize === 'ok',
      isLightingOk: true,
      isStable: true,
      faceSize,
      facePosition,
    };
    
    // Add message if needed
    if (!feedback.isFaceCentered) {
      feedback.message = 'Please center your face in the frame';
      feedback.severity = 'info';
    } else if (!feedback.isFaceDistanceOk) {
      feedback.message = faceSize === 'too_small' ? 
        'Please move closer to the camera' : 
        'Please move further from the camera';
      feedback.severity = 'info';
    }
    
    this.onPositioningFeedback?.(feedback);
  }
  
  private parseStressLevel(status: StressStatus): number | undefined {
    switch (status) {
      case StressStatus.LOW: return 0.25;
      case StressStatus.NORMAL: return 0.5;
      case StressStatus.ELEVATED: return 0.7;
      case StressStatus.VERY_HIGH: return 0.9;
      case StressStatus.NO_DATA:
      case StressStatus.UNKNOWN:
      default: return undefined;
    }
  }
  
  private getSignalQualityLevel(snr: number): ScanProgress['signalQuality'] {
    if (snr >= 0.8) return 'excellent';
    if (snr >= 0.6) return 'good';
    if (snr >= 0.4) return 'fair';
    return 'poor';
  }
  
  // Check if we have received complete metrics
  hasCompleteMetrics(): boolean {
    const allMetricsReceived = 
      this.metricsReceived.heartRate && 
      this.metricsReceived.heartRateVariability &&
      this.metricsReceived.oxygenSaturation && 
      this.metricsReceived.respiratoryRate &&
      this.metricsReceived.bloodPressure &&
      this.metricsReceived.stressLevel;
    
    const receivedCount = Object.values(this.metricsReceived).filter(Boolean).length;
    
    console.log('RPPG: Metrics completion check:', {
      heartRate: this.metricsReceived.heartRate,
      heartRateVariability: this.metricsReceived.heartRateVariability,
      oxygenSaturation: this.metricsReceived.oxygenSaturation,
      respiratoryRate: this.metricsReceived.respiratoryRate,
      bloodPressure: this.metricsReceived.bloodPressure,
      stressLevel: this.metricsReceived.stressLevel,
      totalReceived: receivedCount,
      isComplete: allMetricsReceived,
    });
    
    return allMetricsReceived;
  }
  
  // Get the status of which metrics have been received
  getMetricsStatus(): typeof this.metricsReceived {
    return { ...this.metricsReceived };
  }
  
  // Getters
  get currentState(): RPPGState {
    return this.state;
  }
  
  get isRunning(): boolean {
    return this.rppgInstance?.processing || false;
  }
  
  get facePosition(): FacePosition | undefined {
    return this.lastFacePosition;
  }
  
  // Cleanup
  dispose(): void {
    this.stopAnalysis();
    if (this.rppgInstance) {
      this.rppgInstance.closeCamera();
    }
    this.rppgInstance = null;
    this.updateState(RPPGState.UNINITIALIZED);
  }
}

// Export singleton instance
let rppgServiceInstance: RPPGService | null = null;

export function getRPPGService(): RPPGService {
  if (!rppgServiceInstance) {
    rppgServiceInstance = new RPPGService();
  }
  return rppgServiceInstance;
}