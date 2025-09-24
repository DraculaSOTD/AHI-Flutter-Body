import * as poseDetection from '@tensorflow-models/pose-detection';

// Pose inspection result types matching iOS SDK
export enum PoseInspectionResult {
  UNKNOWN = 'unknown',
  TRUE_IN_CONTOUR = 'true_in_contour',
  FALSE_NOT_IN_CONTOUR = 'false_not_in_contour',
  FALSE_NOT_DETECTED = 'false_not_detected',
  FALSE_MULTIPLE_FACES = 'false_multiple_faces'
}

export interface PoseInspectionDictionary {
  face: PoseInspectionResult;
  la?: PoseInspectionResult; // Left Arm (front only)
  ra?: PoseInspectionResult; // Right Arm (front only)
  ll?: PoseInspectionResult; // Left Leg/Ankle (front only)
  rl?: PoseInspectionResult; // Right Leg/Ankle (front only)
  ub?: PoseInspectionResult; // Upper Body (side only)
  lb?: PoseInspectionResult; // Lower Body (side only)
}

export interface OptimalZone {
  x: number;
  y: number;
  width: number;
  height: number;
}

export interface ContourZones {
  head: OptimalZone;
  torso: OptimalZone;
  legs: OptimalZone;
  arms?: OptimalZone;
}

export type Profile = 'front' | 'side';

export class PoseInspectionService {
  private static instance: PoseInspectionService;
  
  // Confidence thresholds
  private readonly MIN_JOINT_CONFIDENCE = 0.3;
  private readonly MIN_FACE_CONFIDENCE = 0.5;
  private readonly OPTIMAL_JOINT_CONFIDENCE = 0.7;
  
  // Position thresholds
  private readonly ARM_AWAY_FROM_BODY_THRESHOLD = 0.1; // 10% of body width
  private readonly ANKLE_VISIBILITY_MARGIN = 0.05; // 5% margin from bottom
  private readonly SIDE_POSE_ALIGNMENT_THRESHOLD = 0.15; // 15% for side alignment
  
  private constructor() {}
  
  static getInstance(): PoseInspectionService {
    if (!PoseInspectionService.instance) {
      PoseInspectionService.instance = new PoseInspectionService();
    }
    return PoseInspectionService.instance;
  }
  
  /**
   * Inspect pose for validity based on profile type
   */
  inspect(
    poses: poseDetection.Pose[],
    contourZones: ContourZones,
    profile: Profile,
    imageWidth: number,
    imageHeight: number
  ): PoseInspectionDictionary {
    // Check for multiple people
    if (poses.length > 1) {
      return this.createFailedInspection(profile, PoseInspectionResult.FALSE_MULTIPLE_FACES);
    }
    
    if (poses.length === 0) {
      return this.createFailedInspection(profile, PoseInspectionResult.FALSE_NOT_DETECTED);
    }
    
    const pose = poses[0];
    const keypoints = pose.keypoints;
    
    if (profile === 'front') {
      return this.inspectFrontPose(keypoints, contourZones, imageWidth, imageHeight);
    } else {
      return this.inspectSidePose(keypoints, contourZones, imageWidth, imageHeight);
    }
  }
  
  /**
   * Check if inspection result is acceptable for proceeding
   */
  isInContour(profile: Profile, inspectionResult: PoseInspectionDictionary): boolean {
    const requiredChecks = profile === 'front' 
      ? ['face', 'la', 'ra', 'll', 'rl']
      : ['face', 'ub', 'lb'];
    
    return requiredChecks.every(check => {
      const result = inspectionResult[check as keyof PoseInspectionDictionary];
      return result === PoseInspectionResult.TRUE_IN_CONTOUR;
    });
  }
  
  /**
   * Get human-readable feedback for pose issues
   */
  getFeedback(inspectionResult: PoseInspectionDictionary, profile: Profile): string[] {
    const feedback: string[] = [];
    
    // Only provide feedback for specific pose issues, not positioning
    if (profile === 'front') {
      // Check arms only if they're detected but not in correct position
      if (inspectionResult.la === PoseInspectionResult.FALSE_NOT_IN_CONTOUR) {
        feedback.push('Move left arm away from body');
      }
      if (inspectionResult.ra === PoseInspectionResult.FALSE_NOT_IN_CONTOUR) {
        feedback.push('Move right arm away from body');
      }
    } else {
      // Side pose checks
      if (inspectionResult.ub === PoseInspectionResult.FALSE_NOT_IN_CONTOUR) {
        feedback.push('Keep arms at your side');
      }
      if (inspectionResult.lb === PoseInspectionResult.FALSE_NOT_IN_CONTOUR) {
        feedback.push('Keep feet together');
      }
    }
    
    return feedback;
  }
  
  private inspectFrontPose(
    keypoints: poseDetection.Keypoint[],
    contourZones: ContourZones,
    imageWidth: number,
    imageHeight: number
  ): PoseInspectionDictionary {
    const result: PoseInspectionDictionary = {
      face: this.inspectFace(keypoints, contourZones.head),
      la: this.inspectLeftArm(keypoints, imageWidth),
      ra: this.inspectRightArm(keypoints, imageWidth),
      ll: this.inspectLeftAnkle(keypoints, imageHeight),
      rl: this.inspectRightAnkle(keypoints, imageHeight)
    };
    
    return result;
  }
  
  private inspectSidePose(
    keypoints: poseDetection.Keypoint[],
    contourZones: ContourZones,
    imageWidth: number,
    imageHeight: number
  ): PoseInspectionDictionary {
    const result: PoseInspectionDictionary = {
      face: this.inspectFace(keypoints, contourZones.head),
      ub: this.inspectUpperBodySide(keypoints, imageWidth),
      lb: this.inspectLowerBodySide(keypoints, imageWidth)
    };
    
    return result;
  }
  
  private inspectFace(keypoints: poseDetection.Keypoint[], headZone: OptimalZone): PoseInspectionResult {
    const nose = keypoints.find(kp => kp.name === 'nose');
    const leftEye = keypoints.find(kp => kp.name === 'left_eye');
    const rightEye = keypoints.find(kp => kp.name === 'right_eye');
    
    // Check if face keypoints are detected
    if (!nose || (nose.score || 0) < this.MIN_FACE_CONFIDENCE) {
      return PoseInspectionResult.FALSE_NOT_DETECTED;
    }
    
    // Note: The keypoints and zones are expected to be in the same coordinate system
    // The zone scaling happens in PositioningValidator, not here
    // For face inspection, we just check if the keypoint exists with good confidence
    return PoseInspectionResult.TRUE_IN_CONTOUR;
  }
  
  private inspectLeftArm(keypoints: poseDetection.Keypoint[], imageWidth: number): PoseInspectionResult {
    const leftShoulder = keypoints.find(kp => kp.name === 'left_shoulder');
    const leftElbow = keypoints.find(kp => kp.name === 'left_elbow');
    const leftWrist = keypoints.find(kp => kp.name === 'left_wrist');
    const leftHip = keypoints.find(kp => kp.name === 'left_hip');
    
    if (!leftShoulder || !leftElbow || !leftWrist || !leftHip) {
      return PoseInspectionResult.FALSE_NOT_DETECTED;
    }
    
    if ((leftShoulder.score || 0) < this.MIN_JOINT_CONFIDENCE ||
        (leftElbow.score || 0) < this.MIN_JOINT_CONFIDENCE ||
        (leftWrist.score || 0) < this.MIN_JOINT_CONFIDENCE) {
      return PoseInspectionResult.FALSE_NOT_DETECTED;
    }
    
    // Check if arm is away from body
    const shoulderHipDistance = Math.abs(leftShoulder.x - leftHip.x);
    const wristHipDistance = Math.abs(leftWrist.x - leftHip.x);
    const threshold = imageWidth * this.ARM_AWAY_FROM_BODY_THRESHOLD;
    
    if (wristHipDistance > threshold && wristHipDistance > shoulderHipDistance) {
      return PoseInspectionResult.TRUE_IN_CONTOUR;
    }
    
    return PoseInspectionResult.FALSE_NOT_IN_CONTOUR;
  }
  
  private inspectRightArm(keypoints: poseDetection.Keypoint[], imageWidth: number): PoseInspectionResult {
    const rightShoulder = keypoints.find(kp => kp.name === 'right_shoulder');
    const rightElbow = keypoints.find(kp => kp.name === 'right_elbow');
    const rightWrist = keypoints.find(kp => kp.name === 'right_wrist');
    const rightHip = keypoints.find(kp => kp.name === 'right_hip');
    
    if (!rightShoulder || !rightElbow || !rightWrist || !rightHip) {
      return PoseInspectionResult.FALSE_NOT_DETECTED;
    }
    
    if ((rightShoulder.score || 0) < this.MIN_JOINT_CONFIDENCE ||
        (rightElbow.score || 0) < this.MIN_JOINT_CONFIDENCE ||
        (rightWrist.score || 0) < this.MIN_JOINT_CONFIDENCE) {
      return PoseInspectionResult.FALSE_NOT_DETECTED;
    }
    
    // Check if arm is away from body
    const shoulderHipDistance = Math.abs(rightShoulder.x - rightHip.x);
    const wristHipDistance = Math.abs(rightWrist.x - rightHip.x);
    const threshold = imageWidth * this.ARM_AWAY_FROM_BODY_THRESHOLD;
    
    if (wristHipDistance > threshold && wristHipDistance > shoulderHipDistance) {
      return PoseInspectionResult.TRUE_IN_CONTOUR;
    }
    
    return PoseInspectionResult.FALSE_NOT_IN_CONTOUR;
  }
  
  private inspectLeftAnkle(keypoints: poseDetection.Keypoint[], imageHeight: number): PoseInspectionResult {
    const leftAnkle = keypoints.find(kp => kp.name === 'left_ankle');
    
    if (!leftAnkle || (leftAnkle.score || 0) < this.MIN_JOINT_CONFIDENCE) {
      return PoseInspectionResult.FALSE_NOT_DETECTED;
    }
    
    // Zone-based positioning is handled by PositioningValidator
    // Here we just check if the ankle is detected with good confidence
    return PoseInspectionResult.TRUE_IN_CONTOUR;
  }
  
  private inspectRightAnkle(keypoints: poseDetection.Keypoint[], imageHeight: number): PoseInspectionResult {
    const rightAnkle = keypoints.find(kp => kp.name === 'right_ankle');
    
    if (!rightAnkle || (rightAnkle.score || 0) < this.MIN_JOINT_CONFIDENCE) {
      return PoseInspectionResult.FALSE_NOT_DETECTED;
    }
    
    // Zone-based positioning is handled by PositioningValidator
    // Here we just check if the ankle is detected with good confidence
    return PoseInspectionResult.TRUE_IN_CONTOUR;
  }
  
  private inspectUpperBodySide(keypoints: poseDetection.Keypoint[], imageWidth: number): PoseInspectionResult {
    const leftWrist = keypoints.find(kp => kp.name === 'left_wrist');
    const rightWrist = keypoints.find(kp => kp.name === 'right_wrist');
    const leftHip = keypoints.find(kp => kp.name === 'left_hip');
    const rightHip = keypoints.find(kp => kp.name === 'right_hip');
    
    // In side pose, we should see primarily one side
    // Check if wrists are close to hips (arms at side)
    if (leftWrist && leftHip && (leftWrist.score || 0) > this.MIN_JOINT_CONFIDENCE) {
      const distance = Math.abs(leftWrist.x - leftHip.x);
      if (distance < imageWidth * this.SIDE_POSE_ALIGNMENT_THRESHOLD) {
        return PoseInspectionResult.TRUE_IN_CONTOUR;
      }
    }
    
    if (rightWrist && rightHip && (rightWrist.score || 0) > this.MIN_JOINT_CONFIDENCE) {
      const distance = Math.abs(rightWrist.x - rightHip.x);
      if (distance < imageWidth * this.SIDE_POSE_ALIGNMENT_THRESHOLD) {
        return PoseInspectionResult.TRUE_IN_CONTOUR;
      }
    }
    
    return PoseInspectionResult.FALSE_NOT_IN_CONTOUR;
  }
  
  private inspectLowerBodySide(keypoints: poseDetection.Keypoint[], imageWidth: number): PoseInspectionResult {
    const leftAnkle = keypoints.find(kp => kp.name === 'left_ankle');
    const rightAnkle = keypoints.find(kp => kp.name === 'right_ankle');
    
    if (!leftAnkle || !rightAnkle) {
      return PoseInspectionResult.FALSE_NOT_DETECTED;
    }
    
    if ((leftAnkle.score || 0) < this.MIN_JOINT_CONFIDENCE ||
        (rightAnkle.score || 0) < this.MIN_JOINT_CONFIDENCE) {
      return PoseInspectionResult.FALSE_NOT_DETECTED;
    }
    
    // Check if feet are together (for side pose)
    const ankleDistance = Math.abs(leftAnkle.x - rightAnkle.x);
    if (ankleDistance < imageWidth * this.SIDE_POSE_ALIGNMENT_THRESHOLD) {
      return PoseInspectionResult.TRUE_IN_CONTOUR;
    }
    
    return PoseInspectionResult.FALSE_NOT_IN_CONTOUR;
  }
  
  private isPointInZone(point: poseDetection.Keypoint, zone: OptimalZone): boolean {
    return point.x >= zone.x &&
           point.x <= zone.x + zone.width &&
           point.y >= zone.y &&
           point.y <= zone.y + zone.height;
  }
  
  private createFailedInspection(profile: Profile, reason: PoseInspectionResult): PoseInspectionDictionary {
    const result: PoseInspectionDictionary = {
      face: reason
    };
    
    if (profile === 'front') {
      result.la = reason;
      result.ra = reason;
      result.ll = reason;
      result.rl = reason;
    } else {
      result.ub = reason;
      result.lb = reason;
    }
    
    return result;
  }
}