import React from 'react';
import { useNavigate } from 'react-router-dom';
import { UnifiedProfileModal, UnifiedProfileData } from './UnifiedProfileModal';
import { initializeUserData, addProfileToUser } from '../../utils/userDataHelpers';

interface ProfileCreationModalProps {
  isOpen: boolean;
  onClose: () => void;
  onSkip: () => void;
  initialData?: any;
  isEditMode?: boolean;
}

export const ProfileCreationModal: React.FC<ProfileCreationModalProps> = ({
  isOpen,
  onClose,
  onSkip,
  initialData,
  isEditMode = false
}) => {
  const navigate = useNavigate();

  const handleComplete = (profileData: UnifiedProfileData) => {
    try {
      // Calculate age from date of birth
      const calculateAge = (dob: string) => {
        const birthDate = new Date(dob);
        const today = new Date();
        let age = today.getFullYear() - birthDate.getFullYear();
        const monthDiff = today.getMonth() - birthDate.getMonth();
        if (monthDiff < 0 || (monthDiff === 0 && today.getDate() < birthDate.getDate())) {
          age--;
        }
        return age;
      };

      const userEmail = localStorage.getItem('userEmail') || 'user@example.com';
      
      // Save profile data
      const fullProfileData = {
        ...profileData,
        lastUpdated: new Date().toISOString(),
        age: calculateAge(profileData.dateOfBirth)
      };

      // Initialize user data for first-time users
      if (!localStorage.getItem('profileCreated')) {
        initializeUserData(userEmail, fullProfileData);
        localStorage.setItem('profileCreated', 'true');
        localStorage.setItem('userProfile', JSON.stringify(fullProfileData));
      } else if (isEditMode) {
        // Update existing profile
        localStorage.setItem('userProfile', JSON.stringify(fullProfileData));
      } else {
        // Add as additional profile for existing users
        const userName = userEmail.split('@')[0];
        
        const newProfile = addProfileToUser(userEmail, {
          name: userName + '_' + Date.now(),
          age: calculateAge(profileData.dateOfBirth),
          gender: profileData.gender,
          height: profileData.height,
          weight: profileData.weight,
          heightUnit: profileData.heightUnit,
          weightUnit: profileData.weightUnit,
          dateOfBirth: profileData.dateOfBirth,
          exerciseLevel: profileData.exerciseLevel,
          chronicMedication: profileData.chronicMedication,
          smokingStatus: profileData.smokingStatus,
          diabetesStatus: profileData.diabetesStatus,
          hasHypertension: profileData.hasHypertension,
          takingBPMedication: profileData.takingBPMedication
        });
        
        if (newProfile) {
          localStorage.setItem('selectedProfile', JSON.stringify(newProfile));
        }
      }

      // Navigate to home or call onSkip if in edit mode
      if (isEditMode) {
        onSkip(); // In edit mode, onSkip is used to proceed after update
      } else {
        navigate('/home');
      }
    } catch (error) {
      console.error('Error saving profile:', error);
    }
  };

  // Convert initialData to UnifiedProfileData format if provided
  const unifiedInitialData: Partial<UnifiedProfileData> | undefined = initialData ? {
    gender: initialData.gender || initialData.biologicalSex,
    dateOfBirth: initialData.dateOfBirth,
    height: initialData.height,
    weight: initialData.weight || 70,
    heightUnit: initialData.heightUnit || 'cm',
    weightUnit: initialData.weightUnit || 'kg',
    exerciseLevel: initialData.exerciseLevel || 'inactive',
    chronicMedication: initialData.chronicMedication || 'none'
  } : undefined;

  return (
    <UnifiedProfileModal
      isOpen={isOpen}
      onClose={onClose}
      onComplete={handleComplete}
      title={isEditMode ? "Update Your Profile" : "Complete Your Profile"}
      submitButtonText={isEditMode ? "Update Profile" : "Save Profile"}
      initialData={unifiedInitialData}
    />
  );
};