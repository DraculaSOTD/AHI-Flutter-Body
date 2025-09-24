export class AudioService {
  private static instance: AudioService;
  private audioContext: AudioContext | null = null;
  private sounds: Map<string, AudioBuffer> = new Map();
  
  // Sound types
  static readonly SOUNDS = {
    COUNTDOWN_TICK: 'countdown_tick',
    COUNTDOWN_GO: 'countdown_go',
    ALIGNMENT_GOOD: 'alignment_good',
    ALIGNMENT_BAD: 'alignment_bad',
    CAPTURE_SUCCESS: 'capture_success',
    POSE_VALID: 'pose_valid',
    POSE_INVALID: 'pose_invalid'
  };
  
  private constructor() {}
  
  static getInstance(): AudioService {
    if (!AudioService.instance) {
      AudioService.instance = new AudioService();
    }
    return AudioService.instance;
  }
  
  /**
   * Initialize the audio service
   */
  async initialize(): Promise<void> {
    try {
      // Create audio context
      this.audioContext = new (window.AudioContext || (window as any).webkitAudioContext)();
      
      // Generate synthetic sounds
      await this.generateSounds();
    } catch (error) {
      console.error('Failed to initialize audio service:', error);
    }
  }
  
  /**
   * Generate synthetic sounds using Web Audio API
   */
  private async generateSounds(): Promise<void> {
    if (!this.audioContext) return;
    
    // Countdown tick sound (short beep)
    this.sounds.set(
      AudioService.SOUNDS.COUNTDOWN_TICK,
      await this.createBeep(800, 0.1, 'sine')
    );
    
    // Countdown go sound (longer, lower beep)
    this.sounds.set(
      AudioService.SOUNDS.COUNTDOWN_GO,
      await this.createBeep(600, 0.3, 'sine')
    );
    
    // Alignment good sound (ascending tone)
    this.sounds.set(
      AudioService.SOUNDS.ALIGNMENT_GOOD,
      await this.createAscendingTone(400, 600, 0.2)
    );
    
    // Alignment bad sound (descending tone)
    this.sounds.set(
      AudioService.SOUNDS.ALIGNMENT_BAD,
      await this.createDescendingTone(600, 400, 0.2)
    );
    
    // Capture success sound (two quick ascending beeps)
    this.sounds.set(
      AudioService.SOUNDS.CAPTURE_SUCCESS,
      await this.createSuccessSound()
    );
    
    // Pose valid sound (soft chime)
    this.sounds.set(
      AudioService.SOUNDS.POSE_VALID,
      await this.createChime(800, 0.15)
    );
    
    // Pose invalid sound (soft buzz)
    this.sounds.set(
      AudioService.SOUNDS.POSE_INVALID,
      await this.createBuzz(200, 0.1)
    );
  }
  
  /**
   * Play a sound
   */
  play(soundName: string, volume: number = 0.5): void {
    if (!this.audioContext || this.audioContext.state === 'suspended') {
      // Try to resume context
      this.audioContext?.resume();
    }
    
    const buffer = this.sounds.get(soundName);
    if (!buffer || !this.audioContext) return;
    
    try {
      const source = this.audioContext.createBufferSource();
      const gainNode = this.audioContext.createGain();
      
      source.buffer = buffer;
      gainNode.gain.value = volume;
      
      source.connect(gainNode);
      gainNode.connect(this.audioContext.destination);
      
      source.start(0);
    } catch (error) {
      console.error('Failed to play sound:', error);
    }
  }
  
  /**
   * Create a simple beep sound
   */
  private createBeep(frequency: number, duration: number, type: OscillatorType): Promise<AudioBuffer> {
    return new Promise((resolve) => {
      if (!this.audioContext) {
        resolve(this.createEmptyBuffer());
        return;
      }
      
      const sampleRate = this.audioContext.sampleRate;
      const length = sampleRate * duration;
      const buffer = this.audioContext.createBuffer(1, length, sampleRate);
      const channel = buffer.getChannelData(0);
      
      for (let i = 0; i < length; i++) {
        const t = i / sampleRate;
        const envelope = Math.sin(Math.PI * t / duration); // Smooth envelope
        
        if (type === 'sine') {
          channel[i] = Math.sin(2 * Math.PI * frequency * t) * envelope * 0.3;
        } else if (type === 'square') {
          channel[i] = (Math.sin(2 * Math.PI * frequency * t) > 0 ? 1 : -1) * envelope * 0.1;
        }
      }
      
      resolve(buffer);
    });
  }
  
  /**
   * Create an ascending tone
   */
  private createAscendingTone(startFreq: number, endFreq: number, duration: number): Promise<AudioBuffer> {
    return new Promise((resolve) => {
      if (!this.audioContext) {
        resolve(this.createEmptyBuffer());
        return;
      }
      
      const sampleRate = this.audioContext.sampleRate;
      const length = sampleRate * duration;
      const buffer = this.audioContext.createBuffer(1, length, sampleRate);
      const channel = buffer.getChannelData(0);
      
      for (let i = 0; i < length; i++) {
        const t = i / sampleRate;
        const progress = t / duration;
        const frequency = startFreq + (endFreq - startFreq) * progress;
        const envelope = Math.sin(Math.PI * progress);
        channel[i] = Math.sin(2 * Math.PI * frequency * t) * envelope * 0.3;
      }
      
      resolve(buffer);
    });
  }
  
  /**
   * Create a descending tone
   */
  private createDescendingTone(startFreq: number, endFreq: number, duration: number): Promise<AudioBuffer> {
    return this.createAscendingTone(startFreq, endFreq, duration);
  }
  
  /**
   * Create a success sound (two quick beeps)
   */
  private async createSuccessSound(): Promise<AudioBuffer> {
    if (!this.audioContext) return this.createEmptyBuffer();
    
    const sampleRate = this.audioContext.sampleRate;
    const duration = 0.3;
    const length = sampleRate * duration;
    const buffer = this.audioContext.createBuffer(1, length, sampleRate);
    const channel = buffer.getChannelData(0);
    
    // First beep at 600Hz
    for (let i = 0; i < length * 0.4; i++) {
      const t = i / sampleRate;
      const envelope = Math.sin(Math.PI * t / (duration * 0.4));
      channel[i] = Math.sin(2 * Math.PI * 600 * t) * envelope * 0.3;
    }
    
    // Silence
    for (let i = length * 0.4; i < length * 0.5; i++) {
      channel[i] = 0;
    }
    
    // Second beep at 800Hz
    for (let i = length * 0.5; i < length; i++) {
      const t = (i - length * 0.5) / sampleRate;
      const envelope = Math.sin(Math.PI * t / (duration * 0.5));
      channel[i] = Math.sin(2 * Math.PI * 800 * t) * envelope * 0.3;
    }
    
    return buffer;
  }
  
  /**
   * Create a chime sound
   */
  private createChime(frequency: number, duration: number): Promise<AudioBuffer> {
    return new Promise((resolve) => {
      if (!this.audioContext) {
        resolve(this.createEmptyBuffer());
        return;
      }
      
      const sampleRate = this.audioContext.sampleRate;
      const length = sampleRate * duration;
      const buffer = this.audioContext.createBuffer(1, length, sampleRate);
      const channel = buffer.getChannelData(0);
      
      for (let i = 0; i < length; i++) {
        const t = i / sampleRate;
        const envelope = Math.exp(-3 * t); // Exponential decay
        
        // Mix of fundamental and harmonics for chime-like sound
        channel[i] = (
          Math.sin(2 * Math.PI * frequency * t) * 0.5 +
          Math.sin(2 * Math.PI * frequency * 2 * t) * 0.3 +
          Math.sin(2 * Math.PI * frequency * 3 * t) * 0.2
        ) * envelope * 0.2;
      }
      
      resolve(buffer);
    });
  }
  
  /**
   * Create a buzz sound
   */
  private createBuzz(frequency: number, duration: number): Promise<AudioBuffer> {
    return new Promise((resolve) => {
      if (!this.audioContext) {
        resolve(this.createEmptyBuffer());
        return;
      }
      
      const sampleRate = this.audioContext.sampleRate;
      const length = sampleRate * duration;
      const buffer = this.audioContext.createBuffer(1, length, sampleRate);
      const channel = buffer.getChannelData(0);
      
      for (let i = 0; i < length; i++) {
        const t = i / sampleRate;
        const envelope = Math.sin(Math.PI * t / duration);
        
        // Mix of low frequencies for buzz
        channel[i] = (
          Math.sin(2 * Math.PI * frequency * t) +
          Math.sin(2 * Math.PI * (frequency * 1.5) * t) * 0.5 +
          Math.random() * 0.1 // Add some noise
        ) * envelope * 0.1;
      }
      
      resolve(buffer);
    });
  }
  
  /**
   * Create an empty buffer as fallback
   */
  private createEmptyBuffer(): AudioBuffer {
    const ac = new (window.AudioContext || (window as any).webkitAudioContext)();
    return ac.createBuffer(1, 1, 22050);
  }
  
  /**
   * Enable audio (call on user interaction)
   */
  async enableAudio(): Promise<void> {
    if (this.audioContext && this.audioContext.state === 'suspended') {
      await this.audioContext.resume();
    }
  }
}