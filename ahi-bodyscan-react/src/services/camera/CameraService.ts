// Temporary stub for CameraService - will be replaced when body scan SDK is integrated

export enum CameraState {
  UNINITIALIZED = 'UNINITIALIZED',
  INITIALIZING = 'INITIALIZING',
  READY = 'READY',
  ACTIVE = 'ACTIVE',
  STREAMING = 'STREAMING',
  ERROR = 'ERROR',
}

export interface CameraConfig {
  video: {
    width: { ideal: number };
    height: { ideal: number };
    facingMode?: string;
  };
  audio: boolean;
}

export const DEFAULT_CAMERA_CONFIG: CameraConfig = {
  video: {
    width: { ideal: 1280 },
    height: { ideal: 720 },
    facingMode: 'user',
  },
  audio: false,
};

export class CameraService {
  private _state: CameraState = CameraState.UNINITIALIZED;
  private _stream: MediaStream | null = null;
  private _videoElement: HTMLVideoElement | null = null;
  private _stateChangeCallback?: (state: CameraState) => void;

  get state(): CameraState {
    return this._state;
  }

  get stream(): MediaStream | null {
    return this._stream;
  }

  get videoElement(): HTMLVideoElement | null {
    return this._videoElement;
  }

  onStateChange(callback: (state: CameraState) => void): void {
    this._stateChangeCallback = callback;
  }

  private setState(newState: CameraState): void {
    this._state = newState;
    if (this._stateChangeCallback) {
      this._stateChangeCallback(newState);
    }
  }

  async initialize(config: CameraConfig = DEFAULT_CAMERA_CONFIG): Promise<void> {
    console.log('CameraService: Temporary stub - initialize called');
    this.setState(CameraState.INITIALIZING);
    
    try {
      // Temporary: Just get camera stream
      this._stream = await navigator.mediaDevices.getUserMedia(config);
      this.setState(CameraState.READY);
    } catch (error) {
      console.error('CameraService: Failed to initialize camera', error);
      this.setState(CameraState.ERROR);
      throw error;
    }
  }

  async startCamera(videoElement?: HTMLVideoElement): Promise<void> {
    console.log('CameraService: Temporary stub - startCamera called');
    
    if (!this._stream) {
      throw new Error('Camera not initialized. Call initialize() first.');
    }

    if (videoElement) {
      this._videoElement = videoElement;
      this._videoElement.srcObject = this._stream;
      await this._videoElement.play();
    }

    this.setState(CameraState.STREAMING);
  }
  
  setVideoElement(videoElement: HTMLVideoElement): void {
    console.log('CameraService: Setting video element');
    this._videoElement = videoElement;
    if (this._stream && videoElement) {
      videoElement.srcObject = this._stream;
    }
  }

  stopCamera(): void {
    console.log('CameraService: Temporary stub - stopCamera called');
    
    if (this._stream) {
      this._stream.getTracks().forEach(track => track.stop());
      this._stream = null;
    }

    if (this._videoElement) {
      this._videoElement.srcObject = null;
      this._videoElement = null;
    }

    this._state = CameraState.READY;
  }

  captureFrame(videoElement: HTMLVideoElement, canvasElement: HTMLCanvasElement): ImageData | null {
    const context = canvasElement.getContext('2d');
    if (!context) return null;

    canvasElement.width = videoElement.videoWidth;
    canvasElement.height = videoElement.videoHeight;
    context.drawImage(videoElement, 0, 0);

    return context.getImageData(0, 0, canvasElement.width, canvasElement.height);
  }

  async takePicture(): Promise<string | null> {
    console.log('CameraService: Temporary stub - takePicture called');
    
    if (!this._videoElement || !this._stream) {
      return null;
    }

    // Create a canvas to capture the frame
    const canvas = document.createElement('canvas');
    canvas.width = this._videoElement.videoWidth;
    canvas.height = this._videoElement.videoHeight;
    
    const context = canvas.getContext('2d');
    if (!context) return null;
    
    // Draw the current video frame
    context.drawImage(this._videoElement, 0, 0);
    
    // Convert to base64 data URL
    return canvas.toDataURL('image/jpeg', 0.9);
  }

  reset(): void {
    this.stopCamera();
    this._state = CameraState.UNINITIALIZED;
  }

  dispose(): void {
    this.reset();
  }
}

// Export singleton instance
let cameraServiceInstance: CameraService | null = null;

export function getCameraService(): CameraService {
  if (!cameraServiceInstance) {
    cameraServiceInstance = new CameraService();
  }
  return cameraServiceInstance;
}