import React, { useState } from 'react';
import styled from 'styled-components';
import { theme } from '../../styles/theme';

interface SelectOption {
  value: string;
  label: string;
}

interface SelectProps extends Omit<React.SelectHTMLAttributes<HTMLSelectElement>, 'onChange'> {
  label?: string;
  error?: string;
  options: SelectOption[];
  placeholder?: string;
  onChange?: (value: string) => void;
}

const SelectWrapper = styled.div`
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

const SelectContainer = styled.div<{ hasError: boolean; isFocused: boolean }>`
  position: relative;
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
  
  &::after {
    content: '';
    position: absolute;
    top: 50%;
    right: ${theme.dimensions.paddingMedium};
    transform: translateY(-50%);
    width: 0;
    height: 0;
    border-left: 5px solid transparent;
    border-right: 5px solid transparent;
    border-top: 5px solid ${theme.colors.textTertiary};
    pointer-events: none;
  }
`;

const StyledSelect = styled.select`
  width: 100%;
  height: 100%;
  padding: 0 ${theme.dimensions.paddingXLarge} 0 ${theme.dimensions.paddingMedium};
  font-size: ${theme.typography.bodyMedium.fontSize};
  color: ${theme.colors.textPrimary};
  background: transparent;
  appearance: none;
  cursor: pointer;
  
  &:invalid {
    color: ${theme.colors.textPlaceholder};
  }
`;

const ErrorText = styled.span`
  display: block;
  font-size: ${theme.typography.bodySmall.fontSize};
  color: ${theme.colors.error};
  margin-top: ${theme.dimensions.paddingXSmall};
`;

export const Select: React.FC<SelectProps> = ({
  label,
  error,
  options,
  placeholder,
  onChange,
  onFocus,
  onBlur,
  value,
  ...props
}) => {
  const [isFocused, setIsFocused] = useState(false);
  
  const handleFocus = (e: React.FocusEvent<HTMLSelectElement>) => {
    setIsFocused(true);
    onFocus?.(e);
  };
  
  const handleBlur = (e: React.FocusEvent<HTMLSelectElement>) => {
    setIsFocused(false);
    onBlur?.(e);
  };
  
  const handleChange = (e: React.ChangeEvent<HTMLSelectElement>) => {
    onChange?.(e.target.value);
  };
  
  return (
    <SelectWrapper>
      {label && <Label>{label}</Label>}
      <SelectContainer hasError={!!error} isFocused={isFocused}>
        <StyledSelect
          value={value}
          onChange={handleChange}
          onFocus={handleFocus}
          onBlur={handleBlur}
          required={placeholder ? true : false}
          {...props}
        >
          {placeholder && (
            <option value="" disabled hidden>
              {placeholder}
            </option>
          )}
          {options.map(option => (
            <option key={option.value} value={option.value}>
              {option.label}
            </option>
          ))}
        </StyledSelect>
      </SelectContainer>
      {error && <ErrorText>{error}</ErrorText>}
    </SelectWrapper>
  );
};