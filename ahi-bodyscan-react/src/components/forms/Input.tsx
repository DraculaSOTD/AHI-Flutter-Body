import React, { useState } from 'react';
import styled from 'styled-components';
import { theme } from '../../styles/theme';

interface InputProps extends React.InputHTMLAttributes<HTMLInputElement> {
  label?: string;
  error?: string;
  icon?: React.ReactNode;
  rightElement?: React.ReactNode;
  showClear?: boolean;
  onClear?: () => void;
}

const InputWrapper = styled.div`
  width: 100%;
  margin-bottom: ${theme.dimensions.paddingMedium};
`;

const Label = styled.label`
  display: block;
  font-size: ${theme.typography.bodyMedium.fontSize};
  color: ${theme.colors.textLabel};
  margin-bottom: ${theme.dimensions.paddingSmall};
  font-weight: 500;
`;

const InputContainer = styled.div<{ hasError: boolean; isFocused: boolean }>`
  position: relative;
  display: flex;
  align-items: center;
  background: ${theme.colors.backgroundWhite};
  border: 1px solid ${props => 
    props.hasError 
      ? theme.colors.error 
      : props.isFocused 
      ? theme.colors.primaryBlue 
      : theme.colors.borderLight
  };
  border-radius: ${theme.dimensions.radiusMedium};
  height: 50px;
  transition: all ${theme.transitions.fast};
  
  &:hover {
    border-color: ${props => !props.hasError && theme.colors.borderAccent};
  }
`;

const StyledInput = styled.input`
  flex: 1;
  padding: 0 ${theme.dimensions.paddingMedium};
  font-size: ${theme.typography.bodyMedium.fontSize};
  color: ${theme.colors.textPrimary};
  background: transparent;
  border: none;
  outline: none;
  height: 100%;
  width: 100%;
  
  &::placeholder {
    color: ${theme.colors.textPlaceholder};
  }
  
  &:focus {
    outline: none;
  }
`;

const IconWrapper = styled.div`
  padding-left: ${theme.dimensions.paddingMedium};
  color: ${theme.colors.textTertiary};
  display: flex;
  align-items: center;
  
  svg {
    width: 20px;
    height: 20px;
  }
`;

const ClearButton = styled.button`
  padding: 0 ${theme.dimensions.paddingMedium};
  color: ${theme.colors.textTertiary};
  display: flex;
  align-items: center;
  
  &:hover {
    color: ${theme.colors.textSecondary};
  }
  
  svg {
    width: 18px;
    height: 18px;
  }
`;

const ErrorText = styled.span`
  display: block;
  font-size: ${theme.typography.bodySmall.fontSize};
  color: ${theme.colors.error};
  margin-top: ${theme.dimensions.paddingXSmall};
`;

export const Input: React.FC<InputProps> = ({
  label,
  error,
  icon,
  rightElement,
  showClear,
  onClear,
  onFocus,
  onBlur,
  onChange,
  value,
  ...props
}) => {
  const [isFocused, setIsFocused] = useState(false);
  
  const handleFocus = (e: React.FocusEvent<HTMLInputElement>) => {
    setIsFocused(true);
    onFocus?.(e);
  };
  
  const handleBlur = (e: React.FocusEvent<HTMLInputElement>) => {
    setIsFocused(false);
    onBlur?.(e);
  };
  
  return (
    <InputWrapper>
      {label && <Label>{label}</Label>}
      <InputContainer hasError={!!error} isFocused={isFocused}>
        {icon && <IconWrapper>{icon}</IconWrapper>}
        <StyledInput
          {...props}
          value={value}
          onChange={onChange}
          onFocus={handleFocus}
          onBlur={handleBlur}
        />
        {showClear && value && (
          <ClearButton type="button" onClick={onClear}>
            <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2">
              <circle cx="12" cy="12" r="10" />
              <path d="M15 9l-6 6M9 9l6 6" />
            </svg>
          </ClearButton>
        )}
        {rightElement}
      </InputContainer>
      {error && <ErrorText>{error}</ErrorText>}
    </InputWrapper>
  );
};

// Password Input with visibility toggle
interface PasswordInputProps extends Omit<InputProps, 'type'> {
  showVisibilityToggle?: boolean;
}

const VisibilityButton = styled.button`
  padding: 0 ${theme.dimensions.paddingMedium};
  color: ${theme.colors.textTertiary};
  display: flex;
  align-items: center;
  
  &:hover {
    color: ${theme.colors.textSecondary};
  }
  
  svg {
    width: 20px;
    height: 20px;
  }
`;

export const PasswordInput: React.FC<PasswordInputProps> = ({
  showVisibilityToggle = true,
  ...props
}) => {
  const [showPassword, setShowPassword] = useState(false);
  
  const toggleVisibility = () => setShowPassword(!showPassword);
  
  return (
    <Input
      {...props}
      type={showPassword ? 'text' : 'password'}
      icon={
        <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2">
          <rect x="3" y="11" width="18" height="11" rx="2" ry="2" />
          <path d="M7 11V7a5 5 0 0110 0v4" />
        </svg>
      }
      rightElement={
        showVisibilityToggle ? (
          <VisibilityButton type="button" onClick={toggleVisibility}>
            {showPassword ? (
              <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2">
                <path d="M17.94 17.94A10.07 10.07 0 0112 20c-7 0-11-8-11-8a18.45 18.45 0 015.06-5.94M9.9 4.24A9.12 9.12 0 0112 4c7 0 11 8 11 8a18.5 18.5 0 01-2.16 3.19m-6.72-1.07a3 3 0 11-4.24-4.24" />
                <line x1="1" y1="1" x2="23" y2="23" />
              </svg>
            ) : (
              <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2">
                <path d="M1 12s4-8 11-8 11 8 11 8-4 8-11 8-11-8-11-8z" />
                <circle cx="12" cy="12" r="3" />
              </svg>
            )}
          </VisibilityButton>
        ) : null
      }
    />
  );
};