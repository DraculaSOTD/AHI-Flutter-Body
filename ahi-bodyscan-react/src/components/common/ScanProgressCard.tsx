import React from 'react';
import styled from 'styled-components';
import { theme } from '../../styles/theme';
import { Card } from './Card';
import { ProgressBar } from './ProgressBar';
import { OutlinedButton } from './Button';
import { ScanProgress, ScanResult } from '../../utils/scanDataHelpers';

interface ScanProgressCardProps {
  scanProgress: ScanProgress;
  onResumeScan?: (scanType: 'body' | 'face') => void;
  onStartScan?: (scanType: 'body' | 'face') => void;
}

const ProgressHeader = styled.div`
  margin-bottom: ${theme.dimensions.paddingLarge};
`;

const ProfileName = styled.h3`
  font-size: ${theme.typography.headingSmall.fontSize};
  font-weight: ${theme.typography.headingSmall.fontWeight};
  color: ${theme.colors.textPrimary};
  margin-bottom: ${theme.dimensions.paddingSmall};
`;

const ProgressStats = styled.div`
  display: flex;
  justify-content: space-between;
  align-items: center;
  margin-bottom: ${theme.dimensions.paddingMedium};
  
  .progress-text {
    font-size: ${theme.typography.bodyMedium.fontSize};
    color: ${theme.colors.textSecondary};
  }
  
  .progress-percentage {
    font-size: ${theme.typography.headingSmall.fontSize};
    font-weight: ${theme.typography.headingSmall.fontWeight};
    color: ${theme.colors.primaryPurple};
  }
`;

const ScansList = styled.div`
  margin-top: ${theme.dimensions.paddingLarge};
`;

const ScanItem = styled.div`
  display: flex;
  align-items: center;
  justify-content: space-between;
  padding: ${theme.dimensions.paddingMedium};
  background: ${theme.colors.backgroundLight};
  border-radius: ${theme.dimensions.radiusMedium};
  margin-bottom: ${theme.dimensions.paddingSmall};
  
  &:last-child {
    margin-bottom: 0;
  }
`;

const ScanInfo = styled.div`
  display: flex;
  align-items: center;
  gap: ${theme.dimensions.paddingMedium};
  
  .scan-icon {
    width: 40px;
    height: 40px;
    display: flex;
    align-items: center;
    justify-content: center;
    background: ${theme.colors.backgroundWhite};
    border-radius: ${theme.dimensions.radiusMedium};
    font-size: 20px;
  }
  
  .scan-details {
    h4 {
      font-size: ${theme.typography.bodyLarge.fontSize};
      font-weight: 600;
      color: ${theme.colors.textPrimary};
      margin-bottom: 2px;
    }
    
    p {
      font-size: ${theme.typography.bodySmall.fontSize};
      color: ${theme.colors.textSecondary};
    }
  }
`;

const ScanStatus = styled.div<{ $completed: boolean }>`
  display: flex;
  align-items: center;
  gap: ${theme.dimensions.paddingSmall};
  
  .status-icon {
    width: 24px;
    height: 24px;
    border-radius: 50%;
    display: flex;
    align-items: center;
    justify-content: center;
    background: ${props => props.$completed ? theme.colors.success : theme.colors.backgroundLight};
    color: ${props => props.$completed ? theme.colors.textWhite : theme.colors.textTertiary};
    
    svg {
      width: 16px;
      height: 16px;
    }
  }
  
  .status-text {
    font-size: ${theme.typography.bodySmall.fontSize};
    color: ${props => props.$completed ? theme.colors.success : theme.colors.textTertiary};
    font-weight: ${props => props.$completed ? '600' : '400'};
  }
`;

const ActionButton = styled(OutlinedButton)`
  padding: ${theme.dimensions.paddingSmall} ${theme.dimensions.paddingMedium};
  font-size: ${theme.typography.bodySmall.fontSize};
`;

const CompletionMessage = styled.div`
  text-align: center;
  padding: ${theme.dimensions.paddingLarge};
  background: ${theme.colors.success}10;
  border-radius: ${theme.dimensions.radiusMedium};
  margin-top: ${theme.dimensions.paddingLarge};
  
  .icon {
    font-size: 48px;
    margin-bottom: ${theme.dimensions.paddingMedium};
  }
  
  p {
    font-size: ${theme.typography.bodyMedium.fontSize};
    color: ${theme.colors.success};
    font-weight: 600;
  }
`;

export const ScanProgressCard: React.FC<ScanProgressCardProps> = ({
  scanProgress,
  onResumeScan,
  onStartScan
}) => {
  const getScanIcon = (scanType: 'body' | 'face') => {
    if (scanType === 'body') {
      return (
        <svg viewBox="0 0 24 24" fill="currentColor" width="24" height="24">
          <path d="M12 1C10.89 1 10 1.9 10 3C10 4.11 10.89 5 12 5C13.11 5 14 4.11 14 3C14 1.9 13.11 1 12 1M10 6C9.73 6 9.5 6.11 9.31 6.28H9.3L4 11.59L5.42 13L9 9.41V22H11V15H13V22H15V9.41L18.58 13L20 11.59L14.7 6.28C14.5 6.11 14.27 6 14 6" />
        </svg>
      );
    } else {
      return (
        <svg viewBox="0 0 24 24" fill="currentColor" width="24" height="24">
          <path d="M9,11.75A1.25,1.25 0 0,0 7.75,13A1.25,1.25 0 0,0 9,14.25A1.25,1.25 0 0,0 10.25,13A1.25,1.25 0 0,0 9,11.75M15,11.75A1.25,1.25 0 0,0 13.75,13A1.25,1.25 0 0,0 15,14.25A1.25,1.25 0 0,0 16.25,13A1.25,1.25 0 0,0 15,11.75M12,2A10,10 0 0,0 2,12A10,10 0 0,0 12,22A10,10 0 0,0 22,12A10,10 0 0,0 12,2M12,20C7.59,20 4,16.41 4,12C4,11.71 4,11.42 4.05,11.14C6.41,10.09 8.28,8.16 9.26,5.77C11.07,8.33 14.05,10 17.42,10C18.2,10 18.95,9.91 19.67,9.74C19.88,10.45 20,11.21 20,12C20,16.41 16.41,20 12,20Z" />
        </svg>
      );
    }
  };

  const getScanName = (scanType: 'body' | 'face') => {
    return scanType === 'body' ? 'Body Scan' : 'Face Scan';
  };

  const getScanDescription = (scanType: 'body' | 'face') => {
    return scanType === 'body' 
      ? 'Body composition & measurements' 
      : 'Vital signs & cardiovascular health';
  };

  return (
    <Card>
      <ProgressHeader>
        <ProfileName>{scanProgress.profileName}</ProfileName>
        <ProgressStats>
          <span className="progress-text">
            {scanProgress.completedScans} of {scanProgress.totalScansRequired} scans completed
          </span>
          <span className="progress-percentage">{scanProgress.progressPercentage}%</span>
        </ProgressStats>
        <ProgressBar 
          progress={scanProgress.progressPercentage}
          height="12px"
          animate={!scanProgress.hasCompletedAllScans}
        />
      </ProgressHeader>

      {scanProgress.hasCompletedAllScans ? (
        <CompletionMessage>
          <div className="icon">✅</div>
          <p>All scans completed! Health assessment ready.</p>
        </CompletionMessage>
      ) : (
        <ScansList>
          {/* Show completed scans */}
          {scanProgress.completedScanTypes.map(scanType => (
            <ScanItem key={scanType}>
              <ScanInfo>
                <div className="scan-icon">{getScanIcon(scanType)}</div>
                <div className="scan-details">
                  <h4>{getScanName(scanType)}</h4>
                  <p>{getScanDescription(scanType)}</p>
                </div>
              </ScanInfo>
              <ScanStatus $completed={true}>
                <div className="status-icon">
                  <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="3">
                    <polyline points="20 6 9 17 4 12" />
                  </svg>
                </div>
                <span className="status-text">Completed</span>
              </ScanStatus>
            </ScanItem>
          ))}

          {/* Show pending scans */}
          {scanProgress.pendingScanTypes.map(scanType => (
            <ScanItem key={scanType}>
              <ScanInfo>
                <div className="scan-icon">{getScanIcon(scanType)}</div>
                <div className="scan-details">
                  <h4>{getScanName(scanType)}</h4>
                  <p>{getScanDescription(scanType)}</p>
                </div>
              </ScanInfo>
              <ScanStatus $completed={false}>
                <ActionButton
                  onClick={() => onStartScan?.(scanType)}
                >
                  Start Scan
                </ActionButton>
              </ScanStatus>
            </ScanItem>
          ))}
        </ScansList>
      )}
    </Card>
  );
};