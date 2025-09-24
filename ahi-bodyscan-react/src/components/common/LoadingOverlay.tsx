import React from 'react';
import styled, { keyframes } from 'styled-components';
import { theme } from '../../styles/theme';

interface LoadingOverlayProps {
  isVisible: boolean;
  text?: string;
  fullScreen?: boolean;
  dark?: boolean;
}

const fadeIn = keyframes`
  from {
    opacity: 0;
  }
  to {
    opacity: 1;
  }
`;

const progressAnimation = keyframes`
  0% {
    transform: translateX(-100%);
  }
  100% {
    transform: translateX(400%);
  }
`;

const Container = styled.div<{ isVisible: boolean; fullScreen: boolean; dark: boolean }>`
  position: ${props => props.fullScreen ? 'fixed' : 'absolute'};
  top: 0;
  left: 0;
  right: 0;
  bottom: 0;
  background: ${props => props.dark ? theme.colors.backgroundDark : theme.colors.backgroundWhite};
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  z-index: 999;
  opacity: ${props => props.isVisible ? 1 : 0};
  visibility: ${props => props.isVisible ? 'visible' : 'hidden'};
  transition: opacity ${theme.transitions.normal}, visibility ${theme.transitions.normal};
  animation: ${fadeIn} ${theme.transitions.normal};
`;

const LoadingText = styled.div<{ dark: boolean }>`
  font-size: ${theme.typography.headingMedium.fontSize};
  font-weight: ${theme.typography.headingMedium.fontWeight};
  color: ${props => props.dark ? theme.colors.textWhite : theme.colors.textPrimary};
  margin-bottom: ${theme.dimensions.paddingLarge};
`;

const ProgressContainer = styled.div`
  width: 200px;
  height: 2px;
  background: rgba(255, 255, 255, 0.2);
  border-radius: ${theme.dimensions.radiusSmall};
  overflow: hidden;
  position: relative;
`;

const ProgressBar = styled.div<{ dark: boolean }>`
  width: 40%;
  height: 100%;
  background: ${props => props.dark ? theme.colors.primaryBlue : theme.colors.primaryPurple};
  border-radius: ${theme.dimensions.radiusSmall};
  animation: ${progressAnimation} 1.5s ease-in-out infinite;
`;

const IconContainer = styled.div<{ dark: boolean }>`
  width: 80px;
  height: 80px;
  margin-bottom: ${theme.dimensions.paddingLarge};
  color: ${props => props.dark ? theme.colors.primaryBlue : theme.colors.primaryPurple};
  
  svg {
    width: 100%;
    height: 100%;
  }
`;

const ProcessingIcon = () => (
  <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2">
    <path d="M13 2L3 14h9l-1 8 10-12h-9l1-8z" />
  </svg>
);

export const LoadingOverlay: React.FC<LoadingOverlayProps> = ({
  isVisible,
  text = 'Loading...',
  fullScreen = true,
  dark = false,
}) => {
  return (
    <Container isVisible={isVisible} fullScreen={fullScreen} dark={dark}>
      <IconContainer dark={dark}>
        <ProcessingIcon />
      </IconContainer>
      <LoadingText dark={dark}>{text}</LoadingText>
      <ProgressContainer>
        <ProgressBar dark={dark} />
      </ProgressContainer>
    </Container>
  );
};

// Specific loading screens matching Flutter app
export const InitializingScreen: React.FC = () => (
  <LoadingOverlay
    isVisible={true}
    text="Initializing"
    fullScreen={true}
    dark={true}
  />
);

export const ProcessingScreen: React.FC = () => (
  <LoadingOverlay
    isVisible={true}
    text="Processing"
    fullScreen={true}
    dark={true}
  />
);