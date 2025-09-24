// RPPG SDK State
export enum RPPGState {
  UNINITIALIZED = 'UNINITIALIZED',
  INITIALIZING = 'INITIALIZING',
  INITIALIZED = 'INITIALIZED',
  CAMERA_READY = 'CAMERA_READY',
  TRACKING_READY = 'TRACKING_READY',
  SOCKET_CONNECTED = 'SOCKET_CONNECTED',
  ANALYSIS_RUNNING = 'ANALYSIS_RUNNING',
  ERROR = 'ERROR',
}

// Face position data
export interface FacePosition {
  leftEyeX: number;
  leftEyeY: number;
  rightEyeX: number;
  rightEyeY: number;
  landmarks?: number[][];
}

// Positioning feedback for user guidance
export interface PositioningFeedback {
  isFaceDetected: boolean;
  isFaceCentered: boolean;
  isFaceDistanceOk: boolean;
  isLightingOk: boolean;
  isStable: boolean;
  
  // Optional detailed feedback
  faceSize?: 'too_small' | 'too_large' | 'ok';
  facePosition?: 'centered' | 'too_left' | 'too_right' | 'too_high' | 'too_low';
  lighting?: 'too_bright' | 'too_dark' | 'uneven' | 'ok';
  movement?: 'no_movement' | 'slight_movement' | 'excessive_movement';
  
  // User-friendly message
  message?: string;
  severity?: 'info' | 'warning' | 'error';
}

// Scan progress information
export interface ScanProgress {
  percentage: number;
  timeElapsed: number;
  timeRemaining: number;
  framesProcessed: number;
  signalQuality: 'poor' | 'fair' | 'good' | 'excellent';
}

// Face scan data from SDK
export interface FaceScanData {
  bgrSignal: number[][];
  timestamp: number;
  receivedAt: Date;
  
  // Vital signs measurements
  heartRate?: number;
  heartRateVariability?: number;
  oxygenSaturation?: number;
  respiratoryRate?: number;
  bloodPressure?: {
    systolic: number;
    diastolic: number;
  };
  stressIndex?: number;
  
  // Metadata
  isFromVastMindz?: boolean;
  hasRealHealthMetrics?: boolean;
  signalQuality?: number;
}

// Face scan result after processing
export interface FaceScanResult {
  sessionId: string;
  timestamp: Date;
  duration: number; // seconds
  
  // Primary metrics
  heartRate: number; // bpm
  heartRateVariability: number; // ms
  respiratoryRate: number; // breaths per minute
  oxygenSaturation: number; // percentage
  systolicBP: number; // mmHg
  diastolicBP: number; // mmHg
  stressLevel: 'low' | 'normal' | 'moderate' | 'high';
  stressScore: number; // 0-1 scale
  
  // HRV metrics
  sdnn?: number; // Standard deviation of NN intervals
  rmssd?: number; // Root mean square of successive differences
  pnn50?: number; // Percentage of NN intervals differing by more than 50ms
  
  // Quality metrics
  signalQuality: number; // 0-1 scale
  confidence: number; // 0-1 scale
  
  // Lambda/BHA assessment fields
  healthScore?: number;
  heartAge?: number;
  biologicalAge?: number;
  arterialStiffness?: number;
  bmi?: number;
  
  // Cholesterol and Lipid Panel
  totalCholesterol?: number;
  ldlCholesterol?: number;
  hdlCholesterol?: number;
  triglycerides?: number;
  
  // Fitness and Cardiovascular Metrics
  fitnessScore?: number;
  vo2Max?: number;
  cvdRisk10Year?: number;
  framinghamScore?: number;
  maxHeartRate?: number;
  meanArterialPressure?: number;
  
  // Risk assessments
  bloodPressureRisk?: string;
  heartRateRisk?: string;
  cardiovascularRisk?: string;
  diabetesRisk?: string;
  metabolicRisk?: string;
  cholesterolRisk?: string;
  ldlRisk?: string;
  hdlRisk?: string;
  triglyceridesRisk?: string;
  
  // Insights
  recommendations?: string[];
  insights?: string[];
}