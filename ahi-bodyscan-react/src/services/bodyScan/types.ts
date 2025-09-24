// Body part measurements in cm
export interface BodyMeasurements {
  chest: number;
  waist: number;
  hip: number;
  thigh: number;
  inseam: number;
  bodyFat: number; // percentage
  muscleMass: number; // kg
  visceralFat: number; // level
  weight: number; // kg
  metabolicAge?: number; // years
  bodyType?: string; // ectomorph, mesomorph, endomorph
}

// Joint points for pose detection
export interface Joint {
  name: string;
  x: number;
  y: number;
  z?: number;
  confidence: number;
}

// Pose validation feedback
export interface PoseFeedback {
  type: 'error' | 'warning' | 'info';
  message: string;
  jointName?: string;
}

// Segmentation mask
export interface SegmentationMask {
  data: Uint8Array;
  width: number;
  height: number;
}

// 3D Avatar mesh data
export interface AvatarMesh {
  vertices: Float32Array;
  faces: Uint32Array;
  normals: Float32Array;
  bones?: any[];
}

// Processing options
export interface ProcessingOptions {
  useGPU?: boolean;
  modelVersion?: string;
  generateAvatar?: boolean;
}

// Model configuration
export interface ModelConfig {
  jointDetectionModel: string;
  segmentationModel: string;
  classificationModels: {
    male: string;
    female: string;
  };
  svrModels: {
    [key: string]: string;
  };
}

// Alignment requirements
export interface AlignmentRequirements {
  deviceAngleTolerance: number; // degrees
  minJointConfidence: number; // 0-1
  requiredJoints: string[];
}

// Image requirements
export interface ImageRequirements {
  minWidth: number;
  minHeight: number;
  aspectRatio: number;
  format: 'jpeg' | 'png';
}

// Error types
export enum BodyScanErrorType {
  CAMERA_ACCESS_DENIED = 'CAMERA_ACCESS_DENIED',
  INVALID_USER_PROFILE = 'INVALID_USER_PROFILE',
  POSE_DETECTION_FAILED = 'POSE_DETECTION_FAILED',
  SEGMENTATION_FAILED = 'SEGMENTATION_FAILED',
  MEASUREMENT_FAILED = 'MEASUREMENT_FAILED',
  MODEL_LOAD_FAILED = 'MODEL_LOAD_FAILED',
  ALIGNMENT_ERROR = 'ALIGNMENT_ERROR',
  PROCESSING_TIMEOUT = 'PROCESSING_TIMEOUT'
}

export class BodyScanError extends Error {
  constructor(
    public type: BodyScanErrorType,
    message: string,
    public details?: any
  ) {
    super(message);
    this.name = 'BodyScanError';
  }
}