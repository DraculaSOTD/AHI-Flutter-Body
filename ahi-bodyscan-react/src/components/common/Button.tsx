import React from 'react';
import styled, { css } from 'styled-components';
import { theme } from '../../styles/theme';

interface ButtonProps extends React.ButtonHTMLAttributes<HTMLButtonElement> {
  variant?: 'primary' | 'outlined' | 'text';
  size?: 'small' | 'medium' | 'large';
  fullWidth?: boolean;
  loading?: boolean;
  icon?: React.ReactNode;
  children: React.ReactNode;
}

const buttonSizes = {
  small: css`
    height: ${theme.dimensions.buttonHeightSmall};
    padding: 0 ${theme.dimensions.paddingMedium};
    font-size: ${theme.typography.bodyMedium.fontSize};
  `,
  medium: css`
    height: ${theme.dimensions.buttonHeightMedium};
    padding: 0 ${theme.dimensions.paddingLarge};
    font-size: ${theme.typography.buttonMedium.fontSize};
  `,
  large: css`
    height: ${theme.dimensions.buttonHeightLarge};
    padding: 0 ${theme.dimensions.paddingXLarge};
    font-size: ${theme.typography.buttonLarge.fontSize};
  `,
};

const buttonVariants = {
  primary: css`
    background: ${theme.gradients.primary};
    color: ${theme.colors.textWhite};
    
    &:hover:not(:disabled) {
      filter: brightness(1.1);
      transform: translateY(-1px);
    }
    
    &:active:not(:disabled) {
      transform: translateY(0);
    }
  `,
  outlined: css`
    background: ${theme.colors.backgroundWhite};
    color: ${theme.colors.primaryPurple};
    border: 2px solid ${theme.colors.primaryPurple};
    
    &:hover:not(:disabled) {
      background: ${theme.colors.backgroundLight};
    }
  `,
  text: css`
    background: transparent;
    color: ${theme.colors.textSecondary};
    
    &:hover:not(:disabled) {
      color: ${theme.colors.textPrimary};
    }
  `,
};

const StyledButton = styled.button<ButtonProps>`
  display: inline-flex;
  align-items: center;
  justify-content: center;
  gap: ${theme.dimensions.paddingSmall};
  font-weight: ${theme.typography.buttonMedium.fontWeight};
  border-radius: ${theme.dimensions.radiusPill};
  transition: all ${theme.transitions.fast};
  white-space: nowrap;
  position: relative;
  overflow: hidden;
  
  ${props => buttonSizes[props.size || 'medium']}
  ${props => buttonVariants[props.variant || 'primary']}
  ${props => props.fullWidth && css`width: 100%;`}
  
  &:disabled {
    opacity: 0.5;
    cursor: not-allowed;
  }
  
  ${props => props.loading && css`
    cursor: wait;
    
    &::after {
      content: '';
      position: absolute;
      width: 20px;
      height: 20px;
      border: 2px solid ${props.variant === 'primary' ? theme.colors.textWhite : theme.colors.primaryPurple};
      border-top-color: transparent;
      border-radius: 50%;
      animation: spin 0.8s linear infinite;
    }
  `}
`;

const IconWrapper = styled.span`
  display: inline-flex;
  align-items: center;
`;

const LoadingSpinner = styled.span`
  display: inline-block;
  width: 16px;
  height: 16px;
  border: 2px solid currentColor;
  border-top-color: transparent;
  border-radius: 50%;
  animation: spin 0.8s linear infinite;
`;

export const Button: React.FC<ButtonProps> = ({
  children,
  variant = 'primary',
  size = 'medium',
  fullWidth = false,
  loading = false,
  icon,
  disabled,
  ...props
}) => {
  return (
    <StyledButton
      variant={variant}
      size={size}
      fullWidth={fullWidth}
      loading={loading}
      disabled={disabled || loading}
      {...props}
    >
      {loading ? (
        <LoadingSpinner />
      ) : (
        <>
          {icon && <IconWrapper>{icon}</IconWrapper>}
          {children}
        </>
      )}
    </StyledButton>
  );
};

// Gradient Primary Button
export const PrimaryGradientButton = styled(Button)`
  background: ${theme.gradients.primary};
  color: ${theme.colors.textWhite};
  font-weight: 600;
  
  &:hover:not(:disabled) {
    box-shadow: ${theme.shadows.medium};
  }
`;

// Outlined Button
export const OutlinedButton = styled(Button).attrs({ variant: 'outlined' })`
  background: ${theme.colors.backgroundWhite};
  border: 2px solid ${theme.colors.primaryPurple};
  color: ${theme.colors.primaryPurple};
`;

// Text Button
export const TextButton = styled(Button).attrs({ variant: 'text' })`
  background: transparent;
  color: ${theme.colors.textSecondary};
  padding: ${theme.dimensions.paddingSmall} ${theme.dimensions.paddingMedium};
`;

// Secondary Button (outlined style)
export const SecondaryButton = styled(Button).attrs({ variant: 'outlined' })`
  background: transparent;
  border: 2px solid ${theme.colors.divider};
  color: ${theme.colors.textWhite};
  
  &:hover:not(:disabled) {
    background: rgba(255, 255, 255, 0.1);
    border-color: ${theme.colors.textWhite};
  }
`;