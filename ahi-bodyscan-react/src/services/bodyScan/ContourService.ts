import { Profile } from './PoseInspectionService';

export interface ContourPoint {
  x: number;
  y: number;
}

export interface ContourConfig {
  width: number;
  height: number;
  profile: Profile;
  gender: 'male' | 'female';
  devicePixelRatio?: number;
}

export interface ContourStyle {
  backgroundColor: string;
  borderColor: string;
  borderWidth: number;
  isDashed: boolean;
  dashPattern?: number[];
}

export interface OptimalContourZones {
  head: { x: number; y: number; width: number; height: number };
  torso: { x: number; y: number; width: number; height: number };
  legs: { x: number; y: number; width: number; height: number };
  arms?: { x: number; y: number; width: number; height: number };
}

export class ContourService {
  private static instance: ContourService;
  
  // iOS SDK standard capture dimensions
  private readonly STANDARD_WIDTH = 720;
  private readonly STANDARD_HEIGHT = 1280;
  
  // Contour shape parameters (normalized 0-1)
  private readonly CONTOUR_RATIOS = {
    front: {
      width: 0.6,  // 60% of frame width
      height: 0.9, // 90% of frame height
      headHeight: 0.15, // 15% for head zone
      torsoHeight: 0.4, // 40% for torso zone
      legsHeight: 0.45, // 45% for legs zone
      armExtension: 0.15 // 15% extension for arms
    },
    side: {
      width: 0.4,  // 40% of frame width (narrower for side view)
      height: 0.9, // 90% of frame height
      headHeight: 0.15,
      torsoHeight: 0.45,
      legsHeight: 0.4
    }
  };
  
  private constructor() {}
  
  static getInstance(): ContourService {
    if (!ContourService.instance) {
      ContourService.instance = new ContourService();
    }
    return ContourService.instance;
  }
  
  /**
   * Generate contour points for the given configuration
   */
  generateContour(config: ContourConfig): ContourPoint[] {
    const { profile } = config;
    const ratios = this.CONTOUR_RATIOS[profile];
    
    // Always generate contour based on standard dimensions
    const contourWidth = this.STANDARD_WIDTH * ratios.width;
    const contourHeight = this.STANDARD_HEIGHT * ratios.height;
    const centerX = this.STANDARD_WIDTH / 2;
    const centerY = this.STANDARD_HEIGHT / 2;
    
    if (profile === 'front') {
      return this.generateFrontContour(centerX, centerY, contourWidth, contourHeight);
    } else {
      return this.generateSideContour(centerX, centerY, contourWidth, contourHeight);
    }
  }
  
  /**
   * Generate optimal zones for pose inspection
   */
  generateOptimalZones(config: ContourConfig): OptimalContourZones {
    const { profile } = config;
    const ratios = this.CONTOUR_RATIOS[profile];
    
    // Use standard dimensions for zone generation
    const contourWidth = this.STANDARD_WIDTH * ratios.width;
    const contourHeight = this.STANDARD_HEIGHT * ratios.height;
    const centerX = this.STANDARD_WIDTH / 2;
    const startY = (this.STANDARD_HEIGHT - contourHeight) / 2;
    
    const zones: OptimalContourZones = {
      head: {
        x: centerX - contourWidth / 2,
        y: startY,
        width: contourWidth,
        height: contourHeight * ratios.headHeight
      },
      torso: {
        x: centerX - contourWidth / 2,
        y: startY + contourHeight * ratios.headHeight,
        width: contourWidth,
        height: contourHeight * ratios.torsoHeight
      },
      legs: {
        x: centerX - contourWidth / 2,
        y: startY + contourHeight * (ratios.headHeight + ratios.torsoHeight),
        width: contourWidth,
        height: contourHeight * ratios.legsHeight
      }
    };
    
    if (profile === 'front' && 'armExtension' in ratios) {
      // Add arm zones for front view
      zones.arms = {
        x: centerX - (contourWidth * (1 + ratios.armExtension * 2)) / 2,
        y: startY + contourHeight * ratios.headHeight,
        width: contourWidth * (1 + ratios.armExtension * 2),
        height: contourHeight * ratios.torsoHeight * 0.7
      };
    }
    
    return zones;
  }
  
  /**
   * Draw contour on canvas
   */
  drawContour(
    ctx: CanvasRenderingContext2D,
    points: ContourPoint[],
    style: ContourStyle
  ): void {
    if (points.length < 3) return;
    
    // Save current context state
    ctx.save();
    
    // Create the contour path
    const contourPath = new Path2D();
    contourPath.moveTo(points[0].x, points[0].y);
    
    for (let i = 1; i < points.length; i++) {
      contourPath.lineTo(points[i].x, points[i].y);
    }
    contourPath.closePath();
    
    // Create full canvas path (equivalent to iOS bigMaskPath)
    const fullCanvasPath = new Path2D();
    fullCanvasPath.rect(0, 0, ctx.canvas.width, ctx.canvas.height);
    
    // Create clipping path with even-odd fill rule (iOS clipPath)
    // This creates an inverse path - full canvas with contour cutout
    ctx.beginPath();
    ctx.rect(0, 0, ctx.canvas.width, ctx.canvas.height);
    ctx.moveTo(points[0].x, points[0].y);
    for (let i = 1; i < points.length; i++) {
      ctx.lineTo(points[i].x, points[i].y);
    }
    ctx.closePath();
    
    // Apply even-odd clipping
    ctx.clip('evenodd');
    
    // Fill the background with semi-transparent overlay
    ctx.fillStyle = style.backgroundColor;
    ctx.fillRect(0, 0, ctx.canvas.width, ctx.canvas.height);
    
    // Restore to draw border without clipping
    ctx.restore();
    ctx.save();
    
    // Draw the solid border line (baseLineLayer)
    ctx.strokeStyle = style.borderColor;
    ctx.lineWidth = 6; // Match iOS lineWidth
    ctx.lineJoin = 'round'; // Match iOS kCALineJoinRound
    ctx.setLineDash([]); // Solid line
    ctx.stroke(contourPath);
    
    // Draw the dashed overlay if requested (dashLineLayer)
    if (style.isDashed) {
      ctx.strokeStyle = '#FFFFFF'; // White dash overlay
      ctx.lineWidth = 6;
      ctx.lineJoin = 'round';
      ctx.setLineDash([14, 24]); // Match iOS dash pattern
      ctx.stroke(contourPath);
    }
    
    // Restore context state
    ctx.restore();
  }
  
  /**
   * Get contour style based on pose validity
   */
  getContourStyle(isValid: boolean): ContourStyle {
    return {
      backgroundColor: 'rgba(0, 0, 0, 0.5)', // Match iOS semi-transparent black
      borderColor: isValid ? '#007AFF' : '#007AFF', // iOS systemBlue color
      borderWidth: 6, // Match iOS lineWidth
      isDashed: true,
      dashPattern: [14, 24] // Match iOS dash pattern
    };
  }
  
  private generateFrontContour(
    centerX: number,
    centerY: number,
    width: number,
    height: number
  ): ContourPoint[] {
    const points: ContourPoint[] = [];
    const halfWidth = width / 2;
    const halfHeight = height / 2;
    
    // Create hourglass-like shape for human body
    // Starting from top center, going clockwise
    
    // Head (narrower top)
    const headWidth = halfWidth * 0.4;
    const shoulderY = centerY - halfHeight + height * 0.15;
    
    // Top of head
    points.push({ x: centerX, y: centerY - halfHeight });
    
    // Head curves
    points.push({ x: centerX + headWidth * 0.7, y: centerY - halfHeight + height * 0.05 });
    points.push({ x: centerX + headWidth, y: centerY - halfHeight + height * 0.1 });
    
    // Shoulders
    points.push({ x: centerX + halfWidth * 0.9, y: shoulderY });
    points.push({ x: centerX + halfWidth, y: shoulderY + height * 0.05 });
    
    // Arms/Torso
    points.push({ x: centerX + halfWidth, y: centerY - height * 0.1 });
    
    // Waist (narrower)
    points.push({ x: centerX + halfWidth * 0.7, y: centerY });
    
    // Hips
    points.push({ x: centerX + halfWidth * 0.8, y: centerY + height * 0.1 });
    
    // Legs
    points.push({ x: centerX + halfWidth * 0.5, y: centerY + height * 0.3 });
    points.push({ x: centerX + halfWidth * 0.4, y: centerY + halfHeight - height * 0.05 });
    
    // Feet
    points.push({ x: centerX + halfWidth * 0.3, y: centerY + halfHeight });
    
    // Mirror for left side (going back up)
    points.push({ x: centerX - halfWidth * 0.3, y: centerY + halfHeight });
    points.push({ x: centerX - halfWidth * 0.4, y: centerY + halfHeight - height * 0.05 });
    points.push({ x: centerX - halfWidth * 0.5, y: centerY + height * 0.3 });
    points.push({ x: centerX - halfWidth * 0.8, y: centerY + height * 0.1 });
    points.push({ x: centerX - halfWidth * 0.7, y: centerY });
    points.push({ x: centerX - halfWidth, y: centerY - height * 0.1 });
    points.push({ x: centerX - halfWidth, y: shoulderY + height * 0.05 });
    points.push({ x: centerX - halfWidth * 0.9, y: shoulderY });
    points.push({ x: centerX - headWidth, y: centerY - halfHeight + height * 0.1 });
    points.push({ x: centerX - headWidth * 0.7, y: centerY - halfHeight + height * 0.05 });
    
    return points;
  }
  
  private generateSideContour(
    centerX: number,
    centerY: number,
    width: number,
    height: number
  ): ContourPoint[] {
    const points: ContourPoint[] = [];
    const halfWidth = width / 2;
    const halfHeight = height / 2;
    
    // Side profile is narrower and more vertical
    
    // Top of head
    points.push({ x: centerX, y: centerY - halfHeight });
    
    // Head profile
    points.push({ x: centerX + halfWidth * 0.4, y: centerY - halfHeight + height * 0.05 });
    points.push({ x: centerX + halfWidth * 0.5, y: centerY - halfHeight + height * 0.1 });
    
    // Neck/shoulders
    points.push({ x: centerX + halfWidth * 0.3, y: centerY - halfHeight + height * 0.15 });
    
    // Back
    points.push({ x: centerX + halfWidth * 0.4, y: centerY - height * 0.2 });
    points.push({ x: centerX + halfWidth * 0.35, y: centerY });
    
    // Buttocks
    points.push({ x: centerX + halfWidth * 0.5, y: centerY + height * 0.1 });
    
    // Back of legs
    points.push({ x: centerX + halfWidth * 0.3, y: centerY + height * 0.3 });
    points.push({ x: centerX + halfWidth * 0.2, y: centerY + halfHeight - height * 0.05 });
    
    // Heels
    points.push({ x: centerX + halfWidth * 0.1, y: centerY + halfHeight });
    
    // Front of feet
    points.push({ x: centerX - halfWidth * 0.3, y: centerY + halfHeight });
    
    // Front of legs
    points.push({ x: centerX - halfWidth * 0.2, y: centerY + halfHeight - height * 0.05 });
    points.push({ x: centerX - halfWidth * 0.15, y: centerY + height * 0.3 });
    
    // Front torso
    points.push({ x: centerX - halfWidth * 0.3, y: centerY + height * 0.1 });
    points.push({ x: centerX - halfWidth * 0.4, y: centerY });
    points.push({ x: centerX - halfWidth * 0.45, y: centerY - height * 0.2 });
    
    // Chest/face profile
    points.push({ x: centerX - halfWidth * 0.5, y: centerY - halfHeight + height * 0.15 });
    points.push({ x: centerX - halfWidth * 0.4, y: centerY - halfHeight + height * 0.1 });
    points.push({ x: centerX - halfWidth * 0.2, y: centerY - halfHeight + height * 0.05 });
    
    return points;
  }
  
  /**
   * Smooth contour points using bezier interpolation
   */
  smoothContour(points: ContourPoint[]): ContourPoint[] {
    if (points.length < 3) return points;
    
    const smoothed: ContourPoint[] = [];
    const tension = 0.3;
    
    for (let i = 0; i < points.length; i++) {
      const p0 = points[(i - 1 + points.length) % points.length];
      const p1 = points[i];
      const p2 = points[(i + 1) % points.length];
      const p3 = points[(i + 2) % points.length];
      
      for (let t = 0; t < 1; t += 0.1) {
        const t2 = t * t;
        const t3 = t2 * t;
        
        const x = 0.5 * (
          (2 * p1.x) +
          (-p0.x + p2.x) * t +
          (2 * p0.x - 5 * p1.x + 4 * p2.x - p3.x) * t2 +
          (-p0.x + 3 * p1.x - 3 * p2.x + p3.x) * t3
        );
        
        const y = 0.5 * (
          (2 * p1.y) +
          (-p0.y + p2.y) * t +
          (2 * p0.y - 5 * p1.y + 4 * p2.y - p3.y) * t2 +
          (-p0.y + 3 * p1.y - 3 * p2.y + p3.y) * t3
        );
        
        smoothed.push({ x, y });
      }
    }
    
    return smoothed;
  }
  
  /**
   * Scale contour points from standard dimensions to actual display dimensions
   */
  scaleContourPoints(points: ContourPoint[], targetWidth: number, targetHeight: number): ContourPoint[] {
    // Calculate scale factors
    const standardAspectRatio = this.STANDARD_WIDTH / this.STANDARD_HEIGHT;
    const targetAspectRatio = targetWidth / targetHeight;
    
    let scaleX: number;
    let scaleY: number;
    let offsetX = 0;
    let offsetY = 0;
    
    // Maintain aspect ratio - fit to contain
    if (targetAspectRatio > standardAspectRatio) {
      // Target is wider - scale based on height
      scaleY = targetHeight / this.STANDARD_HEIGHT;
      scaleX = scaleY;
      offsetX = (targetWidth - this.STANDARD_WIDTH * scaleX) / 2;
    } else {
      // Target is taller - scale based on width
      scaleX = targetWidth / this.STANDARD_WIDTH;
      scaleY = scaleX;
      offsetY = (targetHeight - this.STANDARD_HEIGHT * scaleY) / 2;
    }
    
    return points.map(point => ({
      x: point.x * scaleX + offsetX,
      y: point.y * scaleY + offsetY
    }));
  }
  
  /**
   * Get standard capture dimensions
   */
  getStandardDimensions(): { width: number; height: number } {
    return {
      width: this.STANDARD_WIDTH,
      height: this.STANDARD_HEIGHT
    };
  }
}