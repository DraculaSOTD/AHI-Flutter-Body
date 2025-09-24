import React from 'react';
import styled from 'styled-components';
import { theme } from '../../styles/theme';

interface ProgressBarProps {
  progress: number; // 0-100
  height?: string;
  color?: string;
  backgroundColor?: string;
  showLabel?: boolean;
  label?: string;
  animate?: boolean;
}

const ProgressContainer = styled.div<{ $height: string; $backgroundColor: string }>`
  width: 100%;
  height: ${props => props.$height};
  background-color: ${props => props.$backgroundColor};
  border-radius: ${theme.dimensions.radiusPill};
  overflow: hidden;
  position: relative;
`;

const ProgressFill = styled.div<{ 
  $progress: number; 
  $color: string; 
  $animate: boolean;
}>`
  height: 100%;
  width: ${props => props.$progress}%;
  background: ${props => props.$color};
  border-radius: ${theme.dimensions.radiusPill};
  transition: ${props => props.$animate ? `width ${theme.transitions.normal}` : 'none'};
  position: relative;
  
  &::after {
    content: '';
    position: absolute;
    top: 0;
    left: 0;
    bottom: 0;
    right: 0;
    background: linear-gradient(
      90deg,
      rgba(255, 255, 255, 0) 0%,
      rgba(255, 255, 255, 0.2) 50%,
      rgba(255, 255, 255, 0) 100%
    );
    animation: ${props => props.$animate ? 'shimmer 2s ease-in-out infinite' : 'none'};
  }
  
  @keyframes shimmer {
    0% {
      transform: translateX(-100%);
    }
    100% {
      transform: translateX(100%);
    }
  }
`;

const ProgressLabel = styled.div`
  position: absolute;
  top: 50%;
  left: 50%;
  transform: translate(-50%, -50%);
  font-size: ${theme.typography.bodySmall.fontSize};
  font-weight: 600;
  color: ${theme.colors.textPrimary};
  white-space: nowrap;
  z-index: 1;
`;

export const ProgressBar: React.FC<ProgressBarProps> = ({
  progress,
  height = '8px',
  color = theme.gradients.primary,
  backgroundColor = theme.colors.backgroundLight,
  showLabel = false,
  label,
  animate = true
}) => {
  // Ensure progress is between 0 and 100
  const clampedProgress = Math.min(Math.max(progress, 0), 100);
  
  return (
    <ProgressContainer $height={height} $backgroundColor={backgroundColor}>
      <ProgressFill 
        $progress={clampedProgress} 
        $color={color}
        $animate={animate}
      />
      {showLabel && (
        <ProgressLabel>
          {label || `${Math.round(clampedProgress)}%`}
        </ProgressLabel>
      )}
    </ProgressContainer>
  );
};