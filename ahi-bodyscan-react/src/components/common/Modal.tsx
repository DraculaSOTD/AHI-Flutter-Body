import React, { useEffect } from 'react';
import styled from 'styled-components';
import { theme } from '../../styles/theme';

interface ModalProps {
  isOpen: boolean;
  onClose: () => void;
  title?: string;
  children: React.ReactNode;
  size?: 'small' | 'medium' | 'large' | 'fullscreen';
  showCloseButton?: boolean;
}

const ModalOverlay = styled.div<{ isOpen: boolean }>`
  position: fixed;
  top: 0;
  left: 0;
  right: 0;
  bottom: 0;
  background: rgba(0, 0, 0, 0.5);
  display: flex;
  align-items: center;
  justify-content: center;
  z-index: 1000;
  opacity: ${props => props.isOpen ? 1 : 0};
  visibility: ${props => props.isOpen ? 'visible' : 'hidden'};
  transition: opacity ${theme.transitions.normal}, visibility ${theme.transitions.normal};
  padding: ${theme.dimensions.paddingLarge};
`;

const modalSizes = {
  small: '400px',
  medium: '600px',
  large: '800px',
  fullscreen: '100%',
};

const ModalContent = styled.div<{ size: string }>`
  background: ${theme.colors.backgroundWhite};
  border-radius: ${theme.dimensions.radiusLarge};
  max-width: ${props => modalSizes[props.size as keyof typeof modalSizes]};
  width: 100%;
  max-height: ${props => props.size === 'fullscreen' ? '100%' : '90vh'};
  height: ${props => props.size === 'fullscreen' ? '100%' : 'auto'};
  display: flex;
  flex-direction: column;
  position: relative;
  box-shadow: ${theme.shadows.large};
  animation: slideUp ${theme.transitions.normal};
  
  @media (max-width: ${theme.breakpoints.mobile}) {
    max-width: 100%;
    max-height: 100%;
    height: 100%;
    border-radius: 0;
  }
`;

const ModalHeader = styled.div`
  padding: ${theme.dimensions.paddingLarge};
  border-bottom: 1px solid ${theme.colors.divider};
  display: flex;
  align-items: center;
  justify-content: space-between;
  
  h2 {
    font-size: ${theme.typography.headingMedium.fontSize};
    font-weight: ${theme.typography.headingMedium.fontWeight};
    color: ${theme.colors.textPrimary};
    margin: 0;
  }
`;

const CloseButton = styled.button`
  width: 32px;
  height: 32px;
  border-radius: ${theme.dimensions.radiusSmall};
  display: flex;
  align-items: center;
  justify-content: center;
  color: ${theme.colors.textTertiary};
  transition: all ${theme.transitions.fast};
  
  &:hover {
    background: ${theme.colors.backgroundLight};
    color: ${theme.colors.textPrimary};
  }
  
  svg {
    width: 20px;
    height: 20px;
  }
`;

const ModalBody = styled.div`
  flex: 1;
  padding: ${theme.dimensions.paddingLarge};
  overflow-y: auto;
`;

const ModalFooter = styled.div`
  padding: ${theme.dimensions.paddingLarge};
  border-top: 1px solid ${theme.colors.divider};
  display: flex;
  align-items: center;
  justify-content: flex-end;
  gap: ${theme.dimensions.paddingMedium};
`;

export const Modal: React.FC<ModalProps> = ({
  isOpen,
  onClose,
  title,
  children,
  size = 'medium',
  showCloseButton = true,
}) => {
  useEffect(() => {
    if (isOpen) {
      document.body.classList.add('no-scroll');
    } else {
      document.body.classList.remove('no-scroll');
    }
    
    return () => {
      document.body.classList.remove('no-scroll');
    };
  }, [isOpen]);
  
  const handleOverlayClick = (e: React.MouseEvent) => {
    if (e.target === e.currentTarget) {
      onClose();
    }
  };
  
  return (
    <ModalOverlay isOpen={isOpen} onClick={handleOverlayClick}>
      <ModalContent size={size}>
        {(title || showCloseButton) && (
          <ModalHeader>
            {title && <h2>{title}</h2>}
            {showCloseButton && (
              <CloseButton onClick={onClose}>
                <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2">
                  <path d="M18 6L6 18M6 6l12 12" />
                </svg>
              </CloseButton>
            )}
          </ModalHeader>
        )}
        <ModalBody>{children}</ModalBody>
      </ModalContent>
    </ModalOverlay>
  );
};

export { ModalBody, ModalFooter };