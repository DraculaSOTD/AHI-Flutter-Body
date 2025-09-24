import React from 'react';
import styled from 'styled-components';
import { theme } from '../../styles/theme';
import { Modal, ModalBody, ModalFooter } from '../common/Modal';
import { PrimaryGradientButton, OutlinedButton } from '../common/Button';

interface HealthHistoryConfirmationModalProps {
  isOpen: boolean;
  onClose: () => void;
  onProceed: () => void;
  onUpdate: () => void;
  profileData: any;
}

const ContentWrapper = styled.div`
  padding: ${theme.dimensions.paddingLarge} 0;
`;

const Question = styled.h3`
  font-size: ${theme.typography.headingMedium.fontSize};
  font-weight: ${theme.typography.headingMedium.fontWeight};
  color: ${theme.colors.textPrimary};
  text-align: center;
  margin-bottom: ${theme.dimensions.paddingXLarge};
`;

const HealthHistorySection = styled.div`
  background: ${theme.colors.backgroundLight};
  border-radius: ${theme.dimensions.radiusMedium};
  padding: ${theme.dimensions.paddingLarge};
  margin-bottom: ${theme.dimensions.paddingLarge};
`;

const SectionTitle = styled.h4`
  font-size: ${theme.typography.headingSmall.fontSize};
  font-weight: ${theme.typography.headingSmall.fontWeight};
  color: ${theme.colors.textPrimary};
  margin-bottom: ${theme.dimensions.paddingMedium};
`;

const HealthList = styled.ul`
  list-style: none;
  padding: 0;
  margin: 0;
`;

const HealthItem = styled.li`
  display: flex;
  align-items: center;
  gap: ${theme.dimensions.paddingSmall};
  padding: ${theme.dimensions.paddingSmall} 0;
  font-size: ${theme.typography.bodyMedium.fontSize};
  color: ${theme.colors.textSecondary};
  
  &::before {
    content: '•';
    color: ${theme.colors.primaryPurple};
    font-weight: bold;
    font-size: 20px;
  }
`;

const ButtonGroup = styled.div`
  display: flex;
  gap: ${theme.dimensions.paddingMedium};
  width: 100%;
`;

// Helper function to format health history for display
const formatHealthHistoryDisplay = (profileData: any) => {
  const items: string[] = [];
  
  if (!profileData) {
    return items;
  }
  
  if (profileData.biologicalSex) {
    items.push(`Biological sex: ${profileData.biologicalSex === 'male' ? 'Male' : 'Female'}`);
  }
  
  if (profileData.smokingStatus) {
    const smokingMap: Record<string, string> = {
      'never': 'Never smoked',
      'past': 'Past smoker',
      'current': 'Current smoker'
    };
    items.push(smokingMap[profileData.smokingStatus] || profileData.smokingStatus);
  }
  
  if (profileData.diabetesStatus) {
    const diabetesMap: Record<string, string> = {
      'no': 'No diabetes',
      'type1': 'Type 1 diabetes',
      'type2': 'Type 2 diabetes'
    };
    items.push(diabetesMap[profileData.diabetesStatus] || profileData.diabetesStatus);
  }
  
  if (profileData.hasHypertension) {
    items.push(profileData.hasHypertension === 'yes' ? 'Has hypertension' : 'No hypertension');
  }
  
  if (profileData.takingBPMedication) {
    items.push(profileData.takingBPMedication === 'yes' ? 'Taking BP medication' : 'Not taking BP medication');
  }
  
  return items;
};

export const HealthHistoryConfirmationModal: React.FC<HealthHistoryConfirmationModalProps> = ({
  isOpen,
  onClose,
  onProceed,
  onUpdate,
  profileData
}) => {
  const healthItems = formatHealthHistoryDisplay(profileData);
  
  return (
    <Modal isOpen={isOpen} onClose={onClose} title="Health History Check" size="medium">
      <ModalBody>
        <ContentWrapper>
          <Question>Has any of your health information changed?</Question>
          
          <HealthHistorySection>
            <SectionTitle>Your Current Health Information:</SectionTitle>
            <HealthList>
              {healthItems.map((item, index) => (
                <HealthItem key={index}>{item}</HealthItem>
              ))}
            </HealthList>
          </HealthHistorySection>
        </ContentWrapper>
      </ModalBody>
      
      <ModalFooter>
        <ButtonGroup>
          <OutlinedButton onClick={onUpdate} style={{ flex: 1 }}>
            Update Profile
          </OutlinedButton>
          <PrimaryGradientButton onClick={onProceed} style={{ flex: 1 }}>
            Scan
          </PrimaryGradientButton>
        </ButtonGroup>
      </ModalFooter>
    </Modal>
  );
};