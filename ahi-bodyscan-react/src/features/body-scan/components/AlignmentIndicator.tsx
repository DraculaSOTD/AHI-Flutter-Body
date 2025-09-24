import React, { useEffect, useState } from 'react';
import styled from 'styled-components';
import { AlignmentService, AlignmentResult } from '../../../services/bodyScan';
import { theme } from '../../../styles/theme';

const AlignmentContainer = styled.div`
  position: absolute;
  top: ${theme.dimensions.paddingXLarge};
  right: ${theme.dimensions.paddingLarge};
  width: 120px;
  height: 120px;
  background: rgba(0, 0, 0, 0.7);
  backdrop-filter: blur(10px);
  border-radius: 50%;
  padding: 20px;
  z-index: 25;
  display: flex;
  align-items: center;
  justify-content: center;
`;

const AlignmentGrid = styled.div`
  position: relative;
  width: 80px;
  height: 80px;
`;

const CrossHair = styled.div`
  position: absolute;
  width: 100%;
  height: 100%;
  
  &::before,
  &::after {
    content: '';
    position: absolute;
    background: rgba(255, 255, 255, 0.3);
  }
  
  &::before {
    top: 50%;
    left: 0;
    right: 0;
    height: 1px;
    transform: translateY(-50%);
  }
  
  &::after {
    left: 50%;
    top: 0;
    bottom: 0;
    width: 1px;
    transform: translateX(-50%);
  }
`;

const AlignmentDot = styled.div<{ $x: number; $y: number; $isAligned: boolean }>`
  position: absolute;
  width: 20px;
  height: 20px;
  background: ${props => props.$isAligned ? theme.colors.success : theme.colors.error};
  border-radius: 50%;
  left: 50%;
  top: 50%;
  transform: translate(
    calc(-50% + ${props => props.$x * 30}px),
    calc(-50% + ${props => props.$y * 30}px)
  );
  box-shadow: 0 0 10px rgba(0, 0, 0, 0.5);
  transition: all 0.1s ease-out;
`;

const AlignmentScore = styled.div<{ $isAligned: boolean }>`
  position: absolute;
  bottom: -30px;
  left: 50%;
  transform: translateX(-50%);
  font-size: ${theme.typography.bodySmall.fontSize};
  color: ${props => props.$isAligned ? theme.colors.success : theme.colors.warning};
  font-weight: 600;
  white-space: nowrap;
`;

interface AlignmentIndicatorProps {
  alignmentService: AlignmentService;
}

export const AlignmentIndicator: React.FC<AlignmentIndicatorProps> = ({ alignmentService }) => {
  const [alignment, setAlignment] = useState<AlignmentResult | null>(null);
  const [position, setPosition] = useState({ x: 0, y: 0 });
  
  useEffect(() => {
    const updateAlignment = () => {
      const result = alignmentService.getAlignment();
      setAlignment(result);
      
      const pos = alignmentService.getAlignmentIndicatorPosition();
      setPosition(pos);
    };
    
    // Update alignment status frequently
    const interval = setInterval(updateAlignment, 100);
    
    return () => clearInterval(interval);
  }, [alignmentService]);
  
  if (!alignment) return null;
  
  const score = alignmentService.getAlignmentScore();
  
  return (
    <AlignmentContainer>
      <AlignmentGrid>
        <CrossHair />
        <AlignmentDot
          $x={position.x}
          $y={position.y}
          $isAligned={alignment.isAligned}
        />
        <AlignmentScore $isAligned={alignment.isAligned}>
          {alignment.isAligned ? 'Aligned' : `${score}%`}
        </AlignmentScore>
      </AlignmentGrid>
    </AlignmentContainer>
  );
};