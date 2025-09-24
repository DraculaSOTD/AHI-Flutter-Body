import React, { useEffect, useRef } from 'react';
import styled from 'styled-components';
import { ContourService, ContourPoint } from '../../../services/bodyScan';

const ContourCanvas = styled.canvas`
  position: absolute;
  top: 50%;
  left: 50%;
  transform: translate(-50%, -50%);
  pointer-events: none;
  z-index: 12;
`;

interface ContourOverlayProps {
  isValid: boolean;
  contourPoints: ContourPoint[] | null;
  width: number;
  height: number;
}

export const ContourOverlay: React.FC<ContourOverlayProps> = ({
  isValid,
  contourPoints,
  width,
  height
}) => {
  const canvasRef = useRef<HTMLCanvasElement>(null);
  const contourService = ContourService.getInstance();
  const animationFrameRef = useRef<number | undefined>(undefined);
  
  useEffect(() => {
    const canvas = canvasRef.current;
    if (!canvas || !contourPoints || contourPoints.length === 0) return;
    
    const ctx = canvas.getContext('2d');
    if (!ctx) return;
    
    // Set canvas size
    canvas.width = width;
    canvas.height = height;
    
    // Animation function for smooth updates
    const animate = () => {
      // Clear the entire canvas first
      ctx.clearRect(0, 0, canvas.width, canvas.height);
      
      // Get contour style based on validity
      const style = contourService.getContourStyle(isValid);
      
      // Draw the contour
      contourService.drawContour(ctx, contourPoints, style);
      
      // Continue animation
      animationFrameRef.current = requestAnimationFrame(animate);
    };
    
    // Start animation
    animate();
    
    // Cleanup
    return () => {
      if (animationFrameRef.current) {
        cancelAnimationFrame(animationFrameRef.current);
      }
    };
  }, [isValid, contourPoints, width, height, contourService]);
  
  return <ContourCanvas ref={canvasRef} />;
};