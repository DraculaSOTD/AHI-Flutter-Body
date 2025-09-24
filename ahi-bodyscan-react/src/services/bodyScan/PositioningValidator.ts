import { PoseInspectionDictionary, PoseInspectionResult, ContourZones } from './PoseInspectionService';
import { JointPoint } from './BodyScanService';

export enum PositioningMessage {
  NONE = 'none',
  PERFECT_SPOT = 'perfect_spot',
  MOVE_BACK = 'move_back',
  MOVE_CLOSER = 'move_closer',
  MOVE_LEFT = 'move_left',
  MOVE_RIGHT = 'move_right',
  PHONE_TOO_HIGH = 'phone_too_high',
  PHONE_TOO_LOW = 'phone_too_low',
  ANKLES_NOT_VISIBLE = 'ankles_not_visible',
  HEAD_NOT_IN_FRAME = 'head_not_in_frame',
  ARMS_NOT_AWAY = 'arms_not_away'
}

export interface PositioningResult {
  positioningMessage: PositioningMessage;
  isTerminal: boolean;
  feedback: string[];
}

export interface DetectedJoints {
  [key: string]: { x: number; y: number; confidence: number } | undefined;
}

export class PositioningValidator {
  private static instance: PositioningValidator;
  
  // Buffer zones (in pixels)
  private readonly ZONE_BUFFER = 50;
  
  // Error tracking for terminal conditions
  private errorCodeCount: Map<PositioningMessage, number> = new Map();
  private readonly ERROR_COUNT_THRESHOLD = 30; // After 30 frames (~3 seconds), show terminal error
  
  private constructor() {}
  
  static getInstance(): PositioningValidator {
    if (!PositioningValidator.instance) {
      PositioningValidator.instance = new PositioningValidator();
    }
    return PositioningValidator.instance;
  }
  
  /**
   * Validate positioning based on iOS SDK logic
   */
  validatePositioning(
    inspectionResult: PoseInspectionDictionary,
    joints: DetectedJoints,
    zones: ContourZones,
    imageWidth: number,
    imageHeight: number
  ): PositioningResult {
    // Scale zones from standard dimensions (720x1280) to actual video dimensions
    const scaledZones = this.scaleZones(zones, imageWidth, imageHeight);
    // Check ankle visibility first
    const hasLeftAnkle = inspectionResult.ll !== PoseInspectionResult.FALSE_NOT_DETECTED;
    const hasRightAnkle = inspectionResult.rl !== PoseInspectionResult.FALSE_NOT_DETECTED;
    const hasAnkle = hasLeftAnkle || hasRightAnkle;
    
    if (!hasAnkle) {
      return this.createResult(PositioningMessage.ANKLES_NOT_VISIBLE, ['Show ankles in frame']);
    }
    
    // Get head and ankle positions
    const headTop = this.getHeadTopPosition(joints);
    const anklesPoint = this.getAverageAnklesPoint(joints);
    
    if (!headTop) {
      return this.createResult(PositioningMessage.HEAD_NOT_IN_FRAME, ['Move head into frame']);
    }
    
    // Check arms position
    if (inspectionResult.la === PoseInspectionResult.FALSE_NOT_IN_CONTOUR ||
        inspectionResult.ra === PoseInspectionResult.FALSE_NOT_IN_CONTOUR) {
      return this.createResult(PositioningMessage.ARMS_NOT_AWAY, ['Move arms away from body']);
    }
    
    // Get zone boundaries
    const headZone = scaledZones.head;
    const ankleZone = scaledZones.legs; // Using legs zone as ankle zone
    
    // Zone checks
    const headInZone = this.isPointInZone(headTop, headZone);
    const anklesInZone = this.isPointInZone(anklesPoint, ankleZone);
    
    // Calculate boundaries with buffers
    const topOfAnkleZone = ankleZone.y;
    const bottomOfAnkleZone = ankleZone.y + ankleZone.height;
    const topOfHeadZone = headZone.y;
    const bottomOfHeadZone = headZone.y + headZone.height;
    
    const anklesBelowZone = anklesPoint.y > bottomOfAnkleZone;
    const anklesAboveZone = anklesPoint.y < topOfAnkleZone;
    const headBelowZone = headTop.y > bottomOfHeadZone;
    const headAboveZone = headTop.y < topOfHeadZone;
    const headInZoneOrAbove = headTop.y <= bottomOfHeadZone;
    
    // Perfect spot - both zones are good
    if (headInZone && anklesInZone) {
      this.resetErrorCount();
      return this.createResult(PositioningMessage.PERFECT_SPOT, ['Perfect! Hold still']);
    }
    
    // Move back conditions (iOS logic)
    if (headInZoneOrAbove && !anklesAboveZone && (headInZone !== anklesInZone)) {
      if (!hasAnkle) {
        return this.createResult(PositioningMessage.ANKLES_NOT_VISIBLE, ['Show ankles']);
      }
      return this.createResult(PositioningMessage.MOVE_BACK, ['Step back']);
    }
    
    if (headInZoneOrAbove && anklesBelowZone && !headInZone) {
      return this.createResult(PositioningMessage.MOVE_BACK, ['Step back']);
    }
    
    // Move closer conditions
    if (headBelowZone && anklesAboveZone) {
      return this.createResult(PositioningMessage.MOVE_CLOSER, ['Step forward']);
    }
    
    if (headInZone && anklesAboveZone) {
      return this.createResult(PositioningMessage.MOVE_CLOSER, ['Step forward']);
    }
    
    if (headBelowZone && anklesInZone) {
      return this.createResult(PositioningMessage.MOVE_CLOSER, ['Step forward']);
    }
    
    // Phone positioning
    if (headBelowZone && !hasAnkle) {
      return this.createResult(PositioningMessage.PHONE_TOO_HIGH, ['Lower phone']);
    }
    
    if (headAboveZone && anklesAboveZone) {
      return this.createResult(PositioningMessage.PHONE_TOO_LOW, ['Raise phone']);
    }
    
    // Default case
    if (!headInZone) {
      return this.createResult(PositioningMessage.HEAD_NOT_IN_FRAME, ['Center head in frame']);
    }
    
    if (!anklesInZone) {
      return this.createResult(PositioningMessage.ANKLES_NOT_VISIBLE, ['Show ankles']);
    }
    
    return this.createResult(PositioningMessage.NONE, ['Adjust position']);
  }
  
  private createResult(message: PositioningMessage, feedback: string[]): PositioningResult {
    // Track error counts for terminal conditions
    if (message !== PositioningMessage.PERFECT_SPOT && message !== PositioningMessage.NONE) {
      const count = (this.errorCodeCount.get(message) || 0) + 1;
      this.errorCodeCount.set(message, count);
    }
    
    const isTerminal = this.isErrorCountOverThreshold(message);
    
    return {
      positioningMessage: message,
      isTerminal,
      feedback
    };
  }
  
  private isErrorCountOverThreshold(message: PositioningMessage): boolean {
    const count = this.errorCodeCount.get(message) || 0;
    return count > this.ERROR_COUNT_THRESHOLD;
  }
  
  private resetErrorCount(): void {
    this.errorCodeCount.clear();
  }
  
  private getHeadTopPosition(joints: DetectedJoints): { x: number; y: number } | null {
    // Try to get head top first
    if (joints.CentroidHeadTop) {
      return joints.CentroidHeadTop;
    }
    
    // Fall back to nose position
    if (joints.nose) {
      return joints.nose;
    }
    
    // Fall back to eye positions
    if (joints.left_eye && joints.right_eye) {
      return {
        x: (joints.left_eye.x + joints.right_eye.x) / 2,
        y: Math.min(joints.left_eye.y, joints.right_eye.y) - 20 // Approximate head top
      };
    }
    
    return null;
  }
  
  private getAverageAnklesPoint(joints: DetectedJoints): { x: number; y: number } {
    const leftAnkle = joints.left_ankle || joints.CentroidLeftAnkle;
    const rightAnkle = joints.right_ankle || joints.CentroidRightAnkle;
    
    if (!leftAnkle && !rightAnkle) {
      return { x: 0, y: 0 };
    }
    
    if (!leftAnkle) {
      return rightAnkle!;
    }
    
    if (!rightAnkle) {
      return leftAnkle;
    }
    
    return {
      x: (leftAnkle.x + rightAnkle.x) / 2,
      y: (leftAnkle.y + rightAnkle.y) / 2
    };
  }
  
  private isPointInZone(point: { x: number; y: number }, zone: { x: number; y: number; width: number; height: number }): boolean {
    return point.x >= zone.x &&
           point.x <= zone.x + zone.width &&
           point.y >= zone.y &&
           point.y <= zone.y + zone.height;
  }
  
  /**
   * Get user-friendly feedback message
   */
  static getFeedbackMessage(message: PositioningMessage): string {
    switch (message) {
      case PositioningMessage.PERFECT_SPOT:
        return 'Perfect position! Hold still';
      case PositioningMessage.MOVE_BACK:
        return 'Step back from camera';
      case PositioningMessage.MOVE_CLOSER:
        return 'Step closer to camera';
      case PositioningMessage.MOVE_LEFT:
        return 'Move left';
      case PositioningMessage.MOVE_RIGHT:
        return 'Move right';
      case PositioningMessage.PHONE_TOO_HIGH:
        return 'Lower your phone';
      case PositioningMessage.PHONE_TOO_LOW:
        return 'Raise your phone';
      case PositioningMessage.ANKLES_NOT_VISIBLE:
        return 'Make sure both ankles are visible';
      case PositioningMessage.HEAD_NOT_IN_FRAME:
        return 'Move your head into the frame';
      case PositioningMessage.ARMS_NOT_AWAY:
        return 'Keep arms away from your body';
      default:
        return 'Adjust your position';
    }
  }
  
  /**
   * Scale zones from standard dimensions (720x1280) to actual dimensions
   */
  private scaleZones(zones: ContourZones, targetWidth: number, targetHeight: number): ContourZones {
    const STANDARD_WIDTH = 720;
    const STANDARD_HEIGHT = 1280;
    
    const scaleX = targetWidth / STANDARD_WIDTH;
    const scaleY = targetHeight / STANDARD_HEIGHT;
    
    const scaleZone = (zone: { x: number; y: number; width: number; height: number }) => ({
      x: zone.x * scaleX,
      y: zone.y * scaleY,
      width: zone.width * scaleX,
      height: zone.height * scaleY
    });
    
    return {
      head: scaleZone(zones.head),
      torso: scaleZone(zones.torso),
      legs: scaleZone(zones.legs),
      arms: zones.arms ? scaleZone(zones.arms) : undefined
    };
  }
}