import React from 'react';
import styled from 'styled-components';
import { theme } from '../../../styles/theme';

interface MetricsIconRowProps {
  isVisible: boolean;
}

const Container = styled.div<{ isVisible: boolean }>`
  display: flex;
  justify-content: center;
  gap: ${theme.dimensions.paddingLarge};
  padding: ${theme.dimensions.paddingLarge};
  opacity: ${props => props.isVisible ? 1 : 0};
  transition: opacity ${theme.transitions.normal};
  
  @media (max-width: ${theme.breakpoints.mobile}) {
    gap: ${theme.dimensions.paddingMedium};
  }
`;

const MetricIcon = styled.div`
  display: flex;
  flex-direction: column;
  align-items: center;
  gap: ${theme.dimensions.paddingSmall};
`;

const IconCircle = styled.div`
  width: 60px;
  height: 60px;
  border-radius: 50%;
  background: #4A90E2;
  display: flex;
  align-items: center;
  justify-content: center;
  color: ${theme.colors.textWhite};
  font-size: 24px;
  
  @media (max-width: ${theme.breakpoints.mobile}) {
    width: 50px;
    height: 50px;
    font-size: 20px;
  }
  
  svg {
    width: 28px;
    height: 28px;
    
    @media (max-width: ${theme.breakpoints.mobile}) {
      width: 24px;
      height: 24px;
    }
  }
`;

const MetricLabel = styled.span`
  font-size: ${theme.typography.bodySmall.fontSize};
  color: ${theme.colors.textSecondary};
  font-weight: 500;
`;

export const MetricsIconRow: React.FC<MetricsIconRowProps> = ({ isVisible }) => {
  const metrics = [
    { 
      label: 'BPM',
      icon: (
        <svg viewBox="0 0 24 24" fill="currentColor">
          <path d="M12 21.35l-1.45-1.32C5.4 15.36 2 12.28 2 8.5 2 5.42 4.42 3 7.5 3c1.74 0 3.41.81 4.5 2.09C13.09 3.81 14.76 3 16.5 3 19.58 3 22 5.42 22 8.5c0 3.78-3.4 6.86-8.55 11.54L12 21.35z" />
        </svg>
      )
    },
    { 
      label: 'RR',
      icon: (
        <svg viewBox="0 0 24 24" fill="currentColor">
          <path d="M16 12c0-1.11-.89-2-2-2-1 0-1.82.73-1.98 1.69l-1.18-3.54c-.15-.46-.55-.79-1.03-.84-.5-.05-.98.16-1.23.57L7.8 9.5H6c-.55 0-1 .45-1 1s.45 1 1 1h2.5c.37 0 .72-.2.88-.53l.42-.7.82 2.46c.15.45.54.77 1.01.82.48.04.94-.15 1.2-.54L14.18 11H16v1zm6-3V7c0-1.1-.9-2-2-2h-2c0-1.38-1.12-2.5-2.5-2.5S13 3.62 13 5h-2c0-1.38-1.12-2.5-2.5-2.5S6 3.62 6 5H4c-1.11 0-1.99.89-1.99 2v2c0 1.1.89 2 1.99 2v8c0 1.1.89 2 1.99 2h12.01c1.1 0 2-.9 2-2v-8c1.1 0 2-.9 2-2z" />
        </svg>
      )
    },
    { 
      label: 'SPO2',
      icon: (
        <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2">
          <circle cx="12" cy="12" r="10" />
          <path d="M12 6a6 6 0 0 0 0 12" />
        </svg>
      )
    },
    { 
      label: 'HRV',
      icon: (
        <svg viewBox="0 0 24 24" fill="currentColor">
          <path d="M22 12h-4l-3 9L9 3l-3 9H2" stroke="currentColor" strokeWidth="2" fill="none" strokeLinecap="round" strokeLinejoin="round" />
        </svg>
      )
    },
    { 
      label: 'SI',
      icon: (
        <svg viewBox="0 0 24 24" fill="currentColor">
          <path d="M11.99 2C6.47 2 2 6.48 2 12s4.47 10 9.99 10C17.52 22 22 17.52 22 12S17.52 2 11.99 2zM12 20c-4.42 0-8-3.58-8-8s3.58-8 8-8 8 3.58 8 8-3.58 8-8 8zm3.5-9c.83 0 1.5-.67 1.5-1.5S16.33 8 15.5 8 14 8.67 14 9.5s.67 1.5 1.5 1.5zm-7 0c.83 0 1.5-.67 1.5-1.5S9.33 8 8.5 8 7 8.67 7 9.5 7.67 11 8.5 11zm3.5 6.5c2.33 0 4.31-1.46 5.11-3.5H6.89c.8 2.04 2.78 3.5 5.11 3.5z" />
        </svg>
      )
    },
    { 
      label: 'BP',
      icon: (
        <svg viewBox="0 0 24 24" fill="currentColor">
          <path d="M12 2C6.48 2 2 6.48 2 12s4.48 10 10 10 10-4.48 10-10S17.52 2 12 2zm-2 15l-5-5 1.41-1.41L10 14.17l7.59-7.59L19 8l-9 9z" />
        </svg>
      )
    }
  ];
  
  return (
    <Container isVisible={isVisible}>
      {metrics.map((metric) => (
        <MetricIcon key={metric.label}>
          <IconCircle>
            {metric.icon}
          </IconCircle>
          <MetricLabel>{metric.label}</MetricLabel>
        </MetricIcon>
      ))}
    </Container>
  );
};