import * as tf from '@tensorflow/tfjs';
import * as poseDetection from '@tensorflow-models/pose-detection';
import * as bodyPix from '@tensorflow-models/body-pix';

export interface ModelLoadProgress {
  modelName: string;
  progress: number;
  loaded: boolean;
}

export class ModelService {
  private static instance: ModelService;
  private poseDetector: poseDetection.PoseDetector | null = null;
  private segmentationModel: bodyPix.BodyPix | null = null;
  private loadingCallbacks: ((progress: ModelLoadProgress) => void)[] = [];

  private constructor() {}

  static getInstance(): ModelService {
    if (!ModelService.instance) {
      ModelService.instance = new ModelService();
    }
    return ModelService.instance;
  }

  // Load all required models
  async loadModels(): Promise<void> {
    // Initialize TensorFlow.js backend first
    try {
      console.log('Initializing TensorFlow.js...');
      await tf.ready();
      console.log('TensorFlow.js initialized with backend:', tf.getBackend());
    } catch (error) {
      console.error('Failed to initialize TensorFlow.js:', error);
      throw new Error(`TensorFlow.js initialization failed: ${error instanceof Error ? error.message : 'Unknown error'}`);
    }

    // Now load the models with timeout
    const timeout = 30000; // 30 seconds timeout
    const loadPromise = Promise.all([
      this.loadPoseDetectionModel(),
      this.loadSegmentationModel()
    ]);
    
    const timeoutPromise = new Promise<never>((_, reject) => {
      setTimeout(() => reject(new Error('Model loading timed out after 30 seconds')), timeout);
    });
    
    try {
      await Promise.race([loadPromise, timeoutPromise]);
      console.log('All models loaded successfully');
    } catch (error) {
      console.error('Failed to load models:', error);
      throw error;
    }
  }

  // Load pose detection model (MoveNet)
  private async loadPoseDetectionModel(): Promise<void> {
    try {
      this.notifyProgress({ modelName: 'pose_detection', progress: 0, loaded: false });

      // Use MoveNet for better performance
      const detectorConfig: poseDetection.MoveNetModelConfig = {
        modelType: poseDetection.movenet.modelType.SINGLEPOSE_LIGHTNING,
        enableSmoothing: true,
        minPoseScore: 0.25
      };

      this.poseDetector = await poseDetection.createDetector(
        poseDetection.SupportedModels.MoveNet,
        detectorConfig
      );

      this.notifyProgress({ modelName: 'pose_detection', progress: 100, loaded: true });
      console.log('Pose detection model loaded');
    } catch (error) {
      console.error('Failed to load pose detection model:', error);
      this.notifyProgress({ modelName: 'pose_detection', progress: 0, loaded: false });
      throw new Error(`Pose detection model failed to load: ${error instanceof Error ? error.message : 'Unknown error'}`);
    }
  }

  // Load segmentation model (BodyPix)
  private async loadSegmentationModel(): Promise<void> {
    try {
      this.notifyProgress({ modelName: 'segmentation', progress: 0, loaded: false });

      // Load BodyPix for person segmentation
      this.segmentationModel = await bodyPix.load({
        architecture: 'MobileNetV1',
        outputStride: 16,
        multiplier: 0.75,
        quantBytes: 2
      });

      this.notifyProgress({ modelName: 'segmentation', progress: 100, loaded: true });
      console.log('Segmentation model loaded');
    } catch (error) {
      console.error('Failed to load segmentation model:', error);
      this.notifyProgress({ modelName: 'segmentation', progress: 0, loaded: false });
      throw new Error(`Segmentation model failed to load: ${error instanceof Error ? error.message : 'Unknown error'}`);
    }
  }

  // Detect poses in an image
  async detectPose(
    imageElement: HTMLImageElement | HTMLVideoElement | HTMLCanvasElement
  ): Promise<poseDetection.Pose[]> {
    if (!this.poseDetector) {
      throw new Error('Pose detection model not loaded');
    }

    const poses = await this.poseDetector.estimatePoses(imageElement);
    return poses;
  }

  // Segment person from background
  async segmentPerson(
    imageElement: HTMLImageElement | HTMLVideoElement | HTMLCanvasElement
  ): Promise<bodyPix.SemanticPersonSegmentation> {
    if (!this.segmentationModel) {
      throw new Error('Segmentation model not loaded');
    }

    const segmentation = await this.segmentationModel.segmentPerson(imageElement, {
      flipHorizontal: false,
      internalResolution: 'medium',
      segmentationThreshold: 0.7
    });

    return segmentation;
  }

  // Extract person mask as image data
  async extractPersonMask(
    imageElement: HTMLImageElement | HTMLVideoElement | HTMLCanvasElement,
    segmentation: bodyPix.SemanticPersonSegmentation
  ): Promise<ImageData> {
    const canvas = document.createElement('canvas');
    canvas.width = segmentation.width;
    canvas.height = segmentation.height;
    const ctx = canvas.getContext('2d')!;

    // Draw original image
    ctx.drawImage(imageElement, 0, 0, canvas.width, canvas.height);
    const imageData = ctx.getImageData(0, 0, canvas.width, canvas.height);

    // Apply mask
    const pixelData = imageData.data;
    for (let i = 0; i < segmentation.data.length; i++) {
      if (segmentation.data[i] === 0) {
        // Background pixel - make transparent
        const pixelIndex = i * 4;
        pixelData[pixelIndex + 3] = 0; // Alpha channel
      }
    }

    return imageData;
  }

  // Calculate measurements from pose and segmentation
  async calculateMeasurements(
    poses: poseDetection.Pose[],
    segmentation: bodyPix.SemanticPersonSegmentation,
    userProfile: { height: number; weight: number; gender: 'male' | 'female' }
  ): Promise<any> {
    // This is a placeholder for the actual measurement calculation
    // In production, this would use the SVR models or a custom trained model

    if (poses.length === 0) {
      throw new Error('No pose detected');
    }

    const pose = poses[0];
    const keypoints = pose.keypoints;

    // Extract relevant keypoints
    const shoulders = this.getKeypointPair(keypoints, 'left_shoulder', 'right_shoulder');
    const hips = this.getKeypointPair(keypoints, 'left_hip', 'right_hip');
    const knees = this.getKeypointPair(keypoints, 'left_knee', 'right_knee');
    const ankles = this.getKeypointPair(keypoints, 'left_ankle', 'right_ankle');

    // Calculate pixel-based measurements
    const shoulderWidth = this.calculateDistance(shoulders.left, shoulders.right);
    const hipWidth = this.calculateDistance(hips.left, hips.right);
    const torsoLength = this.calculateDistance(shoulders.center, hips.center);
    const legLength = this.calculateDistance(hips.center, ankles.center);

    // Convert to real measurements using height as reference
    const pixelHeight = this.calculateDistance(
      { x: keypoints[0].x, y: keypoints[0].y }, // Approximate head position
      ankles.center
    );
    const pixelToRealRatio = userProfile.height / pixelHeight;

    // Calculate body measurements using profile data
    // Note: These are still approximations - a real implementation would use trained ML models
    const measurements = {
      chest: Math.round(shoulderWidth * pixelToRealRatio * 1.2), // Rough approximation
      waist: Math.round(hipWidth * pixelToRealRatio * 0.9),
      hip: Math.round(hipWidth * pixelToRealRatio * 1.1),
      thigh: Math.round(hipWidth * pixelToRealRatio * 0.6),
      inseam: Math.round(legLength * pixelToRealRatio * 0.8),
      
      // Body composition calculations based on user profile
      // Using simplified formulas - real implementation would use SVR models
      bodyFat: this.calculateBodyFat(userProfile),
      muscleMass: this.calculateMuscleMass(userProfile),
      visceralFat: this.calculateVisceralFat(userProfile),
      weight: userProfile.weight,
      metabolicAge: this.calculateMetabolicAge(userProfile, this.calculateBodyFat(userProfile)),
      bodyType: this.calculateBodyType(hipWidth * pixelToRealRatio, shoulderWidth * pixelToRealRatio, userProfile)
    };

    return measurements;
  }

  // Helper methods
  private getKeypointPair(
    keypoints: poseDetection.Keypoint[],
    leftName: string,
    rightName: string
  ) {
    const left = keypoints.find(kp => kp.name === leftName)!;
    const right = keypoints.find(kp => kp.name === rightName)!;
    return {
      left,
      right,
      center: {
        x: (left.x + right.x) / 2,
        y: (left.y + right.y) / 2
      }
    };
  }

  private calculateDistance(p1: { x: number; y: number }, p2: { x: number; y: number }): number {
    return Math.sqrt(Math.pow(p2.x - p1.x, 2) + Math.pow(p2.y - p1.y, 2));
  }

  // Progress notification
  onLoadProgress(callback: (progress: ModelLoadProgress) => void): void {
    this.loadingCallbacks.push(callback);
  }

  private notifyProgress(progress: ModelLoadProgress): void {
    this.loadingCallbacks.forEach(cb => cb(progress));
  }

  // Cleanup
  // Body composition calculation helpers
  private calculateBodyFat(profile: { height: number; weight: number; gender: 'male' | 'female' }): number {
    // Simplified body fat calculation based on BMI and gender
    const bmi = profile.weight / Math.pow(profile.height / 100, 2);
    
    if (profile.gender === 'male') {
      // Male body fat estimation
      if (bmi < 20) return 8 + Math.random() * 4;
      if (bmi < 25) return 12 + Math.random() * 6;
      if (bmi < 30) return 18 + Math.random() * 7;
      return 25 + Math.random() * 10;
    } else {
      // Female body fat estimation (typically higher)
      if (bmi < 20) return 15 + Math.random() * 5;
      if (bmi < 25) return 20 + Math.random() * 6;
      if (bmi < 30) return 28 + Math.random() * 7;
      return 35 + Math.random() * 10;
    }
  }
  
  private calculateMuscleMass(profile: { height: number; weight: number; gender: 'male' | 'female' }): number {
    // Muscle mass estimation based on weight and gender
    const baseMuscle = profile.weight * (profile.gender === 'male' ? 0.45 : 0.36);
    return Math.round(baseMuscle + (Math.random() - 0.5) * 5);
  }
  
  private calculateVisceralFat(profile: { height: number; weight: number; gender: 'male' | 'female' }): number {
    // Visceral fat level (1-12 scale typically)
    const bmi = profile.weight / Math.pow(profile.height / 100, 2);
    
    if (bmi < 25) return Math.floor(3 + Math.random() * 3);
    if (bmi < 30) return Math.floor(6 + Math.random() * 3);
    return Math.floor(9 + Math.random() * 3);
  }

  private calculateMetabolicAge(profile: { age?: number; height: number; weight: number; gender: 'male' | 'female' }, bodyFat: number): number {
    // Calculate metabolic age based on body composition
    const chronologicalAge = profile.age || 30;
    const bmi = profile.weight / Math.pow(profile.height / 100, 2);
    
    // Base metabolic age on chronological age
    let metabolicAge = chronologicalAge;
    
    // Adjust based on BMI
    if (bmi < 18.5) {
      metabolicAge += 3; // Underweight
    } else if (bmi >= 25 && bmi < 30) {
      metabolicAge += 5; // Overweight
    } else if (bmi >= 30) {
      metabolicAge += 10; // Obese
    }
    
    // Adjust based on body fat percentage
    const idealBodyFat = profile.gender === 'male' ? 18 : 25;
    const bodyFatDiff = bodyFat - idealBodyFat;
    metabolicAge += Math.floor(bodyFatDiff / 3);
    
    // Add some random variation
    metabolicAge += Math.floor((Math.random() - 0.5) * 4);
    
    // Ensure metabolic age is reasonable
    return Math.max(18, Math.min(80, Math.round(metabolicAge)));
  }

  private calculateBodyType(hip: number, shoulder: number, profile: { height: number; weight: number }): string {
    // Calculate body type based on measurements
    const shoulderToHipRatio = shoulder / hip;
    const bmi = profile.weight / Math.pow(profile.height / 100, 2);
    
    if (shoulderToHipRatio < 1.2 && bmi < 22) {
      return 'Ectomorph'; // Lean, narrow frame
    } else if (shoulderToHipRatio > 1.4 && bmi > 24) {
      return 'Mesomorph'; // Athletic, muscular
    } else if (shoulderToHipRatio < 1.3 && bmi > 25) {
      return 'Endomorph'; // Rounder, stores fat easily
    } else {
      // Mixed types
      if (shoulderToHipRatio > 1.35) {
        return 'Meso-Ectomorph';
      } else if (bmi > 23) {
        return 'Endo-Mesomorph';
      } else {
        return 'Ecto-Mesomorph';
      }
    }
  }

  dispose(): void {
    if (this.poseDetector) {
      this.poseDetector.dispose();
      this.poseDetector = null;
    }
    if (this.segmentationModel) {
      this.segmentationModel.dispose();
      this.segmentationModel = null;
    }
  }
}