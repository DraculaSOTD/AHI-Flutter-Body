export interface DeviceOrientation {
  alpha: number | null; // Rotation around z-axis (0-360)
  beta: number | null;  // Front-to-back tilt (-180 to 180)
  gamma: number | null; // Left-to-right tilt (-90 to 90)
}

export interface AlignmentResult {
  isAligned: boolean;
  pitch: number; // Device tilt forward/backward
  roll: number;  // Device tilt left/right
  feedback: string[];
}

export class AlignmentService {
  private static instance: AlignmentService;
  private orientationCallback: ((event: DeviceOrientationEvent) => void) | null = null;
  private currentOrientation: DeviceOrientation = {
    alpha: null,
    beta: null,
    gamma: null
  };
  
  // Smoothing for orientation values
  private orientationHistory: DeviceOrientation[] = [];
  private readonly SMOOTHING_SAMPLES = 5;
  private lastUpdateTime = 0;
  private readonly UPDATE_INTERVAL = 100; // Update every 100ms max
  
  // Alignment thresholds (in degrees)
  private readonly PITCH_THRESHOLD = 1.5; // ±1.5 degrees as per iOS SDK
  private readonly ROLL_THRESHOLD = 1.5;  // ±1.5 degrees as per iOS SDK
  private readonly OPTIMAL_PITCH = 90;    // Device should be vertical
  private readonly OPTIMAL_ROLL = 0;      // Device should be level
  
  private constructor() {}
  
  static getInstance(): AlignmentService {
    if (!AlignmentService.instance) {
      AlignmentService.instance = new AlignmentService();
    }
    return AlignmentService.instance;
  }
  
  /**
   * Start monitoring device orientation
   */
  async startMonitoring(): Promise<void> {
    // Check if device orientation is supported
    if (!window.DeviceOrientationEvent) {
      throw new Error('Device orientation not supported');
    }
    
    // Request permission on iOS 13+
    if (typeof (DeviceOrientationEvent as any).requestPermission === 'function') {
      try {
        const permission = await (DeviceOrientationEvent as any).requestPermission();
        if (permission !== 'granted') {
          throw new Error('Device orientation permission denied');
        }
      } catch (error) {
        console.error('Failed to request device orientation permission:', error);
        throw error;
      }
    }
    
    // Start listening to orientation events with smoothing
    this.orientationCallback = (event: DeviceOrientationEvent) => {
      const now = Date.now();
      
      // Throttle updates
      if (now - this.lastUpdateTime < this.UPDATE_INTERVAL) {
        return;
      }
      this.lastUpdateTime = now;
      
      // Add to history for smoothing
      this.orientationHistory.push({
        alpha: event.alpha,
        beta: event.beta,
        gamma: event.gamma
      });
      
      // Keep only recent samples
      if (this.orientationHistory.length > this.SMOOTHING_SAMPLES) {
        this.orientationHistory.shift();
      }
      
      // Calculate smoothed values
      this.currentOrientation = this.getSmoothedOrientation();
    };
    
    window.addEventListener('deviceorientation', this.orientationCallback);
  }
  
  /**
   * Stop monitoring device orientation
   */
  stopMonitoring(): void {
    if (this.orientationCallback) {
      window.removeEventListener('deviceorientation', this.orientationCallback);
      this.orientationCallback = null;
    }
  }
  
  /**
   * Get current device alignment status
   */
  getAlignment(): AlignmentResult {
    const { beta, gamma } = this.currentOrientation;
    
    if (beta === null || gamma === null) {
      return {
        isAligned: false,
        pitch: 0,
        roll: 0,
        feedback: ['Device orientation not available']
      };
    }
    
    // Get gravity vector for more accurate alignment detection
    const gravityVector = this.getGravityVector();
    
    // Calculate angle in radians similar to iOS SDK
    // Zero pitch is when device is lying flat, but we want zero when standing vertically
    const pitchDirection = gravityVector.z > 0 ? -1 : 1;
    const angleInRad = ((beta * Math.PI / 180) - Math.PI / 2) * pitchDirection;
    const angleInDeg = Math.abs(angleInRad * 180 / Math.PI);
    
    // Calculate roll (left/right tilt)
    const roll = Math.abs(gamma - this.OPTIMAL_ROLL);
    
    const feedback: string[] = [];
    let isAligned = true;
    
    // Check alignment using angle threshold like iOS
    if (angleInDeg > this.PITCH_THRESHOLD) {
      isAligned = false;
      if (beta < this.OPTIMAL_PITCH) {
        feedback.push(`Tilt device back`);
      } else {
        feedback.push(`Tilt device forward`);
      }
    }
    
    // Check roll (left/right tilt)
    if (roll > this.ROLL_THRESHOLD) {
      isAligned = false;
      if (gamma < this.OPTIMAL_ROLL) {
        feedback.push(`Tilt device right`);
      } else {
        feedback.push(`Tilt device left`);
      }
    }
    
    if (isAligned) {
      feedback.push('Device aligned');
    }
    
    return {
      isAligned,
      pitch: angleInDeg,
      roll,
      feedback
    };
  }
  
  /**
   * Get visual indicator position for alignment UI
   */
  getAlignmentIndicatorPosition(): { x: number; y: number } {
    const { beta, gamma } = this.currentOrientation;
    
    if (beta === null || gamma === null) {
      return { x: 0, y: 0 };
    }
    
    // Normalize to -1 to 1 range
    const normalizedPitch = (beta - this.OPTIMAL_PITCH) / 45; // 45 degrees = full range
    const normalizedRoll = gamma / 45;
    
    // Clamp values
    const x = Math.max(-1, Math.min(1, normalizedRoll));
    const y = Math.max(-1, Math.min(1, normalizedPitch));
    
    return { x, y };
  }
  
  /**
   * Check if device is in portrait orientation
   */
  isPortrait(): boolean {
    return window.innerHeight > window.innerWidth;
  }
  
  /**
   * Get device motion permission status
   */
  async checkPermission(): Promise<boolean> {
    if (typeof (DeviceOrientationEvent as any).requestPermission !== 'function') {
      // Permission not required on this device
      return true;
    }
    
    try {
      const permission = await (DeviceOrientationEvent as any).requestPermission();
      return permission === 'granted';
    } catch (error) {
      console.error('Failed to check device orientation permission:', error);
      return false;
    }
  }
  
  /**
   * Calibrate the device alignment (set current position as optimal)
   */
  calibrate(): void {
    const { beta, gamma } = this.currentOrientation;
    
    if (beta !== null && gamma !== null) {
      // Store calibration offsets
      // This could be expanded to store calibration in localStorage
      console.log('Device calibrated at pitch:', beta, 'roll:', gamma);
    }
  }
  
  /**
   * Get alignment quality score (0-100)
   */
  getAlignmentScore(): number {
    const { pitch, roll } = this.getAlignment();
    
    // Calculate score based on how close to optimal alignment
    const pitchScore = Math.max(0, 100 - (pitch / this.PITCH_THRESHOLD) * 50);
    const rollScore = Math.max(0, 100 - (roll / this.ROLL_THRESHOLD) * 50);
    
    return Math.round((pitchScore + rollScore) / 2);
  }
  
  /**
   * Check if device supports orientation detection
   */
  isSupported(): boolean {
    return 'DeviceOrientationEvent' in window;
  }
  
  /**
   * Get smoothed orientation values
   */
  private getSmoothedOrientation(): DeviceOrientation {
    if (this.orientationHistory.length === 0) {
      return { alpha: null, beta: null, gamma: null };
    }
    
    let sumAlpha = 0, sumBeta = 0, sumGamma = 0;
    let validSamples = 0;
    
    for (const sample of this.orientationHistory) {
      if (sample.alpha !== null && sample.beta !== null && sample.gamma !== null) {
        sumAlpha += sample.alpha;
        sumBeta += sample.beta;
        sumGamma += sample.gamma;
        validSamples++;
      }
    }
    
    if (validSamples === 0) {
      return { alpha: null, beta: null, gamma: null };
    }
    
    return {
      alpha: sumAlpha / validSamples,
      beta: sumBeta / validSamples,
      gamma: sumGamma / validSamples
    };
  }
  
  /**
   * Get gravity vector similar to iOS implementation
   */
  getGravityVector(): { x: number; y: number; z: number } {
    const { beta, gamma } = this.currentOrientation;
    
    if (beta === null || gamma === null) {
      return { x: 0, y: 0, z: 0 };
    }
    
    // Convert to radians
    const betaRad = (beta * Math.PI) / 180;
    const gammaRad = (gamma * Math.PI) / 180;
    
    // Calculate gravity components
    // These formulas approximate the device's orientation relative to gravity
    const x = Math.sin(gammaRad);
    const y = Math.sin(betaRad) * Math.cos(gammaRad);
    const z = Math.cos(betaRad) * Math.cos(gammaRad);
    
    return { x, y, z };
  }
}