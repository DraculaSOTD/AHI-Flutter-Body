import React from 'react';
import styled from 'styled-components';
import { theme } from '../../styles/theme';

interface SegmentOption {
  value: string;
  label: string;
  icon?: React.ReactNode;
}

interface SegmentedControlProps {
  options: SegmentOption[];
  value: string;
  onChange: (value: string) => void;
  label?: string;
  fullWidth?: boolean;
}

const ControlWrapper = styled.div`
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

const ControlContainer = styled.div<{ fullWidth: boolean }>`
  display: inline-flex;
  background: ${theme.colors.divider};
  border-radius: ${theme.dimensions.radiusPill};
  padding: 2px;
  width: ${props => props.fullWidth ? '100%' : 'auto'};
`;

const Segment = styled.button<{ isActive: boolean; fullWidth: boolean }>`
  flex: ${props => props.fullWidth ? 1 : 'initial'};
  padding: ${theme.dimensions.paddingSmall} ${theme.dimensions.paddingLarge};
  height: 40px;
  border-radius: ${theme.dimensions.radiusPill};
  background: ${props => props.isActive ? theme.colors.primaryBlue : 'transparent'};
  color: ${props => props.isActive ? theme.colors.textWhite : theme.colors.textSecondary};
  font-size: ${theme.typography.bodyMedium.fontSize};
  font-weight: ${props => props.isActive ? '600' : '500'};
  transition: all ${theme.transitions.fast};
  display: flex;
  align-items: center;
  justify-content: center;
  gap: ${theme.dimensions.paddingSmall};
  white-space: nowrap;
  
  &:hover:not(:disabled) {
    background: ${props => props.isActive ? theme.colors.primaryBlue : theme.colors.backgroundLight};
  }
  
  svg {
    width: 18px;
    height: 18px;
  }
`;

export const SegmentedControl: React.FC<SegmentedControlProps> = ({
  options,
  value,
  onChange,
  label,
  fullWidth = false,
}) => {
  return (
    <ControlWrapper>
      {label && <Label>{label}</Label>}
      <ControlContainer fullWidth={fullWidth}>
        {options.map(option => (
          <Segment
            key={option.value}
            isActive={value === option.value}
            fullWidth={fullWidth}
            onClick={() => onChange(option.value)}
            type="button"
          >
            {option.icon && option.icon}
            {option.label}
          </Segment>
        ))}
      </ControlContainer>
    </ControlWrapper>
  );
};

// Pre-configured Gender Selector
export const GenderSelector: React.FC<{
  value: string;
  onChange: (value: string) => void;
  label?: string;
}> = ({ value, onChange, label = "Gender" }) => {
  const genderOptions: SegmentOption[] = [
    {
      value: 'male',
      label: 'Male',
      icon: (
        <svg viewBox="0 0 24 24" fill="currentColor">
          <path d="M9,9C10.29,9 11.5,9.41 12.47,10.11L17.58,5H13V3H21V11H19V6.41L13.89,11.5C14.59,12.5 15,13.7 15,15A6,6 0 0,1 9,21A6,6 0 0,1 3,15A6,6 0 0,1 9,9M9,11A4,4 0 0,0 5,15A4,4 0 0,0 9,19A4,4 0 0,0 13,15A4,4 0 0,0 9,11Z" />
        </svg>
      )
    },
    {
      value: 'female',
      label: 'Female',
      icon: (
        <svg viewBox="0 0 24 24" fill="currentColor">
          <path d="M12,4A6,6 0 0,1 18,10C18,12.97 15.84,15.44 13,15.92V18H15V20H13V22H11V20H9V18H11V15.92C8.16,15.44 6,12.97 6,10A6,6 0 0,1 12,4M12,6A4,4 0 0,0 8,10A4,4 0 0,0 12,14A4,4 0 0,0 16,10A4,4 0 0,0 12,6Z" />
        </svg>
      )
    }
  ];
  
  return (
    <SegmentedControl
      options={genderOptions}
      value={value}
      onChange={onChange}
      label={label}
      fullWidth
    />
  );
};