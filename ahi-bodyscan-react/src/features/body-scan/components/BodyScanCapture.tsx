import React, { useEffect, useRef, useState } from 'react';
import styled from 'styled-components';
import { CameraPreview } from '../../../components/camera/CameraPreview';
import { LoadingOverlay } from '../../../components/common/LoadingOverlay';
import { BodyScanService, BodyScanPose, PoseDetectionResult, AlignmentService } from '../../../services/bodyScan';
import { CameraService } from '../../../services/camera/CameraService';
import { theme } from '../../../styles/theme';
import { ContourOverlay } from './ContourOverlay';
import { AlignmentIndicator } from './AlignmentIndicator';
import { AudioService } from '../../../services/audio/AudioService';

// Styled Components
const CameraContainer = styled.div`
  position: relative;
  width: 100%;
  height: 100vh;
  background: #000;
  overflow: hidden;
`;

const CameraView = styled.div`
  position: absolute;
  inset: 0;
  
  & > div {
    width: 100%;
    height: 100%;
  }
`;

const PoseOverlayCanvas = styled.canvas`
  position: absolute;
  inset: 0;
  pointer-events: none;
  z-index: 10;
`;

const CloseButton = styled.button`
  position: absolute;
  top: ${theme.dimensions.paddingLarge};
  left: ${theme.dimensions.paddingLarge};
  background: rgba(0, 0, 0, 0.5);
  backdrop-filter: blur(10px);
  border: none;
  color: ${theme.colors.textWhite};
  width: 48px;
  height: 48px;
  border-radius: 50%;
  display: flex;
  align-items: center;
  justify-content: center;
  cursor: pointer;
  font-size: 24px;
  z-index: 30;
  transition: all ${theme.transitions.normal};
  
  &:hover {
    background: rgba(0, 0, 0, 0.7);
    transform: scale(1.05);
  }
  
  &:active {
    transform: scale(0.95);
  }
`;

const PoseTypeHeading = styled.div`
  position: absolute;
  top: ${theme.dimensions.paddingXLarge};
  left: 50%;
  transform: translateX(-50%);
  background: rgba(0, 0, 0, 0.7);
  backdrop-filter: blur(10px);
  color: ${theme.colors.textWhite};
  padding: ${theme.dimensions.paddingMedium} ${theme.dimensions.paddingXLarge};
  border-radius: 30px;
  font-size: ${theme.typography.headingLarge.fontSize};
  font-weight: ${theme.typography.headingLarge.fontWeight};
  z-index: 25;
  text-align: center;
  min-width: 200px;
  box-shadow: 0 4px 12px rgba(0, 0, 0, 0.3);
  
  @media (max-width: ${theme.breakpoints.mobile}) {
    font-size: ${theme.typography.headingMedium.fontSize};
    padding: ${theme.dimensions.paddingSmall} ${theme.dimensions.paddingLarge};
    min-width: 160px;
  }
`;


const FeedbackOverlay = styled.div`
  position: absolute;
  bottom: 120px;
  left: 50%;
  transform: translateX(-50%);
  background: rgba(0, 0, 0, 0.85);
  backdrop-filter: blur(10px);
  border-radius: 16px;
  padding: 20px 24px;
  max-width: 90%;
  width: 400px;
  z-index: 25;
  
  @media (max-width: ${theme.breakpoints.mobile}) {
    width: 320px;
    padding: 16px 20px;
    bottom: 100px;
  }
`;

const FeedbackList = styled.ul`
  list-style: none;
  padding: 0;
  margin: 0;
  display: flex;
  flex-direction: column;
  gap: 8px;
  
  li {
    color: ${theme.colors.textWhite};
    font-size: ${theme.typography.bodyMedium.fontSize};
    line-height: 1.4;
    display: flex;
    align-items: center;
    gap: 8px;
    
    &::before {
      content: '';
      display: inline-block;
      width: 20px;
      height: 20px;
      flex-shrink: 0;
    }
    
    &.move-left::before {
      content: '←';
      font-size: 18px;
    }
    
    &.move-right::before {
      content: '→';
      font-size: 18px;
    }
    
    &.move-back::before {
      content: '↓';
      font-size: 18px;
    }
    
    &.move-forward::before {
      content: '↑';
      font-size: 18px;
    }
    
    &.warning::before {
      content: '⚠';
      color: ${theme.colors.warning};
      font-size: 18px;
    }
  }
`;

const ControlsOverlay = styled.div`
  position: absolute;
  bottom: 40px;
  left: 0;
  right: 0;
  display: flex;
  justify-content: center;
  gap: 20px;
  padding: 0 20px;
  z-index: 30;
  
  @media (max-width: ${theme.breakpoints.mobile}) {
    bottom: 30px;
    gap: 16px;
  }
`;

const CancelButton = styled.button`
  background: rgba(255, 255, 255, 0.1);
  backdrop-filter: blur(10px);
  border: 2px solid rgba(255, 255, 255, 0.2);
  color: ${theme.colors.textWhite};
  padding: 16px 32px;
  border-radius: 50px;
  font-size: ${theme.typography.bodyLarge.fontSize};
  font-weight: 600;
  cursor: pointer;
  transition: all ${theme.transitions.normal};
  
  &:hover:not(:disabled) {
    background: rgba(255, 255, 255, 0.2);
    border-color: rgba(255, 255, 255, 0.3);
    transform: translateY(-2px);
  }
  
  &:active:not(:disabled) {
    transform: translateY(0);
  }
  
  &:disabled {
    opacity: 0.5;
    cursor: not-allowed;
  }
`;

const CaptureButton = styled.button<{ $isValid: boolean }>`
  background: ${props => props.$isValid 
    ? `linear-gradient(135deg, ${theme.colors.success} 0%, #059669 100%)`
    : 'rgba(255, 255, 255, 0.1)'};
  backdrop-filter: blur(10px);
  border: 2px solid ${props => props.$isValid 
    ? theme.colors.success 
    : 'rgba(255, 255, 255, 0.2)'};
  color: ${theme.colors.textWhite};
  padding: 16px 48px;
  border-radius: 50px;
  font-size: ${theme.typography.bodyLarge.fontSize};
  font-weight: 600;
  cursor: pointer;
  transition: all ${theme.transitions.normal};
  position: relative;
  overflow: hidden;
  
  &::before {
    content: '';
    position: absolute;
    top: 0;
    left: -100%;
    width: 100%;
    height: 100%;
    background: linear-gradient(90deg, transparent, rgba(255, 255, 255, 0.3), transparent);
    transition: left 0.5s;
  }
  
  &:hover:not(:disabled) {
    transform: translateY(-2px);
    box-shadow: 0 8px 24px rgba(0, 0, 0, 0.3);
    
    &::before {
      left: 100%;
    }
  }
  
  &:active:not(:disabled) {
    transform: translateY(0);
  }
  
  &:disabled {
    opacity: 0.5;
    cursor: not-allowed;
    background: rgba(255, 255, 255, 0.1);
  }
`;

const PoseIndicator = styled.div<{ $isValid: boolean }>`
  position: absolute;
  top: 50%;
  left: 50%;
  transform: translate(-50%, -50%);
  height: min(90vh, 850px);
  width: calc(min(90vh, 850px) * 0.6);
  border: 3px solid ${props => props.$isValid ? theme.colors.success : theme.colors.error};
  border-radius: 20px;
  pointer-events: none;
  z-index: 15;
  transition: all ${theme.transitions.normal};
  
  @media (max-width: ${theme.breakpoints.mobile}) {
    height: min(84vh, 630px);
    width: calc(min(84vh, 630px) * 0.6);
  }
`;

const CountdownOverlay = styled.div`
  position: absolute;
  top: 50%;
  left: 50%;
  transform: translate(-50%, -50%);
  font-size: 120px;
  font-weight: bold;
  color: ${theme.colors.primaryPurple};
  text-shadow: 0 0 20px rgba(0, 0, 0, 0.3);
  z-index: 40;
  animation: pulse 1s ease-in-out;
  
  @keyframes pulse {
    0% {
      transform: translate(-50%, -50%) scale(0.8);
      opacity: 0;
    }
    50% {
      transform: translate(-50%, -50%) scale(1.1);
      opacity: 1;
    }
    100% {
      transform: translate(-50%, -50%) scale(1);
      opacity: 1;
    }
  }
`;

// Component
interface BodyScanCaptureProps {
  bodyScanService: BodyScanService;
  pose: BodyScanPose;
  onCapture: (imageData: string) => void;
  onCancel: () => void;
}

export const BodyScanCapture: React.FC<BodyScanCaptureProps> = ({
  bodyScanService,
  pose,
  onCapture,
  onCancel
}) => {
  const [poseDetection, setPoseDetection] = useState<PoseDetectionResult | null>(null);
  const [isCapturing, setIsCapturing] = useState(false);
  const [countdown, setCountdown] = useState<number | null>(null);
  const [poseStableTime, setPoseStableTime] = useState<number>(0);
  const [isCountingDown, setIsCountingDown] = useState(false);
  const [showAlignment, setShowAlignment] = useState(false);
  const canvasRef = useRef<HTMLCanvasElement>(null);
  const countdownIntervalRef = useRef<NodeJS.Timeout | null>(null);
  const stableTimeoutRef = useRef<NodeJS.Timeout | null>(null);
  const alignmentService = useRef(AlignmentService.getInstance());
  const audioService = useRef(AudioService.getInstance());
  const [contourData, setContourData] = useState<{ 
    points: any[], 
    zones: any,
    displayDimensions?: {
      width: number;
      height: number;
    }
  } | null>(null);
  const lastAlignmentState = useRef<boolean | null>(null);
  const lastPoseValidState = useRef<boolean | null>(null);
  
  // Debug logging
  console.log('BodyScanCapture mounted', { pose, bodyScanService });

  useEffect(() => {
    console.log('BodyScanCapture useEffect - initializing capture');
    
    // Subscribe to pose detection updates
    bodyScanService.onPoseDetection((result) => {
      setPoseDetection(result);
      // Only draw if we have valid data
      if (result && result.joints) {
        drawPoseOverlay(result);
      }
    });
    
    // Initialize alignment monitoring
    const initAlignment = async () => {
      try {
        if (alignmentService.current.isSupported()) {
          const hasPermission = await alignmentService.current.checkPermission();
          if (hasPermission) {
            await alignmentService.current.startMonitoring();
            setShowAlignment(true);
          }
        }
      } catch (err) {
        console.warn('Alignment monitoring not available:', err);
      }
    };
    
    // Initialize audio service
    const initAudio = async () => {
      try {
        await audioService.current.initialize();
        // Enable audio on first user interaction
        await audioService.current.enableAudio();
      } catch (err) {
        console.warn('Audio service initialization failed:', err);
      }
    };
    
    initAlignment();
    initAudio();

    // Start capture with error handling
    const startCaptureAsync = async () => {
      try {
        // Wait for service to be initialized
        if (!bodyScanService.isInitialized) {
          console.log('Waiting for body scan service to initialize...');
          // Check every 100ms for up to 5 seconds
          let attempts = 0;
          while (!bodyScanService.isInitialized && attempts < 50) {
            await new Promise(resolve => setTimeout(resolve, 100));
            attempts++;
          }
          
          if (!bodyScanService.isInitialized) {
            throw new Error('Body scan service failed to initialize after 5 seconds');
          }
        }
        
        console.log('Starting capture for pose:', pose);
        await bodyScanService.startCapture(pose);
        console.log('Capture started successfully');
        
        // Get contour data after a delay to ensure video element is ready
        const updateContour = () => {
          const contour = bodyScanService.getContourData();
          console.log('Got contour data:', contour);
          if (contour) {
            setContourData(contour);
          }
        };
        
        // Initial contour generation
        setTimeout(updateContour, 1500);
        
        // Update contour periodically
        const contourInterval = setInterval(updateContour, 2000);
        
        // Cleanup interval on unmount
        return () => clearInterval(contourInterval);
      } catch (err) {
        console.error('Failed to start capture:', err);
        // Show error to user
        if (err instanceof Error) {
          alert(`Camera initialization failed: ${err.message}`);
        } else {
          alert('Failed to start camera. Please check permissions and try again.');
        }
      }
    };
    
    startCaptureAsync();

    return () => {
      // Cleanup timers
      if (countdownIntervalRef.current) {
        clearInterval(countdownIntervalRef.current);
      }
      if (stableTimeoutRef.current) {
        clearTimeout(stableTimeoutRef.current);
      }
      
      // Stop alignment monitoring
      alignmentService.current.stopMonitoring();
    };
  }, [bodyScanService, pose]);

  // Monitor pose stability and trigger countdown
  useEffect(() => {
    if (!poseDetection) return;

    // Play audio feedback for pose validity changes
    if (lastPoseValidState.current !== null && lastPoseValidState.current !== poseDetection.isValid) {
      if (poseDetection.isValid) {
        audioService.current.play(AudioService.SOUNDS.POSE_VALID, 0.3);
      } else {
        audioService.current.play(AudioService.SOUNDS.POSE_INVALID, 0.2);
      }
    }
    lastPoseValidState.current = poseDetection.isValid;
    
    // Check alignment state for audio feedback
    if (poseDetection.alignmentScore !== undefined) {
      const isAligned = poseDetection.alignmentScore >= 80;
      if (lastAlignmentState.current !== null && lastAlignmentState.current !== isAligned) {
        if (isAligned) {
          audioService.current.play(AudioService.SOUNDS.ALIGNMENT_GOOD, 0.3);
        } else {
          audioService.current.play(AudioService.SOUNDS.ALIGNMENT_BAD, 0.2);
        }
      }
      lastAlignmentState.current = isAligned;
    }

    if (poseDetection.isValid && !isCountingDown && !isCapturing) {
      // Pose is valid, start stability timer
      if (!stableTimeoutRef.current) {
        stableTimeoutRef.current = setTimeout(() => {
          // Pose has been stable for 3 seconds, start countdown
          startCountdown();
        }, 3000);
      }
    } else {
      // Pose is not valid or we're already counting down, reset stability timer
      if (stableTimeoutRef.current) {
        clearTimeout(stableTimeoutRef.current);
        stableTimeoutRef.current = null;
      }
      
      // If we were counting down and pose became invalid, stop countdown
      if (isCountingDown && !poseDetection.isValid) {
        stopCountdown();
      }
    }
  }, [poseDetection, isCountingDown, isCapturing]);

  const drawPoseOverlay = (result: PoseDetectionResult) => {
    const canvas = canvasRef.current;
    if (!canvas) return;

    const ctx = canvas.getContext('2d');
    if (!ctx) return;

    // Set canvas size to match viewport
    canvas.width = window.innerWidth;
    canvas.height = window.innerHeight;

    // Clear canvas
    ctx.clearRect(0, 0, canvas.width, canvas.height);
    
    // If no joints detected, don't draw anything
    if (!result.joints || result.joints.length === 0) {
      return;
    }

    // Enhanced joint visualization with confidence-based sizing
    result.joints.forEach((joint, index) => {
      try {
        if (joint.confidence > 0.3) {
          const x = joint.x * canvas.width;
          const y = joint.y * canvas.height;
          
          // Validate coordinates
          if (!isFinite(x) || !isFinite(y)) {
            console.warn('Invalid joint coordinate at index', index, joint);
            return;
          }
          
          // Clamp coordinates to canvas bounds
          const clampedX = Math.max(0, Math.min(canvas.width, x));
          const clampedY = Math.max(0, Math.min(canvas.height, y));
          
          const radius = 4 + (joint.confidence * 6); // 4-10px based on confidence
          
          // Draw outer glow
          ctx.beginPath();
          ctx.arc(clampedX, clampedY, radius + 4, 0, 2 * Math.PI);
          ctx.fillStyle = result.isValid 
            ? `rgba(16, 185, 129, ${joint.confidence * 0.3})` 
            : `rgba(239, 68, 68, ${joint.confidence * 0.3})`;
          ctx.fill();
          
          // Draw joint
          ctx.beginPath();
          ctx.arc(clampedX, clampedY, radius, 0, 2 * Math.PI);
          ctx.fillStyle = result.isValid ? '#10b981' : '#ef4444';
          ctx.fill();
          
          // Draw confidence ring
          ctx.beginPath();
          ctx.arc(clampedX, clampedY, radius + 2, 0, 2 * Math.PI);
          ctx.strokeStyle = result.isValid ? '#10b981' : '#ef4444';
          ctx.lineWidth = 2;
          ctx.stroke();
        }
      } catch (error) {
        console.error('Error drawing joint at index', index, error);
      }
    });

    // Draw skeleton connections
    drawSkeleton(ctx, result.joints, canvas.width, canvas.height, result.isValid);
  };

  const drawSkeleton = (
    ctx: CanvasRenderingContext2D, 
    joints: PoseDetectionResult['joints'],
    width: number,
    height: number,
    isValid: boolean
  ) => {
    const connections = [
      [5, 6], // shoulders
      [5, 7], [7, 9], // left arm
      [6, 8], [8, 10], // right arm
      [5, 11], [6, 12], // torso
      [11, 12], // hips
      [11, 13], [13, 15], // left leg
      [12, 14], [14, 16], // right leg
    ];

    connections.forEach(([start, end]) => {
      try {
        if (joints[start] && joints[end] && 
            joints[start].confidence > 0.3 && 
            joints[end].confidence > 0.3) {
          const startX = joints[start].x * width;
          const startY = joints[start].y * height;
          const endX = joints[end].x * width;
          const endY = joints[end].y * height;
          
          // Validate coordinates are finite
          if (!isFinite(startX) || !isFinite(startY) || !isFinite(endX) || !isFinite(endY)) {
            console.warn('Invalid joint coordinates:', { start: joints[start], end: joints[end] });
            return;
          }
          
          // Ensure coordinates are within canvas bounds
          const clampedStartX = Math.max(0, Math.min(width, startX));
          const clampedStartY = Math.max(0, Math.min(height, startY));
          const clampedEndX = Math.max(0, Math.min(width, endX));
          const clampedEndY = Math.max(0, Math.min(height, endY));
          
          // Calculate connection confidence
          const connectionConfidence = (joints[start].confidence + joints[end].confidence) / 2;
          
          // Use solid color if gradient would fail
          const color = isValid 
            ? `rgba(16, 185, 129, ${connectionConfidence})` 
            : `rgba(239, 68, 68, ${connectionConfidence})`;
          
          // Try to create gradient, fall back to solid color if it fails
          try {
            const gradient = ctx.createLinearGradient(clampedStartX, clampedStartY, clampedEndX, clampedEndY);
            gradient.addColorStop(0, isValid 
              ? `rgba(16, 185, 129, ${joints[start].confidence})` 
              : `rgba(239, 68, 68, ${joints[start].confidence})`);
            gradient.addColorStop(1, isValid 
              ? `rgba(16, 185, 129, ${joints[end].confidence})` 
              : `rgba(239, 68, 68, ${joints[end].confidence})`);
            ctx.strokeStyle = gradient;
          } catch (gradientError) {
            // Fallback to solid color
            ctx.strokeStyle = color;
          }
          
          ctx.lineWidth = 2 + (connectionConfidence * 2); // 2-4px based on confidence
          ctx.lineCap = 'round';
          
          ctx.beginPath();
          ctx.moveTo(clampedStartX, clampedStartY);
          ctx.lineTo(clampedEndX, clampedEndY);
          ctx.stroke();
        }
      } catch (error) {
        console.error('Error drawing skeleton connection:', error, { start, end });
      }
    });
  };

  const startCountdown = () => {
    setIsCountingDown(true);
    setCountdown(3);
    
    let count = 3;
    // Play initial tick
    audioService.current.play(AudioService.SOUNDS.COUNTDOWN_TICK, 0.5);
    
    countdownIntervalRef.current = setInterval(() => {
      count--;
      if (count === 0) {
        // Play 'go' sound
        audioService.current.play(AudioService.SOUNDS.COUNTDOWN_GO, 0.6);
        // Capture immediately when countdown reaches 0
        setCountdown(null);
        clearInterval(countdownIntervalRef.current!);
        countdownIntervalRef.current = null;
        handleAutoCapture();
      } else {
        setCountdown(count);
        // Play tick sound
        audioService.current.play(AudioService.SOUNDS.COUNTDOWN_TICK, 0.5);
      }
    }, 1000);
  };

  const stopCountdown = () => {
    setIsCountingDown(false);
    setCountdown(null);
    if (countdownIntervalRef.current) {
      clearInterval(countdownIntervalRef.current);
      countdownIntervalRef.current = null;
    }
  };

  const handleAutoCapture = async () => {
    setIsCapturing(true);
    try {
      const imageData = await bodyScanService.captureImage(pose);
      // Play success sound
      audioService.current.play(AudioService.SOUNDS.CAPTURE_SUCCESS, 0.7);
      onCapture(imageData);
    } catch (error) {
      console.error('Capture failed:', error);
      alert('Failed to capture image. Please try again.');
      setIsCountingDown(false);
    } finally {
      setIsCapturing(false);
    }
  };

  const handleCapture = async () => {
    if (!poseDetection?.isValid) {
      return;
    }

    // Stop any ongoing countdown
    stopCountdown();
    
    setIsCapturing(true);
    try {
      // Use enhanced capture with validation
      const imageData = await bodyScanService.captureImageWithValidation(pose);
      // Play success sound
      audioService.current.play(AudioService.SOUNDS.CAPTURE_SUCCESS, 0.7);
      onCapture(imageData);
    } catch (error) {
      console.error('Capture failed:', error);
      alert(error instanceof Error ? error.message : 'Failed to capture image. Please try again.');
    } finally {
      setIsCapturing(false);
    }
  };

  const isValid = poseDetection?.isValid || false;

  console.log('BodyScanCapture render', { 
    cameraService: bodyScanService.camera,
    isValid,
    poseDetection 
  });
  
  return (
    <CameraContainer>
      <CameraView>
        <CameraPreview cameraService={bodyScanService.camera} />
      </CameraView>
      
      {/* Temporarily disabled due to TypeScript issues
      {contourData && contourData.points && (
        <ContourOverlay
          isValid={isValid}
          contourPoints={contourData.points}
          width={contourData.displayDimensions?.width || window.innerWidth}
          height={contourData.displayDimensions?.height || window.innerHeight}
          key={`contour-${isValid}`} // Force re-render when validity changes
        />
      )} */}
      
      <PoseOverlayCanvas ref={canvasRef} />
      
      {showAlignment && (
        <AlignmentIndicator alignmentService={alignmentService.current} />
      )}
      
      <CloseButton onClick={onCancel}>×</CloseButton>
      
      <PoseTypeHeading>
        {pose === BodyScanPose.FRONT ? 'Front Pose' : 'Side Pose'}
      </PoseTypeHeading>

      {(!poseDetection || (poseDetection && !poseDetection.isValid)) && (
        <FeedbackOverlay>
          <FeedbackList>
            {(poseDetection?.feedback || ['Step into camera view']).map((msg, idx) => {
              let className = 'warning';
              let formattedMsg = msg;
              
              // Format messages and add appropriate classes
              if (msg.toLowerCase().includes('move') && msg.toLowerCase().includes('left')) {
                className = 'move-left';
                formattedMsg = 'Move left';
              } else if (msg.toLowerCase().includes('move') && msg.toLowerCase().includes('right')) {
                className = 'move-right';
                formattedMsg = 'Move right';
              } else if (msg.toLowerCase().includes('step back') || msg.toLowerCase().includes('move back')) {
                className = 'move-back';
                formattedMsg = 'Step back';
              } else if (msg.toLowerCase().includes('step forward') || msg.toLowerCase().includes('move forward')) {
                className = 'move-forward';
                formattedMsg = 'Step forward';
              } else if (msg.toLowerCase().includes('arms')) {
                formattedMsg = 'Arms away from body';
              } else if (msg.toLowerCase().includes('head')) {
                formattedMsg = 'Head in frame';
              } else if (msg.toLowerCase().includes('ankles')) {
                formattedMsg = 'Show ankles';
              } else if (msg.toLowerCase().includes('tilt')) {
                // Alignment feedback
                formattedMsg = msg;
              }
              
              return <li key={idx} className={className}>{formattedMsg}</li>;
            })}
          </FeedbackList>
        </FeedbackOverlay>
      )}

      {countdown !== null && (
        <CountdownOverlay key={countdown}>
          {countdown || 'GO!'}
        </CountdownOverlay>
      )}

      <ControlsOverlay>
        <CancelButton onClick={onCancel} disabled={isCapturing}>
          Cancel
        </CancelButton>
        <CaptureButton 
          $isValid={isValid}
          onClick={handleCapture}
          disabled={!isValid || isCapturing}
        >
          {isCapturing ? 'Capturing...' : 'Capture'}
        </CaptureButton>
      </ControlsOverlay>

      {isCapturing && <LoadingOverlay isVisible={true} />}
    </CameraContainer>
  );
};