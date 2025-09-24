import React, { useEffect, useRef, useState } from 'react';
import styled from 'styled-components';
import { CameraService, CameraState } from '../../services/camera/CameraService';
import { theme } from '../../styles/theme';

interface CameraPreviewProps {
  cameraService: CameraService;
  mirrored?: boolean;
  onError?: (error: Error) => void;
}

const PreviewContainer = styled.div<{ mirrored: boolean }>`
  width: 100%;
  height: 100%;
  background: ${theme.colors.backgroundDark};
  position: relative;
  overflow: hidden;
  
  video {
    width: 100%;
    height: 100%;
    object-fit: cover;
    transform: ${props => props.mirrored ? 'scaleX(-1)' : 'none'};
  }
`;

const LoadingOverlay = styled.div`
  position: absolute;
  top: 0;
  left: 0;
  right: 0;
  bottom: 0;
  display: flex;
  align-items: center;
  justify-content: center;
  background: ${theme.colors.backgroundDark};
  color: ${theme.colors.textWhite};
  font-size: ${theme.typography.bodyLarge.fontSize};
`;

const ErrorOverlay = styled.div`
  position: absolute;
  top: 0;
  left: 0;
  right: 0;
  bottom: 0;
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  background: ${theme.colors.backgroundDark};
  color: ${theme.colors.textWhite};
  padding: ${theme.dimensions.paddingLarge};
  text-align: center;
  
  .error-icon {
    width: 64px;
    height: 64px;
    margin-bottom: ${theme.dimensions.paddingLarge};
    color: ${theme.colors.error};
  }
  
  .error-message {
    font-size: ${theme.typography.bodyLarge.fontSize};
    margin-bottom: ${theme.dimensions.paddingMedium};
  }
  
  .error-details {
    font-size: ${theme.typography.bodyMedium.fontSize};
    color: ${theme.colors.textTertiary};
  }
`;

export const CameraPreview: React.FC<CameraPreviewProps> = ({
  cameraService,
  mirrored = false,
  onError
}) => {
  const videoRef = useRef<HTMLVideoElement>(null);
  const [cameraState, setCameraState] = useState<CameraState>(cameraService.state);
  const [error, setError] = useState<string | null>(null);
  
  console.log('CameraPreview mounted', { cameraService, initialState: cameraService.state });
  
  useEffect(() => {
    // Subscribe to camera state changes
    const handleStateChange = (state: CameraState) => {
      console.log('Camera state changed:', state);
      setCameraState(state);
      
      if (state === CameraState.ERROR) {
        setError('Camera access failed. Please check permissions.');
      } else if (state === CameraState.STREAMING) {
        setError(null);
      }
    };
    
    cameraService.onStateChange(handleStateChange);
    
    // Connect video element to camera service
    const connectVideo = () => {
      if (videoRef.current && cameraService.videoElement && cameraService.videoElement.srcObject) {
        console.log('Connecting video element to camera service');
        videoRef.current.srcObject = cameraService.videoElement.srcObject;
      } else if (videoRef.current && cameraService.stream) {
        console.log('Connecting stream directly to video element');
        videoRef.current.srcObject = cameraService.stream;
      } else {
        console.log('Video element or camera service not ready', {
          videoRef: videoRef.current,
          cameraVideoElement: cameraService.videoElement,
          stream: cameraService.stream
        });
      }
    };
    
    // Try to connect immediately
    connectVideo();
    
    // Also try when state changes to streaming
    if (cameraState === CameraState.STREAMING) {
      setTimeout(connectVideo, 100);
    }
    
    return () => {
      // Cleanup is handled by camera service
    };
  }, [cameraService]);
  
  useEffect(() => {
    // Update video element when stream changes
    console.log('Stream update effect', { 
      hasVideoRef: !!videoRef.current, 
      hasStream: !!cameraService.stream,
      cameraState 
    });
    if (videoRef.current && cameraService.stream) {
      console.log('Setting stream on video element');
      videoRef.current.srcObject = cameraService.stream;
      // Register video element with camera service
      cameraService.setVideoElement(videoRef.current);
    }
  }, [cameraService.stream, cameraState, cameraService]);
  
  const renderContent = () => {
    switch (cameraState) {
      case CameraState.UNINITIALIZED:
      case CameraState.INITIALIZING:
      case CameraState.READY:
        return (
          <LoadingOverlay>
            <div>Initializing camera...</div>
          </LoadingOverlay>
        );
        
      case CameraState.ERROR:
        return (
          <ErrorOverlay>
            <svg className="error-icon" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2">
              <circle cx="12" cy="12" r="10" />
              <line x1="12" y1="8" x2="12" y2="12" />
              <line x1="12" y1="16" x2="12.01" y2="16" />
            </svg>
            <div className="error-message">Camera Error</div>
            <div className="error-details">{error || 'Unable to access camera'}</div>
          </ErrorOverlay>
        );
        
      case CameraState.STREAMING:
        return (
          <video
            ref={videoRef}
            autoPlay
            playsInline
            muted
          />
        );
        
      default:
        return null;
    }
  };
  
  return (
    <PreviewContainer mirrored={mirrored}>
      {renderContent()}
    </PreviewContainer>
  );
};