import React, { useState } from 'react';
import styled from 'styled-components';
import { useNavigate, useLocation } from 'react-router-dom';
import { theme } from '../../styles/theme';
import { Layout, PageHeader, PageContent } from '../../components/layout/Layout';
import { Card } from '../../components/common/Card';
import { PrimaryGradientButton } from '../../components/common/Button';
import { LoadingModal } from '../../components/common/LoadingModal';
import { saveScanResult } from '../../utils/scanDataHelpers';

const PreparationContainer = styled.div`
  max-width: 800px;
  margin: 0 auto;
`;

const InstructionCard = styled(Card)`
  margin-bottom: ${theme.dimensions.paddingLarge};
`;

const InstructionTitle = styled.h3`
  font-size: ${theme.typography.headingSmall.fontSize};
  font-weight: ${theme.typography.headingSmall.fontWeight};
  color: ${theme.colors.textPrimary};
  margin-bottom: ${theme.dimensions.paddingMedium};
`;

const InstructionList = styled.ol`
  padding-left: ${theme.dimensions.paddingLarge};
  margin: 0;
  
  li {
    margin-bottom: ${theme.dimensions.paddingMedium};
    font-size: ${theme.typography.bodyMedium.fontSize};
    line-height: ${theme.typography.bodyMedium.lineHeight};
    color: ${theme.colors.textSecondary};
  }
`;

const IconGrid = styled.div`
  display: grid;
  grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
  gap: ${theme.dimensions.paddingLarge};
  margin-bottom: ${theme.dimensions.paddingXLarge};
`;

const IconItem = styled.div`
  display: flex;
  flex-direction: column;
  align-items: center;
  text-align: center;
  gap: ${theme.dimensions.paddingMedium};
`;

const IconCircle = styled.div<{ color: string }>`
  width: 80px;
  height: 80px;
  border-radius: 50%;
  background: ${props => props.color};
  display: flex;
  align-items: center;
  justify-content: center;
  
  svg {
    width: 40px;
    height: 40px;
    color: ${theme.colors.textWhite};
  }
`;

const IconLabel = styled.p`
  font-size: ${theme.typography.bodyMedium.fontSize};
  color: ${theme.colors.textPrimary};
  font-weight: 500;
  margin: 0;
`;

const IconDescription = styled.p`
  font-size: ${theme.typography.bodySmall.fontSize};
  color: ${theme.colors.textSecondary};
  margin: 0;
`;

const WarningCard = styled(Card)`
  background: rgba(255, 149, 0, 0.1);
  border: 1px solid ${theme.colors.warning};
  margin-bottom: ${theme.dimensions.paddingLarge};
`;

const WarningContent = styled.div`
  display: flex;
  align-items: flex-start;
  gap: ${theme.dimensions.paddingMedium};
  
  svg {
    width: 24px;
    height: 24px;
    color: ${theme.colors.warning};
    flex-shrink: 0;
  }
  
  p {
    margin: 0;
    font-size: ${theme.typography.bodyMedium.fontSize};
    color: ${theme.colors.textPrimary};
  }
`;

const ButtonContainer = styled.div`
  display: flex;
  justify-content: center;
  margin-top: ${theme.dimensions.paddingXLarge};
`;

export const BodyScanPreparationScreen: React.FC = () => {
  const navigate = useNavigate();
  const location = useLocation();
  const userProfile = location.state?.userProfile;
  const userHeight = location.state?.userHeight;
  const userWeight = location.state?.userWeight;
  const [isLoading, setIsLoading] = useState(false);
  
  const handleStartScan = () => {
    setIsLoading(true);
    
    // Get selected profile from localStorage
    const selectedProfile = localStorage.getItem('selectedProfile');
    if (selectedProfile) {
      const profile = JSON.parse(selectedProfile);
      
      // Create in-progress scan entry
      const inProgressScan = {
        id: `body_${Date.now()}`,
        profileId: profile.id,
        scanType: 'body' as const,
        status: 'in-progress' as const,
        progress: 0,
        startedAt: new Date().toISOString()
      };
      
      saveScanResult(profile.id, inProgressScan);
    }
    
    navigate('/scan/body/camera', { 
      state: { userProfile, userHeight, userWeight } 
    });
  };
  
  const handleBack = () => {
    navigate(-1);
  };
  
  return (
    <Layout>
      <PageHeader
        title="Body Scan Preparation"
        subtitle="Please follow these instructions for accurate results"
        showBackButton
        onBack={handleBack}
      />
      
      <PageContent>
        <PreparationContainer>
          <InstructionCard>
            <InstructionTitle>Before You Begin</InstructionTitle>
            <InstructionList>
              <li>Wear form-fitting clothing for best results</li>
              <li>Find a well-lit area with even lighting</li>
              <li>Ensure you have enough space to stand back from the camera</li>
              <li>Remove bulky items from pockets</li>
              <li>You'll need to capture two poses: front and side view</li>
            </InstructionList>
          </InstructionCard>
          
          <IconGrid>
            <IconItem>
              <IconCircle color={theme.colors.primaryBlue}>
                <svg viewBox="0 0 24 24" fill="currentColor">
                  <path d="M12 2C6.48 2 2 6.48 2 12s4.48 10 10 10 10-4.48 10-10S17.52 2 12 2zm0 3c1.66 0 3 1.34 3 3s-1.34 3-3 3-3-1.34-3-3 1.34-3 3-3zm0 14c-2.5 0-4.71-1.28-6-3.21.03-2 4-3.1 6-3.1s5.97 1.1 6 3.1C16.71 17.72 14.5 19 12 19z"/>
                </svg>
              </IconCircle>
              <IconLabel>Front Pose</IconLabel>
              <IconDescription>Face camera directly, arms slightly away</IconDescription>
            </IconItem>
            
            <IconItem>
              <IconCircle color={theme.colors.warning}>
                <svg viewBox="0 0 24 24" fill="currentColor">
                  <path d="M15.5 5.5c1.1 0 2-.9 2-2s-.9-2-2-2-2 .9-2 2 .9 2 2 2zM5 12c-2.8 0-5 2.2-5 5s2.2 5 5 5 5-2.2 5-5-2.2-5-5-5zm0 8.5c-1.9 0-3.5-1.6-3.5-3.5s1.6-3.5 3.5-3.5 3.5 1.6 3.5 3.5-1.6 3.5-3.5 3.5zm5.8-10l2.4-2.4.8.8c1.3 1.3 3 2.1 5.1 2.1V9c-1.5 0-2.7-.6-3.6-1.5l-1.9-1.9c-.5-.4-1-.6-1.6-.6s-1.1.2-1.4.6L7.9 8.3c-.4.4-.6.9-.6 1.4 0 .6.2 1.1.6 1.4L11 14v5h2v-6.2l-2.2-2.3z"/>
                </svg>
              </IconCircle>
              <IconLabel>Full Body View</IconLabel>
              <IconDescription>Head to ankles visible in frame</IconDescription>
            </IconItem>
            
            <IconItem>
              <IconCircle color={theme.colors.success}>
                <svg viewBox="0 0 24 24" fill="currentColor">
                  <path d="M9.5,3A6.5,6.5 0 0,1 16,9.5C16,11.11 15.41,12.59 14.44,13.73L14.71,14H15.5L20.5,19L19,20.5L14,15.5V14.71L13.73,14.44C12.59,15.41 11.11,16 9.5,16A6.5,6.5 0 0,1 3,9.5A6.5,6.5 0 0,1 9.5,3M9.5,5C7,5 5,7 5,9.5C5,12 7,14 9.5,14C12,14 14,12 14,9.5C14,7 12,5 9.5,5Z"/>
                </svg>
              </IconCircle>
              <IconLabel>Hold Still</IconLabel>
              <IconDescription>Auto-capture after 3 seconds</IconDescription>
            </IconItem>
            
            <IconItem>
              <IconCircle color={theme.colors.primaryPurple}>
                <svg viewBox="0 0 24 24" fill="currentColor">
                  <path d="M17,1H7A2,2 0 0,0 5,3V21A2,2 0 0,0 7,23H17A2,2 0 0,0 19,21V3A2,2 0 0,0 17,1M17,19H7V5H17V19M12,6A4,4 0 0,0 8,10A4,4 0 0,0 12,14A4,4 0 0,0 16,10A4,4 0 0,0 12,6M12,12A2,2 0 0,1 10,10A2,2 0 0,1 12,8A2,2 0 0,1 14,10A2,2 0 0,1 12,12Z"/>
                </svg>
              </IconCircle>
              <IconLabel>Side Pose</IconLabel>
              <IconDescription>Stand sideways, look straight ahead</IconDescription>
            </IconItem>
          </IconGrid>
          
          <WarningCard>
            <WarningContent>
              <svg viewBox="0 0 24 24" fill="currentColor">
                <path d="M1 21h22L12 2 1 21zm12-3h-2v-2h2v2zm0-4h-2v-4h2v4z"/>
              </svg>
              <p>
                This scan uses advanced computer vision to calculate body measurements. 
                For best results, wear fitted clothing and ensure consistent lighting. 
                Results are for informational purposes only.
              </p>
            </WarningContent>
          </WarningCard>
          
          <ButtonContainer>
            <PrimaryGradientButton size="large" onClick={handleStartScan}>
              Start Body Scan
            </PrimaryGradientButton>
          </ButtonContainer>
        </PreparationContainer>
      </PageContent>
      
      <LoadingModal
        isOpen={isLoading}
        text="Initializing camera"
      />
    </Layout>
  );
};