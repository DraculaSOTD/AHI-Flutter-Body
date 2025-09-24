import { CameraService } from '../camera/CameraService';
import { ModelService } from './models/ModelService';
import * as poseDetection from '@tensorflow-models/pose-detection';
import { BodyMeasurements } from './types';
import { PoseInspectionService, PoseInspectionResult, ContourZones } from './PoseInspectionService';
import { ContourService } from './ContourService';
import { AlignmentService } from './AlignmentService';
import { PositioningValidator, PositioningMessage } from './PositioningValidator';

export enum BodyScanPhase {
  IDLE = 'idle',
  SETUP = 'setup',
  FRONT_CAPTURE = 'front_capture',
  SIDE_CAPTURE = 'side_capture',
  PROCESSING = 'processing',
  COMPLETE = 'complete',
  ERROR = 'error'
}

export enum BodyScanPose {
  FRONT = 'front',
  SIDE = 'side'
}

export interface UserProfile {
  height: number; // in cm
  weight: number; // in kg
  gender: 'male' | 'female';
  age?: number;
}


export interface JointPoint {
  x: number;
  y: number;
  confidence: number;
}

export interface PoseDetectionResult {
  joints: JointPoint[];
  isValid: boolean;
  feedback: string[];
  inspectionResult?: any; // PoseInspectionDictionary
  alignmentScore?: number;
}

export interface BodyScanResult {
  measurements: BodyMeasurements;
  frontImage: string;
  sideImage: string;
  avatarModel?: any; // 3D model data
  confidence: number;
  timestamp: Date;
}

export class BodyScanService {
  private cameraService: CameraService;
  private modelService: ModelService;
  private poseInspectionService: PoseInspectionService;
  private contourService: ContourService;
  private alignmentService: AlignmentService;
  private positioningValidator: PositioningValidator;
  private currentPhase: BodyScanPhase = BodyScanPhase.IDLE;
  private userProfile: UserProfile | null = null;
  private capturedImages: {
    front?: string;
    side?: string;
  } = {};
  private frameProcessingTimer: NodeJS.Timer | null = null;
  private _isInitialized: boolean = false;
  private _isPoseDetectionRunning: boolean = false;
  private contourZones: ContourZones | null = null;
  
  private phaseChangeCallbacks: ((phase: BodyScanPhase) => void)[] = [];
  private poseDetectionCallbacks: ((result: PoseDetectionResult) => void)[] = [];

  constructor(cameraService: CameraService) {
    this.cameraService = cameraService;
    this.modelService = ModelService.getInstance();
    this.poseInspectionService = PoseInspectionService.getInstance();
    this.contourService = ContourService.getInstance();
    this.alignmentService = AlignmentService.getInstance();
    this.positioningValidator = PositioningValidator.getInstance();
  }

  // Initialize body scan with user profile
  async initialize(profile: UserProfile): Promise<void> {
    console.log('BodyScanService.initialize called with profile:', profile);
    this.userProfile = profile;
    this.currentPhase = BodyScanPhase.SETUP;
    this.notifyPhaseChange();
    
    // Load ML models if not already loaded
    try {
      console.log('Loading ML models...');
      await this.modelService.loadModels();
      console.log('ML models loaded successfully');
      
      // Initialize alignment service
      if (this.alignmentService.isSupported()) {
        try {
          await this.alignmentService.startMonitoring();
          console.log('Alignment monitoring started');
        } catch (err) {
          console.warn('Alignment monitoring not available:', err);
        }
      }
      
      // Mark as initialized and move to FRONT_CAPTURE phase
      this._isInitialized = true;
      this.currentPhase = BodyScanPhase.FRONT_CAPTURE;
      this.notifyPhaseChange();
    } catch (error) {
      console.error('Failed to load models:', error);
      this._isInitialized = false;
      this.currentPhase = BodyScanPhase.ERROR;
      this.notifyPhaseChange();
      throw error;
    }
  }

  // Start body scan capture
  async startCapture(pose: BodyScanPose): Promise<void> {
    console.log('BodyScanService.startCapture called for pose:', pose);
    
    if (!this.userProfile) {
      throw new Error('User profile not set');
    }

    // Only change phase if needed
    const targetPhase = pose === BodyScanPose.FRONT 
      ? BodyScanPhase.FRONT_CAPTURE 
      : BodyScanPhase.SIDE_CAPTURE;
    
    if (this.currentPhase !== targetPhase) {
      this.currentPhase = targetPhase;
      this.notifyPhaseChange();
    }

    try {
      // Generate contour zones for current pose (at standard dimensions)
      this.contourZones = this.contourService.generateOptimalZones({
        width: 720, // Standard width
        height: 1280, // Standard height
        profile: pose === BodyScanPose.FRONT ? 'front' : 'side',
        gender: this.userProfile.gender
      });
      
      // Start camera if not already started
      console.log('Starting camera...');
      await this.cameraService.startCamera();
      console.log('Camera started successfully, state:', this.cameraService.state);
      
      // Wait for camera to stabilize and video element to be ready
      await new Promise(resolve => setTimeout(resolve, 1000));
      
      // Contour zones are already generated at standard dimensions
      console.log('Contour zones generated:', this.contourZones);
      
      // Start pose detection if not already running
      if (!this._isPoseDetectionRunning) {
        console.log('Starting pose detection...');
        this.startPoseDetection();
      } else {
        console.log('Pose detection already running');
      }
    } catch (error) {
      console.error('Failed to start capture:', error);
      this.currentPhase = BodyScanPhase.ERROR;
      this.notifyPhaseChange();
      throw error;
    }
  }

  // Capture current frame for the specified pose
  async captureImage(pose: BodyScanPose): Promise<string> {
    const imageData = await this.cameraService.takePicture();
    if (!imageData) {
      throw new Error('Failed to capture image');
    }

    if (pose === BodyScanPose.FRONT) {
      this.capturedImages.front = imageData;
    } else {
      this.capturedImages.side = imageData;
    }

    return imageData;
  }

  // Process captured images and generate measurements
  async processImages(): Promise<BodyScanResult> {
    if (!this.capturedImages.front || !this.capturedImages.side) {
      throw new Error('Both front and side images required');
    }

    if (!this.userProfile) {
      throw new Error('User profile not set');
    }

    this.currentPhase = BodyScanPhase.PROCESSING;
    this.notifyPhaseChange();

    try {
      // Process front image
      const frontImage = await this.loadImage(this.capturedImages.front);
      const frontPoses = await this.modelService.detectPose(frontImage);
      const frontSegmentation = await this.modelService.segmentPerson(frontImage);

      // Process side image
      const sideImage = await this.loadImage(this.capturedImages.side);
      const sidePoses = await this.modelService.detectPose(sideImage);
      const sideSegmentation = await this.modelService.segmentPerson(sideImage);

      // Calculate measurements using both views
      const measurements = await this.modelService.calculateMeasurements(
        frontPoses,
        frontSegmentation,
        this.userProfile
      );

      const result: BodyScanResult = {
        measurements,
        frontImage: this.capturedImages.front,
        sideImage: this.capturedImages.side,
        confidence: this.calculateConfidence(frontPoses, sidePoses),
        timestamp: new Date()
      };

      this.currentPhase = BodyScanPhase.COMPLETE;
      this.notifyPhaseChange();

      return result;
    } catch (error) {
      this.currentPhase = BodyScanPhase.ERROR;
      this.notifyPhaseChange();
      throw error;
    }
  }

  private async loadImage(dataUrl: string): Promise<HTMLImageElement> {
    return new Promise((resolve, reject) => {
      const img = new Image();
      img.onload = () => resolve(img);
      img.onerror = reject;
      img.src = dataUrl;
    });
  }

  private calculateConfidence(frontPoses: poseDetection.Pose[], sidePoses: poseDetection.Pose[]): number {
    if (frontPoses.length === 0 || sidePoses.length === 0) return 0;

    const frontConfidence = frontPoses[0].score || 0;
    const sideConfidence = sidePoses[0].score || 0;
    
    return (frontConfidence + sideConfidence) / 2;
  }

  // Reset scan state
  reset(): void {
    this.currentPhase = BodyScanPhase.IDLE;
    this.userProfile = null;
    this.capturedImages = {};
    this._isInitialized = false;
    this._isPoseDetectionRunning = false;
    this.contourZones = null;
    
    // Stop frame processing
    if (this.frameProcessingTimer) {
      clearInterval(this.frameProcessingTimer);
      this.frameProcessingTimer = null;
    }
    
    // Stop alignment monitoring
    this.alignmentService.stopMonitoring();
    
    this.cameraService.stopCamera();
    this.notifyPhaseChange();
  }

  // Pose detection
  private startPoseDetection(): void {
    // Mark pose detection as running
    this._isPoseDetectionRunning = true;
    
    let frameCount = 0;
    
    // Process frames at 10 FPS for performance
    this.frameProcessingTimer = setInterval(async () => {
      try {
        const videoElement = this.cameraService.videoElement;
        if (!videoElement) {
          console.log('Waiting for video element...');
          return;
        }
        
        // Check if video is ready
        if (videoElement.readyState < 2) {
          console.log('Video not ready, state:', videoElement.readyState);
          return;
        }
        
        frameCount++;
        if (frameCount % 30 === 0) { // Log every 3 seconds
          console.log('Processing frame', frameCount, 'Video dimensions:', {
            width: videoElement.videoWidth,
            height: videoElement.videoHeight
          });
        }

        const result = await this.detectPose(videoElement);
        this.notifyPoseDetection(result);
      } catch (error) {
        console.error('Pose detection error:', error);
      }
    }, 100); // 10 FPS
  }

  private async detectPose(videoElement: HTMLVideoElement): Promise<PoseDetectionResult> {
    try {
      // Log video dimensions periodically for debugging
      if (Math.random() < 0.1) { // Log 10% of the time to avoid spam
        console.log('Detecting pose on video element:', {
          width: videoElement.videoWidth,
          height: videoElement.videoHeight,
          readyState: videoElement.readyState
        });
      }
      
      // Detect poses using the model
      const poses = await this.modelService.detectPose(videoElement);
      
      if (poses.length === 0) {
        console.log('No poses detected');
        return {
          joints: [],
          isValid: false,
          feedback: ['Step into camera view', 'Stand 6-8 feet from camera']
        };
      }

      const pose = poses[0];
      const keypoints = pose.keypoints;
      
      // Convert keypoints to our joint format
      // Check if coordinates need normalization
      const needsNormalization = keypoints.some(kp => kp.x > 1 || kp.y > 1);
      const videoWidth = videoElement.videoWidth || videoElement.width || 1;
      const videoHeight = videoElement.videoHeight || videoElement.height || 1;
      
      const joints: JointPoint[] = keypoints.map(kp => {
        let x = kp.x;
        let y = kp.y;
        
        // Normalize if needed
        if (needsNormalization) {
          x = x / videoWidth;
          y = y / videoHeight;
        }
        
        // Ensure values are between 0 and 1
        x = Math.max(0, Math.min(1, x));
        y = Math.max(0, Math.min(1, y));
        
        // Check for NaN or Infinity
        if (!isFinite(x)) x = 0;
        if (!isFinite(y)) y = 0;
        
        return {
          x,
          y,
          confidence: kp.score || 0
        };
      });

      // Get current pose profile
      const profile = this.currentPhase === BodyScanPhase.FRONT_CAPTURE ? 'front' : 'side';
      
      // Perform advanced pose inspection
      let inspectionResult = null;
      let advancedFeedback: string[] = [];
      if (this.contourZones) {
        inspectionResult = this.poseInspectionService.inspect(
          poses,
          this.contourZones,
          profile,
          videoElement.width,
          videoElement.height
        );
        
        // Get feedback from inspection
        advancedFeedback = this.poseInspectionService.getFeedback(inspectionResult, profile);
      }
      
      // Convert joints to format expected by positioning validator
      const detectedJoints: any = {};
      keypoints.forEach(kp => {
        // Use actual pixel coordinates (not normalized)
        let x = kp.x;
        let y = kp.y;
        
        // If coordinates are normalized (0-1), convert to pixel coordinates
        if (x <= 1.0 && y <= 1.0) {
          x = x * (videoElement.videoWidth || 1);
          y = y * (videoElement.videoHeight || 1);
        }
        
        detectedJoints[kp.name || ''] = {
          x,
          y,
          confidence: kp.score || 0
        };
      });
      
      // Add CentroidHeadTop for head detection (approximate from nose position)
      if (detectedJoints.nose && !detectedJoints.CentroidHeadTop) {
        detectedJoints.CentroidHeadTop = {
          x: detectedJoints.nose.x,
          y: detectedJoints.nose.y - 30, // Approximate head top as 30px above nose
          confidence: detectedJoints.nose.confidence
        };
      }
      
      // Add ankle centroids if missing
      if (detectedJoints.left_ankle && !detectedJoints.CentroidLeftAnkle) {
        detectedJoints.CentroidLeftAnkle = detectedJoints.left_ankle;
      }
      if (detectedJoints.right_ankle && !detectedJoints.CentroidRightAnkle) {
        detectedJoints.CentroidRightAnkle = detectedJoints.right_ankle;
      }
      
      // Use positioning validator for zone-based feedback
      let positioningFeedback: string[] = [];
      let isPositionValid = false;
      
      if (inspectionResult && this.contourZones) {
        // Log coordinate system debug info periodically
        if (Math.random() < 0.05) { // 5% of frames
          console.log('Coordinate system debug:', {
            videoWidth: videoElement.videoWidth,
            videoHeight: videoElement.videoHeight,
            headJoint: detectedJoints.CentroidHeadTop || detectedJoints.nose,
            headZone: this.contourZones.head,
            leftAnkle: detectedJoints.left_ankle,
            rightAnkle: detectedJoints.right_ankle,
            ankleZone: this.contourZones.legs
          });
        }
        
        const positioningResult = this.positioningValidator.validatePositioning(
          inspectionResult,
          detectedJoints,
          this.contourZones,
          videoElement.videoWidth || window.innerWidth,
          videoElement.videoHeight || window.innerHeight
        );
        
        positioningFeedback = positioningResult.feedback;
        isPositionValid = positioningResult.positioningMessage === PositioningMessage.PERFECT_SPOT;
      }
      
      // Check if pose is valid according to inspection
      const isInContour = inspectionResult 
        ? this.poseInspectionService.isInContour(profile, inspectionResult)
        : false;
      
      // Get alignment score
      const alignmentResult = this.alignmentService.getAlignment();
      const alignmentScore = this.alignmentService.getAlignmentScore();
      
      // Combine feedback - prioritize positioning feedback
      const feedback: string[] = [];
      
      // Add positioning feedback first
      if (positioningFeedback.length > 0) {
        feedback.push(...positioningFeedback);
      }
      
      // Add alignment feedback if no positioning issues
      if (!alignmentResult.isAligned && positioningFeedback.length === 0) {
        feedback.push(...alignmentResult.feedback);
      }
      
      // Add pose-specific feedback (arms, etc.) if position is good
      if (advancedFeedback.length > 0 && isPositionValid) {
        feedback.push(...advancedFeedback);
      }
      
      // Pose is valid if position, inspection and alignment all pass
      const isValid = isPositionValid && isInContour && alignmentResult.isAligned;
      
      // Add initial guidance if pose is valid but waiting for capture
      if (isValid && feedback.length === 0) {
        feedback.push('Great! Hold still for 3 seconds');
      }
      
      return {
        joints,
        isValid,
        feedback: feedback.length > 0 ? feedback : ['Position yourself in the frame'],
        inspectionResult,
        alignmentScore
      };
    } catch (error) {
      console.error('Pose detection failed:', error);
      return {
        joints: [],
        isValid: false,
        feedback: ['Pose detection failed. Please try again.']
      };
    }
  }

  // Enhanced capture with quality validation
  async captureImageWithValidation(pose: BodyScanPose): Promise<string> {
    // Ensure we have good pose detection before capture
    const videoElement = this.cameraService.videoElement;
    if (!videoElement) {
      throw new Error('Camera not ready');
    }
    
    // Get current pose detection result
    const poseResult = await this.detectPose(videoElement);
    if (!poseResult.isValid) {
      throw new Error('Invalid pose for capture');
    }
    
    // Check alignment is good
    const alignmentScore = poseResult.alignmentScore || 0;
    if (alignmentScore < 80) {
      throw new Error('Device not properly aligned');
    }
    
    // Capture the image
    return this.captureImage(pose);
  }
  
  // Get contour generation data
  getContourData() {
    const videoElement = this.cameraService.videoElement;
    if (!videoElement || !this.userProfile) {
      console.log('Cannot get contour data - missing video element or user profile');
      return null;
    }
    
    const profile = this.currentPhase === BodyScanPhase.FRONT_CAPTURE ? 'front' : 'side';
    const displayWidth = videoElement.videoWidth || window.innerWidth;
    const displayHeight = videoElement.videoHeight || window.innerHeight;
    
    console.log('Generating contour data:', { profile, displayWidth, displayHeight });
    
    // Generate contour points at standard dimensions
    const contourPoints = this.contourService.generateContour({
      width: 720, // Standard width (not used anymore, but kept for interface)
      height: 1280, // Standard height (not used anymore, but kept for interface)
      profile,
      gender: this.userProfile.gender
    });
    
    // Smooth the contour for better visual appearance
    const smoothedPoints = this.contourService.smoothContour(contourPoints);
    
    // Scale points to match actual display dimensions
    const scaledPoints = this.contourService.scaleContourPoints(
      smoothedPoints,
      displayWidth,
      displayHeight
    );
    
    return {
      points: scaledPoints,
      zones: this.contourZones,
      displayDimensions: {
        width: displayWidth,
        height: displayHeight
      }
    };
  }

  // Event handlers
  onPhaseChange(callback: (phase: BodyScanPhase) => void): void {
    this.phaseChangeCallbacks.push(callback);
  }

  onPoseDetection(callback: (result: PoseDetectionResult) => void): void {
    this.poseDetectionCallbacks.push(callback);
  }

  private notifyPhaseChange(): void {
    this.phaseChangeCallbacks.forEach(cb => cb(this.currentPhase));
  }

  private notifyPoseDetection(result: PoseDetectionResult): void {
    this.poseDetectionCallbacks.forEach(cb => cb(result));
  }

  // Getters
  get phase(): BodyScanPhase {
    return this.currentPhase;
  }

  get profile(): UserProfile | null {
    return this.userProfile;
  }

  get camera(): CameraService {
    return this.cameraService;
  }
  
  get isInitialized(): boolean {
    return this._isInitialized;
  }
}