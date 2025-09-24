import React, { useState } from 'react';
import styled from 'styled-components';
import { useNavigate } from 'react-router-dom';
import { theme } from '../../styles/theme';
import { Layout, PageHeader, PageContent } from '../../components/layout/Layout';
import { Card } from '../../components/common/Card';
import { HealthHistoryConfirmationModal } from '../../components/profile/HealthHistoryConfirmationModal';
import { ProfileCreationModal } from '../../components/profile/ProfileCreationModal';
import { ProfileRequiredModal } from '../../components/profile/ProfileRequiredModal';

const ScanOptionsGrid = styled.div`
  display: grid;
  gap: ${theme.dimensions.paddingLarge};
  max-width: 800px;
  margin: 0 auto;
  
  @media (min-width: ${theme.breakpoints.tablet}) {
    grid-template-columns: 1fr 1fr;
  }
`;

const ScanCard = styled(Card)`
  cursor: pointer;
  transition: all ${theme.transitions.normal};
  
  &:hover {
    transform: translateY(-4px);
    box-shadow: ${theme.shadows.large};
  }
`;

const ScanIcon = styled.div`
  width: 80px;
  height: 80px;
  margin: 0 auto ${theme.dimensions.paddingLarge};
  display: flex;
  align-items: center;
  justify-content: center;
  font-size: 48px;
  background: ${theme.gradients.primary};
  border-radius: ${theme.dimensions.radiusLarge};
  
  svg {
    width: 48px;
    height: 48px;
    color: ${theme.colors.textWhite};
  }
`;

const ScanTitle = styled.h3`
  font-size: ${theme.typography.headingMedium.fontSize};
  font-weight: ${theme.typography.headingMedium.fontWeight};
  color: ${theme.colors.textPrimary};
  text-align: center;
  margin-bottom: ${theme.dimensions.paddingMedium};
`;

const ScanDescription = styled.p`
  font-size: ${theme.typography.bodyMedium.fontSize};
  line-height: ${theme.typography.bodyMedium.lineHeight};
  color: ${theme.colors.textSecondary};
  text-align: center;
  margin-bottom: ${theme.dimensions.paddingLarge};
`;

const FeatureList = styled.ul`
  list-style: none;
  padding: 0;
  margin: 0;
`;

const FeatureItem = styled.li`
  display: flex;
  align-items: center;
  gap: ${theme.dimensions.paddingSmall};
  margin-bottom: ${theme.dimensions.paddingSmall};
  font-size: ${theme.typography.bodyMedium.fontSize};
  color: ${theme.colors.textSecondary};
  
  &::before {
    content: '✓';
    color: ${theme.colors.success};
    font-weight: bold;
  }
`;

const ScanDuration = styled.div`
  text-align: center;
  margin-top: ${theme.dimensions.paddingLarge};
  padding-top: ${theme.dimensions.paddingLarge};
  border-top: 1px solid ${theme.colors.divider};
  
  .duration {
    font-size: ${theme.typography.headingSmall.fontSize};
    font-weight: ${theme.typography.headingSmall.fontWeight};
    color: ${theme.colors.primaryBlue};
  }
  
  .label {
    font-size: ${theme.typography.bodySmall.fontSize};
    color: ${theme.colors.textTertiary};
  }
`;

export const NewScanScreen: React.FC = () => {
  const navigate = useNavigate();
  const [showHealthHistoryModal, setShowHealthHistoryModal] = useState(false);
  const [showProfileModal, setShowProfileModal] = useState(false);
  const [showProfileRequiredModal, setShowProfileRequiredModal] = useState(false);
  const [pendingScanType, setPendingScanType] = useState<'body' | 'face' | null>(null);
  const [profileData, setProfileData] = useState<any>(null);
  
  const proceedToScan = (scanType: 'body' | 'face') => {
    const selectedProfile = localStorage.getItem('selectedProfile');
    if (!selectedProfile) {
      alert('Please select a profile before starting a scan');
      setShowProfileRequiredModal(true);
      setPendingScanType(scanType);
      return;
    }
    
    const profile = JSON.parse(selectedProfile);
    
    // Store profile data
    localStorage.setItem('currentUserProfile', JSON.stringify({
      userId: profile.id,
      name: profile.name,
      age: profile.age,
      gender: profile.gender,
      email: localStorage.getItem('userEmail') || 'user@example.com'
    }));
    localStorage.setItem('userHeight', String(profile.height));
    localStorage.setItem('userWeight', String(profile.weight));
    
    // Navigate with profile data
    const userProfile = {
      age: profile.age,
      gender: profile.gender,
      height: profile.height,
      weight: profile.weight
    };
    
    navigate(scanType === 'body' ? '/scan/body/preparation' : '/scan/face/preparation', {
      state: { userProfile }
    });
  };
  
  const checkHealthHistory = (scanType: 'body' | 'face') => {
    // Check if user has selected profile
    const selectedProfile = localStorage.getItem('selectedProfile');
    
    if (selectedProfile) {
      const profile = JSON.parse(selectedProfile);
      
      // Check if profile has health history data
      if (profile.biologicalSex || profile.smokingStatus || profile.diabetesStatus || 
          profile.hasHypertension || profile.takingBPMedication) {
        // Profile has health history, show confirmation modal
        setProfileData(profile);
        setPendingScanType(scanType);
        setShowHealthHistoryModal(true);
      } else {
        // Profile exists but no health history, proceed directly
        proceedToScan(scanType);
      }
    } else {
      // No profile at all, show profile required modal
      setPendingScanType(scanType);
      setShowProfileRequiredModal(true);
    }
  };
  
  const handleBodyScan = () => {
    checkHealthHistory('body');
  };
  
  const handleFaceScan = () => {
    checkHealthHistory('face');
  };
  
  const handleHealthHistoryProceed = () => {
    setShowHealthHistoryModal(false);
    if (pendingScanType) {
      proceedToScan(pendingScanType);
    }
  };
  
  const handleHealthHistoryUpdate = () => {
    setShowHealthHistoryModal(false);
    // Navigate to UserProfileScreen with the selected profile data
    const selectedProfile = localStorage.getItem('selectedProfile');
    if (selectedProfile) {
      const profile = JSON.parse(selectedProfile);
      navigate('/scan/user-profile', { 
        state: { 
          editingProfile: profile,
          scanType: pendingScanType
        } 
      });
    }
  };
  
  const handleProfileUpdate = () => {
    setShowProfileModal(false);
    // After profile update, proceed to scan
    if (pendingScanType) {
      proceedToScan(pendingScanType);
    }
  };

  const handleProfileRequired = (profileData: any) => {
    setShowProfileRequiredModal(false);
    // After creating basic profile, proceed to scan
    if (pendingScanType) {
      proceedToScan(pendingScanType);
    }
  };
  
  return (
    <Layout>
      <PageHeader 
        title="Choose Scan Type"
        subtitle="Select the type of health assessment you want to perform"
        showBackButton
        onBack={() => navigate('/home')}
      />
      <PageContent>
        <ScanOptionsGrid>
          <ScanCard onClick={handleBodyScan}>
            <ScanIcon>
              <svg viewBox="0 0 24 24" fill="currentColor">
                <path d="M12 1c-1.1 0-2 .9-2 2s.9 2 2 2 2-.9 2-2-.9-2-2-2zm9 9h-6v13h-2v-6h-2v6H9V10H3V8h18v2z" />
              </svg>
            </ScanIcon>
            <ScanTitle>Body Scan</ScanTitle>
            <ScanDescription>
              Get comprehensive body measurements and composition analysis
            </ScanDescription>
            <FeatureList>
              <FeatureItem>Body measurements (chest, waist, hip)</FeatureItem>
              <FeatureItem>Body fat percentage</FeatureItem>
              <FeatureItem>Muscle mass estimation</FeatureItem>
              <FeatureItem>BMI and health insights</FeatureItem>
              <FeatureItem>3D body visualization</FeatureItem>
            </FeatureList>
            <ScanDuration>
              <div className="duration">2-3 min</div>
              <div className="label">Scan duration</div>
            </ScanDuration>
          </ScanCard>
          
          <ScanCard onClick={handleFaceScan}>
            <ScanIcon>
              <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2">
                <path d="M12 4.5C7 4.5 2.73 7.61 1 12c1.73 4.39 6 7.5 11 7.5s9.27-3.11 11-7.5c-1.73-4.39-6-7.5-11-7.5zM12 17c-2.76 0-5-2.24-5-5s2.24-5 5-5 5 2.24 5 5-2.24 5-5 5zm0-8c-1.66 0-3 1.34-3 3s1.34 3 3 3 3-1.34 3-3-1.34-3-3-3z" />
                <path d="M21.53 9.47l.53.53M14.83 4.83l.35-.6m-6.36.6l-.35-.6m-3.64 4.3l-.53.53" strokeLinecap="round" />
              </svg>
            </ScanIcon>
            <ScanTitle>Face Scan</ScanTitle>
            <ScanDescription>
              Monitor vital signs and stress levels through facial analysis
            </ScanDescription>
            <FeatureList>
              <FeatureItem>Heart rate variability (HRV)</FeatureItem>
              <FeatureItem>Stress level assessment</FeatureItem>
              <FeatureItem>Blood pressure estimation</FeatureItem>
              <FeatureItem>Respiratory rate</FeatureItem>
              <FeatureItem>Oxygen saturation</FeatureItem>
            </FeatureList>
            <ScanDuration>
              <div className="duration">60 sec</div>
              <div className="label">Scan duration</div>
            </ScanDuration>
          </ScanCard>
        </ScanOptionsGrid>
      </PageContent>
      
      <HealthHistoryConfirmationModal
        isOpen={showHealthHistoryModal}
        onClose={() => setShowHealthHistoryModal(false)}
        onProceed={handleHealthHistoryProceed}
        onUpdate={handleHealthHistoryUpdate}
        profileData={profileData}
      />
      
      <ProfileCreationModal
        isOpen={showProfileModal}
        onClose={() => setShowProfileModal(false)}
        onSkip={handleProfileUpdate}
        initialData={profileData}
        isEditMode={true}
      />
      
      <ProfileRequiredModal
        isOpen={showProfileRequiredModal}
        onClose={() => setShowProfileRequiredModal(false)}
        onComplete={handleProfileRequired}
        scanType={pendingScanType || 'body'}
      />
    </Layout>
  );
};