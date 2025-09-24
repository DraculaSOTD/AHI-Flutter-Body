import React, { useState } from 'react';
import styled from 'styled-components';
import { useNavigate, useLocation } from 'react-router-dom';
import { theme } from '../../styles/theme';
import { Layout, PageHeader, PageContent } from '../../components/layout/Layout';
import { Card } from '../../components/common/Card';
import { PrimaryGradientButton } from '../../components/common/Button';
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

export const FaceScanPreparationScreen: React.FC = () => {
  const navigate = useNavigate();
  const location = useLocation();
  const [userProfile, setUserProfile] = useState<any>(null);
  
  // Load user profile on mount
  React.useEffect(() => {
    // First try location state
    if (location.state?.userProfile) {
      setUserProfile(location.state.userProfile);
      return;
    }
    
    // Then try localStorage
    const selectedProfile = localStorage.getItem('selectedProfile');
    if (!selectedProfile) {
      alert('Please select a profile first from the Home screen');
      navigate('/home');
      return;
    }
    
    try {
      const profile = JSON.parse(selectedProfile);
      const profileData = {
        age: profile.age,
        gender: profile.gender,
        height: profile.height,
        weight: profile.weight
      };
      setUserProfile(profileData);
    } catch (error) {
      console.error('Error parsing selected profile:', error);
      alert('Error loading profile. Please select a profile again.');
      navigate('/home');
    }
  }, [location.state, navigate]);
  
  const handleStartScan = () => {
    if (!userProfile) {
      alert('Please select a profile before starting the scan');
      navigate('/home');
      return;
    }
    
    // Get selected profile from localStorage
    const selectedProfile = localStorage.getItem('selectedProfile');
    if (selectedProfile) {
      const profile = JSON.parse(selectedProfile);
      
      // Create in-progress scan entry
      const inProgressScan = {
        id: `face_${Date.now()}`,
        profileId: profile.id,
        scanType: 'face' as const,
        status: 'in-progress' as const,
        progress: 0,
        startedAt: new Date().toISOString()
      };
      
      saveScanResult(profile.id, inProgressScan);
    }
    
    // Navigate directly to camera screen - SDK will initialize there
    navigate('/scan/face/camera', { state: { userProfile } });
  };
  
  return (
    <Layout>
      <PageHeader
        title="Face Scan Preparation"
        subtitle="Please follow these instructions for accurate results"
        showBackButton
        onBack={() => navigate('/new-scan')}
      />
      
      <PageContent>
        <PreparationContainer>
          <InstructionCard>
            <InstructionTitle>Before You Begin</InstructionTitle>
            <InstructionList>
              <li>Find a well-lit area with even lighting on your face</li>
              <li>Remove glasses, hats, or anything covering your face</li>
              <li>Position yourself arm's length from the camera</li>
              <li>Ensure your face is centered in the frame</li>
              <li>Remain still during the 60-second scan</li>
            </InstructionList>
          </InstructionCard>
          
          <IconGrid>
            <IconItem>
              <IconCircle color={theme.colors.primaryBlue}>
                <svg viewBox="0 0 24 24" fill="currentColor">
                  <path d="M12 2C6.48 2 2 6.48 2 12s4.48 10 10 10 10-4.48 10-10S17.52 2 12 2zm0 3c1.66 0 3 1.34 3 3s-1.34 3-3 3-3-1.34-3-3 1.34-3 3-3zm0 14c-2.5 0-4.71-1.28-6-3.21.03-2 4-3.1 6-3.1s5.97 1.1 6 3.1C16.71 17.72 14.5 19 12 19z"/>
                </svg>
              </IconCircle>
              <IconLabel>Face Centered</IconLabel>
              <IconDescription>Keep your face in the center of the screen</IconDescription>
            </IconItem>
            
            <IconItem>
              <IconCircle color={theme.colors.warning}>
                <svg viewBox="0 0 24 24" fill="currentColor">
                  <path d="M12 7c-2.76 0-5 2.24-5 5s2.24 5 5 5 5-2.24 5-5-2.24-5-5-5zM2 13h2c.55 0 1-.45 1-1s-.45-1-1-1H2c-.55 0-1 .45-1 1s.45 1 1 1zm18 0h2c.55 0 1-.45 1-1s-.45-1-1-1h-2c-.55 0-1 .45-1 1s.45 1 1 1zM11 2v2c0 .55.45 1 1 1s1-.45 1-1V2c0-.55-.45-1-1-1s-1 .45-1 1zm0 18v2c0 .55.45 1 1 1s1-.45 1-1v-2c0-.55-.45-1-1-1s-1 .45-1 1zM5.99 4.58c-.39-.39-1.03-.39-1.41 0-.39.39-.39 1.03 0 1.41l1.06 1.06c.39.39 1.03.39 1.41 0s.39-1.03 0-1.41L5.99 4.58zm12.37 12.37c-.39-.39-1.03-.39-1.41 0-.39.39-.39 1.03 0 1.41l1.06 1.06c.39.39 1.03.39 1.41 0 .39-.39.39-1.03 0-1.41l-1.06-1.06zm1.06-10.96c.39-.39.39-1.03 0-1.41-.39-.39-1.03-.39-1.41 0l-1.06 1.06c-.39.39-.39 1.03 0 1.41s1.03.39 1.41 0l1.06-1.06zM7.05 18.36c.39-.39.39-1.03 0-1.41-.39-.39-1.03-.39-1.41 0l-1.06 1.06c-.39.39-.39 1.03 0 1.41s1.03.39 1.41 0l1.06-1.06z"/>
                </svg>
              </IconCircle>
              <IconLabel>Good Lighting</IconLabel>
              <IconDescription>Ensure even lighting on your face</IconDescription>
            </IconItem>
            
            <IconItem>
              <IconCircle color={theme.colors.success}>
                <svg viewBox="0 0 24 24" fill="currentColor">
                  <path d="M19 3H5c-1.1 0-2 .9-2 2v14c0 1.1.9 2 2 2h14c1.1 0 2-.9 2-2V5c0-1.1-.9-2-2-2zm-7 3c1.93 0 3.5 1.57 3.5 3.5S13.93 13 12 13s-3.5-1.57-3.5-3.5S10.07 6 12 6zm7 13H5v-.23c0-.62.28-1.2.76-1.58C7.47 15.82 9.64 15 12 15s4.53.82 6.24 2.19c.48.38.76.97.76 1.58V19z"/>
                </svg>
              </IconCircle>
              <IconLabel>Clear View</IconLabel>
              <IconDescription>Remove glasses and face coverings</IconDescription>
            </IconItem>
            
            <IconItem>
              <IconCircle color={theme.colors.primaryPurple}>
                <svg viewBox="0 0 24 24" fill="currentColor">
                  <path d="M12 2C6.48 2 2 6.48 2 12s4.48 10 10 10 10-4.48 10-10S17.52 2 12 2zm0 18c-4.41 0-8-3.59-8-8s3.59-8 8-8 8 3.59 8 8-3.59 8-8 8zm.31-8.86c-1.77-.45-2.34-.94-2.34-1.67 0-.84.79-1.43 2.1-1.43 1.38 0 1.9.66 1.94 1.64h1.71c-.05-1.34-.87-2.57-2.49-2.97V5H10.9v1.69c-1.51.32-2.72 1.3-2.72 2.81 0 1.79 1.49 2.69 3.66 3.21 1.95.46 2.34 1.15 2.34 1.87 0 .53-.39 1.39-2.1 1.39-1.6 0-2.23-.72-2.32-1.64H8.04c.1 1.7 1.36 2.66 2.86 2.97V19h2.34v-1.67c1.52-.29 2.72-1.16 2.73-2.77-.01-2.2-1.9-2.96-3.66-3.42z"/>
                </svg>
              </IconCircle>
              <IconLabel>Stay Still</IconLabel>
              <IconDescription>Remain motionless during the scan</IconDescription>
            </IconItem>
          </IconGrid>
          
          <WarningCard>
            <WarningContent>
              <svg viewBox="0 0 24 24" fill="currentColor">
                <path d="M1 21h22L12 2 1 21zm12-3h-2v-2h2v2zm0-4h-2v-4h2v4z"/>
              </svg>
              <p>
                This scan uses advanced computer vision to measure vital signs through subtle color changes in your face. 
                Results are for informational purposes only and should not replace medical advice.
              </p>
            </WarningContent>
          </WarningCard>
          
          <ButtonContainer>
            <PrimaryGradientButton 
              size="large" 
              onClick={handleStartScan}
            >
              Start Face Scan
            </PrimaryGradientButton>
          </ButtonContainer>
        </PreparationContainer>
      </PageContent>
    </Layout>
  );
};