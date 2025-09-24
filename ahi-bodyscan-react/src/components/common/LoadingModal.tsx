import React from 'react';
import styled, { keyframes } from 'styled-components';
import { theme } from '../../styles/theme';
import { Modal } from './Modal';

interface LoadingModalProps {
  isOpen: boolean;
  text?: string;
}

const spin = keyframes`
  0% {
    transform: rotate(0deg);
  }
  100% {
    transform: rotate(360deg);
  }
`;

const pulse = keyframes`
  0%, 100% {
    opacity: 1;
  }
  50% {
    opacity: 0.5;
  }
`;

const LoadingContainer = styled.div`
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  padding: ${theme.dimensions.paddingXLarge};
  min-height: 300px;
`;

const SpinnerContainer = styled.div`
  position: relative;
  width: 80px;
  height: 80px;
  margin-bottom: ${theme.dimensions.paddingXLarge};
`;

const Spinner = styled.div`
  position: absolute;
  width: 100%;
  height: 100%;
  border: 3px solid ${theme.colors.divider};
  border-top-color: ${theme.colors.primaryPurple};
  border-radius: 50%;
  animation: ${spin} 1s linear infinite;
`;

const InnerSpinner = styled.div`
  position: absolute;
  top: 10px;
  left: 10px;
  right: 10px;
  bottom: 10px;
  border: 3px solid ${theme.colors.divider};
  border-bottom-color: ${theme.colors.primaryBlue};
  border-radius: 50%;
  animation: ${spin} 0.8s linear infinite reverse;
`;

const LoadingText = styled.p`
  font-size: ${theme.typography.headingSmall.fontSize};
  font-weight: ${theme.typography.headingSmall.fontWeight};
  color: ${theme.colors.textPrimary};
  text-align: center;
  margin: 0;
  animation: ${pulse} 2s ease-in-out infinite;
`;

const SubText = styled.p`
  font-size: ${theme.typography.bodyMedium.fontSize};
  color: ${theme.colors.textSecondary};
  text-align: center;
  margin-top: ${theme.dimensions.paddingSmall};
`;

const DotsContainer = styled.span`
  display: inline-block;
  width: 20px;
  text-align: left;
`;

const Dot = styled.span<{ delay: number }>`
  opacity: 0;
  animation: ${pulse} 1.4s ease-in-out infinite;
  animation-delay: ${props => props.delay}s;
`;

const LoadingDots: React.FC = () => (
  <DotsContainer>
    <Dot delay={0}>.</Dot>
    <Dot delay={0.2}>.</Dot>
    <Dot delay={0.4}>.</Dot>
  </DotsContainer>
);

export const LoadingModal: React.FC<LoadingModalProps> = ({
  isOpen,
  text = 'Loading'
}) => {
  const handleClose = () => {
    // Prevent closing by clicking outside or close button
  };

  return (
    <Modal
      isOpen={isOpen}
      onClose={handleClose}
      size="small"
      showCloseButton={false}
    >
      <LoadingContainer>
        <SpinnerContainer>
          <Spinner />
          <InnerSpinner />
        </SpinnerContainer>
        <LoadingText>
          {text}<LoadingDots />
        </LoadingText>
        <SubText>Please wait</SubText>
      </LoadingContainer>
    </Modal>
  );
};