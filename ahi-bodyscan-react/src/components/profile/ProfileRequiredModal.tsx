import React from 'react';
import { UnifiedProfileModal, UnifiedProfileData } from './UnifiedProfileModal';

interface ProfileRequiredModalProps {
  isOpen: boolean;
  onClose: () => void;
  onComplete: (profileData: BasicProfileData) => void;
  scanType: 'body' | 'face';
}

interface BasicProfileData {
  gender: string;
  dateOfBirth: string;
  height: number;
  weight: number;
  heightUnit: 'cm' | 'ft';
  weightUnit: 'kg' | 'lbs';
  exerciseLevel?: 'inactive' | 'exercise10Mins' | 'exercise20to60Mins' | 'exercise1to3Hours' | 'exerciseOver3Hours';
  chronicMedication?: 'none' | 'oneOrTwoDiseases' | 'threeOrMoreDiseases';
}

export const ProfileRequiredModal: React.FC<ProfileRequiredModalProps> = ({
  isOpen,
  onClose,
  onComplete,
  scanType
}) => {
  const handleComplete = (profileData: UnifiedProfileData) => {
    // Convert UnifiedProfileData to BasicProfileData
    const basicProfileData: BasicProfileData = {
      gender: profileData.gender,
      dateOfBirth: profileData.dateOfBirth,
      height: profileData.height,
      weight: profileData.weight,
      heightUnit: profileData.heightUnit,
      weightUnit: profileData.weightUnit,
      exerciseLevel: profileData.exerciseLevel,
      chronicMedication: profileData.chronicMedication
    };
    
    onComplete(basicProfileData);
  };

  return (
    <UnifiedProfileModal
      isOpen={isOpen}
      onClose={onClose}
      onComplete={handleComplete}
      title="Complete Your Profile First"
      scanType={scanType}
      submitButtonText="Save Profile & Continue"
    />
  );
};