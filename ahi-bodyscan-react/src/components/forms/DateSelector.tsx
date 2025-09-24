import React, { useState, useEffect } from 'react';
import styled from 'styled-components';
import { theme } from '../../styles/theme';

interface DateSelectorProps {
  value: string; // ISO date string (YYYY-MM-DD)
  onChange: (date: string) => void;
  label?: string;
  required?: boolean;
  minYear?: number;
  maxYear?: number;
}

const Container = styled.div`
  width: 100%;
`;

const Label = styled.label`
  display: block;
  font-size: ${theme.typography.bodyMedium.fontSize};
  font-weight: 500;
  color: ${theme.colors.textPrimary};
  margin-bottom: ${theme.dimensions.paddingSmall};
`;

const SelectGroup = styled.div`
  display: grid;
  grid-template-columns: 2fr 1fr 1.5fr;
  gap: ${theme.dimensions.paddingSmall};
  
  @media (max-width: ${theme.breakpoints.mobile}) {
    grid-template-columns: 1fr;
  }
`;

const StyledSelect = styled.select`
  width: 100%;
  padding: ${theme.dimensions.paddingMedium};
  font-size: ${theme.typography.bodyMedium.fontSize};
  color: ${theme.colors.textPrimary};
  background: ${theme.colors.backgroundWhite};
  border: 1px solid ${theme.colors.borderLight};
  border-radius: ${theme.dimensions.radiusMedium};
  cursor: pointer;
  transition: all ${theme.transitions.fast};
  
  &:focus {
    outline: none;
    border-color: ${theme.colors.primaryBlue};
  }
  
  &:hover {
    border-color: ${theme.colors.borderAccent};
  }
`;

const months = [
  { value: '01', label: 'January' },
  { value: '02', label: 'February' },
  { value: '03', label: 'March' },
  { value: '04', label: 'April' },
  { value: '05', label: 'May' },
  { value: '06', label: 'June' },
  { value: '07', label: 'July' },
  { value: '08', label: 'August' },
  { value: '09', label: 'September' },
  { value: '10', label: 'October' },
  { value: '11', label: 'November' },
  { value: '12', label: 'December' },
];

export const DateSelector: React.FC<DateSelectorProps> = ({
  value,
  onChange,
  label = 'Date of Birth',
  required = false,
  minYear = 1900,
  maxYear = new Date().getFullYear()
}) => {
  // Parse the ISO date string
  const [year, setYear] = useState('');
  const [month, setMonth] = useState('');
  const [day, setDay] = useState('');

  useEffect(() => {
    if (value) {
      const date = new Date(value);
      if (!isNaN(date.getTime())) {
        setYear(date.getFullYear().toString());
        setMonth(String(date.getMonth() + 1).padStart(2, '0'));
        setDay(String(date.getDate()).padStart(2, '0'));
      }
    }
  }, [value]);

  const getDaysInMonth = (year: string, month: string) => {
    if (!year || !month) return 31;
    return new Date(parseInt(year), parseInt(month), 0).getDate();
  };

  const handleDateChange = (newYear: string, newMonth: string, newDay: string) => {
    if (newYear && newMonth && newDay) {
      const dateStr = `${newYear}-${newMonth.padStart(2, '0')}-${newDay.padStart(2, '0')}`;
      onChange(dateStr);
    } else {
      onChange('');
    }
  };

  const handleMonthChange = (newMonth: string) => {
    setMonth(newMonth);
    const maxDays = getDaysInMonth(year, newMonth);
    if (parseInt(day) > maxDays) {
      setDay(maxDays.toString());
      handleDateChange(year, newMonth, maxDays.toString());
    } else {
      handleDateChange(year, newMonth, day);
    }
  };

  const handleYearChange = (newYear: string) => {
    setYear(newYear);
    const maxDays = getDaysInMonth(newYear, month);
    if (parseInt(day) > maxDays) {
      setDay(maxDays.toString());
      handleDateChange(newYear, month, maxDays.toString());
    } else {
      handleDateChange(newYear, month, day);
    }
  };

  const handleDayChange = (newDay: string) => {
    setDay(newDay);
    handleDateChange(year, month, newDay);
  };

  const years = Array.from({ length: maxYear - minYear + 1 }, (_, i) => maxYear - i);
  const days = Array.from({ length: getDaysInMonth(year, month) }, (_, i) => i + 1);

  return (
    <Container>
      {label && <Label>{label}</Label>}
      <SelectGroup>
        <StyledSelect
          value={month}
          onChange={(e) => handleMonthChange(e.target.value)}
          required={required}
        >
          <option value="">Month</option>
          {months.map(m => (
            <option key={m.value} value={m.value}>{m.label}</option>
          ))}
        </StyledSelect>
        
        <StyledSelect
          value={day}
          onChange={(e) => handleDayChange(e.target.value)}
          required={required}
          disabled={!month}
        >
          <option value="">Day</option>
          {days.map(d => (
            <option key={d} value={d}>{d}</option>
          ))}
        </StyledSelect>
        
        <StyledSelect
          value={year}
          onChange={(e) => handleYearChange(e.target.value)}
          required={required}
        >
          <option value="">Year</option>
          {years.map(y => (
            <option key={y} value={y}>{y}</option>
          ))}
        </StyledSelect>
      </SelectGroup>
    </Container>
  );
};