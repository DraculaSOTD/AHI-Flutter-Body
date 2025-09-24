import React, { useState } from 'react';
import styled from 'styled-components';
import { useNavigate } from 'react-router-dom';
import { theme } from '../../../styles/theme';
import { Layout, PageHeader, PageContent } from '../../../components/layout/Layout';
import { Card } from '../../../components/common/Card';
import { PrimaryGradientButton, OutlinedButton } from '../../../components/common/Button';
import { SaveReportModal } from '../../../components/SaveReportModal';
import { BodyScanResult } from '../../../services/bodyScan';

const ResultsContainer = styled.div`
  max-width: 1200px;
  margin: 0 auto;
`;

const ImagesSection = styled.div`
  display: grid;
  grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
  gap: ${theme.dimensions.paddingLarge};
  margin-bottom: ${theme.dimensions.paddingXLarge};
`;

const ImageCard = styled(Card)`
  overflow: hidden;
`;

const ImageWrapper = styled.div`
  position: relative;
  background: ${theme.colors.backgroundLight};
  padding: ${theme.dimensions.paddingLarge};
  text-align: center;
`;

const ImageTitle = styled.h3`
  font-size: ${theme.typography.headingSmall.fontSize};
  font-weight: ${theme.typography.headingSmall.fontWeight};
  color: ${theme.colors.textPrimary};
  margin-bottom: ${theme.dimensions.paddingMedium};
`;

const ScanImage = styled.img`
  width: 100%;
  max-width: 300px;
  height: 400px;
  object-fit: contain;
  margin: 0 auto;
  border-radius: ${theme.dimensions.radiusMedium};
  background: ${theme.colors.backgroundWhite};
`;

const MeasurementsSection = styled.div`
  display: grid;
  grid-template-columns: repeat(auto-fit, minmax(350px, 1fr));
  gap: ${theme.dimensions.paddingLarge};
  margin-bottom: ${theme.dimensions.paddingXLarge};
`;

const MeasurementCard = styled(Card)`
  padding: ${theme.dimensions.paddingLarge};
`;

const MeasurementTitle = styled.h3`
  font-size: ${theme.typography.headingSmall.fontSize};
  font-weight: ${theme.typography.headingSmall.fontWeight};
  color: ${theme.colors.textPrimary};
  margin-bottom: ${theme.dimensions.paddingLarge};
  display: flex;
  align-items: center;
  gap: ${theme.dimensions.paddingMedium};
  
  svg {
    width: 24px;
    height: 24px;
    color: ${theme.colors.primaryPurple};
  }
`;

const MeasurementGrid = styled.div`
  display: grid;
  gap: ${theme.dimensions.paddingMedium};
`;

const MeasurementRow = styled.div`
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: ${theme.dimensions.paddingSmall} 0;
  border-bottom: 1px solid ${theme.colors.divider};
  
  &:last-child {
    border-bottom: none;
  }
`;

const MeasurementLabel = styled.span`
  font-size: ${theme.typography.bodyMedium.fontSize};
  color: ${theme.colors.textSecondary};
`;

const MeasurementValue = styled.span`
  font-size: ${theme.typography.bodyLarge.fontSize};
  font-weight: 600;
  color: ${theme.colors.textPrimary};
  
  .unit {
    font-size: ${theme.typography.bodyMedium.fontSize};
    font-weight: normal;
    color: ${theme.colors.textSecondary};
    margin-left: 4px;
  }
`;

const ConfidenceSection = styled(Card)`
  padding: ${theme.dimensions.paddingLarge};
  margin-bottom: ${theme.dimensions.paddingXLarge};
  background: ${theme.gradients.cardBackground};
`;

const ConfidenceContent = styled.div`
  display: flex;
  align-items: center;
  justify-content: space-between;
  flex-wrap: wrap;
  gap: ${theme.dimensions.paddingLarge};
`;

const ConfidenceLabel = styled.div`
  font-size: ${theme.typography.bodyLarge.fontSize};
  color: ${theme.colors.textPrimary};
  font-weight: 500;
`;

const ConfidenceBar = styled.div`
  flex: 1;
  min-width: 200px;
  max-width: 400px;
`;

const ConfidenceTrack = styled.div`
  height: 8px;
  background: ${theme.colors.backgroundLight};
  border-radius: 4px;
  overflow: hidden;
  position: relative;
`;

const ConfidenceFill = styled.div<{ $percentage: number }>`
  height: 100%;
  width: ${props => props.$percentage}%;
  background: ${props => 
    props.$percentage >= 80 ? theme.colors.success :
    props.$percentage >= 60 ? theme.colors.warning :
    theme.colors.error
  };
  transition: width ${theme.transitions.normal};
`;

const ConfidencePercentage = styled.div`
  font-size: ${theme.typography.headingMedium.fontSize};
  font-weight: ${theme.typography.headingMedium.fontWeight};
  color: ${theme.colors.primaryPurple};
`;

const ActionsSection = styled.div`
  display: flex;
  gap: ${theme.dimensions.paddingLarge};
  justify-content: center;
  
  @media (max-width: ${theme.breakpoints.mobile}) {
    flex-direction: column;
  }
`;

const ActionButton = styled(PrimaryGradientButton)`
  min-width: 200px;
`;

const SecondaryButton = styled(OutlinedButton)`
  min-width: 200px;
`;

interface BodyScanResultsProps {
  result: BodyScanResult;
  onSave: () => void;
  onNewScan: () => void;
}

export const BodyScanResults: React.FC<BodyScanResultsProps> = ({
  result,
  onSave,
  onNewScan
}) => {
  const navigate = useNavigate();
  const [showSaveModal, setShowSaveModal] = useState(false);
  const { measurements } = result;
  const confidencePercentage = Math.round(result.confidence * 100);

  const handleSaveReport = (name: string) => {
    // Save with custom name
    const selectedProfile = localStorage.getItem('selectedProfile');
    const profile = selectedProfile ? JSON.parse(selectedProfile) : null;
    
    // Update the last saved assessment with the custom name
    const assessments = JSON.parse(localStorage.getItem('completedAssessments') || '[]');
    if (assessments.length > 0) {
      assessments[assessments.length - 1].name = name;
      localStorage.setItem('completedAssessments', JSON.stringify(assessments));
    }
    
    setShowSaveModal(false);
    onSave();
  };

  const handleViewReports = () => {
    navigate('/reports');
  };

  const handleBack = () => {
    navigate('/home');
  };

  const measurementGroups = [
    {
      title: 'Body Measurements',
      icon: (
        <svg viewBox="0 0 24 24" fill="currentColor">
          <path d="M12 2C13.1 2 14 2.9 14 4S13.1 6 12 6 10 5.1 10 4 10.9 2 12 2M10 20H11V14L9.5 8.5C9.1 7 7.8 6 6.3 6.4C4.8 6.8 3.8 8.1 4.2 9.6L6 16.2V20H7V13L5.5 7.5C5.3 6.7 5.9 6 6.7 5.8C7.5 5.6 8.2 6.2 8.4 7L11 14V20H14V14L16.6 7C16.8 6.2 17.5 5.6 18.3 5.8C19.1 6 19.7 6.7 19.5 7.5L18 13V20H19V16.2L19.8 9.6C20.2 8.1 19.2 6.8 17.7 6.4C16.2 6 14.9 7 14.5 8.5L13 14V20H14Z"/>
        </svg>
      ),
      items: [
        { label: 'Chest', value: measurements.chest, unit: 'cm' },
        { label: 'Waist', value: measurements.waist, unit: 'cm' },
        { label: 'Hip', value: measurements.hip, unit: 'cm' },
        { label: 'Thigh', value: measurements.thigh, unit: 'cm' },
        { label: 'Inseam', value: measurements.inseam, unit: 'cm' }
      ]
    },
    {
      title: 'Body Composition',
      icon: (
        <svg viewBox="0 0 24 24" fill="currentColor">
          <path d="M12 2A10 10 0 0 0 2 12A10 10 0 0 0 12 22A10 10 0 0 0 22 12A10 10 0 0 0 12 2M12 4A8 8 0 0 1 20 12C20 14.4 19 16.5 17.3 18C15.9 16.7 14.3 16 12.5 16C10.7 16 9.1 16.7 7.7 18C6 16.5 5 14.4 5 12A8 8 0 0 1 12 4M14 5.89C13.62 5.9 13.26 6.15 13.1 6.54L11.81 9.77L11.71 10C11.32 10.26 11.16 10.73 11.29 11.18C11.56 11.72 12.07 12.09 12.65 12.18C13.06 12.24 13.47 12.09 13.76 11.79L14.19 11.29L15.27 11.47C15.41 11.5 15.55 11.5 15.69 11.46C16.17 11.38 16.53 10.97 16.53 10.5C16.5 10.12 16.29 9.77 15.96 9.57C15.71 9.4 15.41 9.35 15.12 9.39L13.96 9.58L13.4 8.1C13.28 7.77 13 7.53 12.65 7.47C12.5 7.43 12.33 7.44 12.17 7.5C12.03 7.55 11.89 7.64 11.78 7.75L10.83 8.88C10.59 9.19 10.53 9.6 10.66 9.96C10.87 10.45 11.4 10.73 11.91 10.61C12.13 10.56 12.32 10.43 12.46 10.26L13.05 9.5L14 5.89M8.5 12C9.06 12.09 9.63 12.17 10.22 12.21C10.28 12.45 10.37 12.67 10.5 12.87C10.66 13.27 10.91 13.64 11.24 13.94C9.5 14.08 7.78 14.45 6.16 15.03C5.31 13.86 4.79 12.46 4.67 10.94C5.95 11.5 7.41 11.86 8.96 11.94L8.5 12M15.79 12.2C16.5 12.17 17.11 12.09 17.66 12C17.25 12.07 16.82 12.11 16.38 12.13C16.18 12.74 15.76 13.26 15.22 13.6C15.54 13.44 15.82 13.23 16.05 12.97C16.23 12.78 16.39 12.57 16.5 12.33C16.45 12.28 16.39 12.24 16.33 12.2L15.79 12.2M19.33 10.94C19.21 12.46 18.69 13.86 17.84 15.03C16.04 14.37 14.03 14 12 14C11.17 14 10.34 14.04 9.5 14.13C9.17 13.85 8.88 13.5 8.68 13.1C9.68 13.03 10.67 13 11.68 13C13.62 13.06 15.54 13.4 17.36 14C18.29 12.88 18.87 11.5 19 10C18.14 10.47 17.18 10.82 16.15 11L16.12 11C16.14 10.83 16.14 10.67 16.12 10.5C17.66 10.3 19.07 9.76 20.3 8.93C20.13 9.74 19.8 10.5 19.33 10.94Z"/>
        </svg>
      ),
      items: [
        { label: 'Body Fat', value: measurements.bodyFat.toFixed(1), unit: '%' },
        { label: 'Muscle Mass', value: measurements.muscleMass.toFixed(1), unit: 'kg' },
        { label: 'Visceral Fat', value: measurements.visceralFat, unit: 'level' },
        { label: 'Weight', value: measurements.weight, unit: 'kg' },
        { label: 'Metabolic Age', value: measurements.metabolicAge || 'N/A', unit: measurements.metabolicAge ? 'years' : '' },
        { label: 'Body Type', value: measurements.bodyType || 'N/A', unit: '' }
      ]
    }
  ];

  return (
    <Layout>
      <PageHeader
        title="Body Scan Complete"
        subtitle="Your body composition analysis results"
        showBackButton
        onBack={handleBack}
      />
      
      <PageContent>
        <ResultsContainer>
          {/* Captured Images */}
          <ImagesSection>
            <ImageCard>
              <ImageWrapper>
                <ImageTitle>Front View</ImageTitle>
                <ScanImage src={result.frontImage} alt="Front view" />
              </ImageWrapper>
            </ImageCard>
            
            <ImageCard>
              <ImageWrapper>
                <ImageTitle>Side View</ImageTitle>
                <ScanImage src={result.sideImage} alt="Side view" />
              </ImageWrapper>
            </ImageCard>
          </ImagesSection>

          {/* Measurements */}
          <MeasurementsSection>
            {measurementGroups.map((group) => (
              <MeasurementCard key={group.title}>
                <MeasurementTitle>
                  {group.icon}
                  {group.title}
                </MeasurementTitle>
                <MeasurementGrid>
                  {group.items.map((item) => (
                    <MeasurementRow key={item.label}>
                      <MeasurementLabel>{item.label}</MeasurementLabel>
                      <MeasurementValue>
                        {item.value}
                        <span className="unit">{item.unit}</span>
                      </MeasurementValue>
                    </MeasurementRow>
                  ))}
                </MeasurementGrid>
              </MeasurementCard>
            ))}
          </MeasurementsSection>

          {/* Confidence Score */}
          <ConfidenceSection>
            <ConfidenceContent>
              <ConfidenceLabel>Measurement Confidence</ConfidenceLabel>
              <ConfidenceBar>
                <ConfidenceTrack>
                  <ConfidenceFill $percentage={confidencePercentage} />
                </ConfidenceTrack>
              </ConfidenceBar>
              <ConfidencePercentage>{confidencePercentage}%</ConfidencePercentage>
            </ConfidenceContent>
          </ConfidenceSection>

          {/* Actions */}
          <ActionsSection>
            <SecondaryButton onClick={onNewScan}>
              New Scan
            </SecondaryButton>
            <ActionButton onClick={() => setShowSaveModal(true)}>
              Save Report
            </ActionButton>
            <SecondaryButton onClick={handleViewReports}>
              View Reports
            </SecondaryButton>
          </ActionsSection>
        </ResultsContainer>
      </PageContent>
      
      <SaveReportModal
        open={showSaveModal}
        onClose={() => setShowSaveModal(false)}
        onSave={handleSaveReport}
        scanType="body"
      />
    </Layout>
  );
};