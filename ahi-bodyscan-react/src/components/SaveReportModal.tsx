import React, { useState } from 'react';
import styled from 'styled-components';
import { Modal, ModalBody, ModalFooter } from './common/Modal';
import { PrimaryGradientButton, OutlinedButton } from './common/Button';
import { theme } from '../styles/theme';

const FormContainer = styled.div`
  display: flex;
  flex-direction: column;
  gap: ${theme.dimensions.paddingLarge};
`;

const Label = styled.label`
  font-size: ${theme.typography.bodyMedium.fontSize};
  color: ${theme.colors.textSecondary};
  margin-bottom: ${theme.dimensions.paddingSmall};
`;

const Input = styled.input`
  width: 100%;
  padding: ${theme.dimensions.paddingMedium};
  background: ${theme.colors.backgroundWhite};
  border: 1px solid ${theme.colors.borderLight};
  border-radius: ${theme.dimensions.radiusMedium};
  font-size: ${theme.typography.bodyMedium.fontSize};
  color: ${theme.colors.textPrimary};
  transition: all ${theme.transitions.fast};
  
  &:focus {
    outline: none;
    border-color: ${theme.colors.primaryPurple};
    box-shadow: 0 0 0 3px ${theme.colors.primaryPurple}20;
  }
  
  &::placeholder {
    color: ${theme.colors.textTertiary};
  }
`;

const HelpText = styled.p`
  font-size: ${theme.typography.bodySmall.fontSize};
  color: ${theme.colors.textTertiary};
  margin: 0;
`;

interface SaveReportModalProps {
  open: boolean;
  onClose: () => void;
  onSave: (name: string) => void;
  scanType: 'body' | 'face';
  profileName?: string;
}

export const SaveReportModal: React.FC<SaveReportModalProps> = ({
  open,
  onClose,
  onSave,
  scanType,
  profileName
}) => {
  const [reportName, setReportName] = useState('');

  const getDefaultName = () => {
    // Use profile name as default
    const selectedProfile = localStorage.getItem('selectedProfile');
    const parsedProfile = selectedProfile ? JSON.parse(selectedProfile) : null;
    return profileName || parsedProfile?.name || 'User';
  };

  const handleSave = () => {
    const nameToSave = reportName.trim() || getDefaultName();
    onSave(nameToSave);
    setReportName('');
  };

  const handleClose = () => {
    setReportName('');
    onClose();
  };

  const handleKeyPress = (e: React.KeyboardEvent<HTMLInputElement>) => {
    if (e.key === 'Enter') {
      handleSave();
    }
  };

  return (
    <Modal 
      isOpen={open} 
      onClose={handleClose} 
      title="Save Report"
      size="small"
    >
      <ModalBody>
        <FormContainer>
          <div>
            <Label htmlFor="report-name">Report Name</Label>
            <Input
              id="report-name"
              type="text"
              placeholder={getDefaultName()}
              value={reportName}
              onChange={(e: React.ChangeEvent<HTMLInputElement>) => setReportName(e.target.value)}
              onKeyPress={handleKeyPress}
              autoFocus
            />
            <HelpText>
              Enter a name for this report to help you find it later
            </HelpText>
          </div>
        </FormContainer>
      </ModalBody>
      <ModalFooter>
        <OutlinedButton onClick={handleClose}>
          Cancel
        </OutlinedButton>
        <PrimaryGradientButton onClick={handleSave}>
          Save Report
        </PrimaryGradientButton>
      </ModalFooter>
    </Modal>
  );
};