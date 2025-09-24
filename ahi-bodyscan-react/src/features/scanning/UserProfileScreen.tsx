import React, { useState, useEffect } from 'react';
import { useNavigate, useLocation } from 'react-router-dom';
import { UnifiedProfileModal, UnifiedProfileData } from '../../components/profile/UnifiedProfileModal';
import { addProfileToUser, updateUserProfile } from '../../utils/userDataHelpers';

export const UserProfileScreen: React.FC = () => {
  const navigate = useNavigate();
  const location = useLocation();
  const scanType = location.state?.scanType || 'body';
  const isNewProfile = location.state?.isNewProfile || false;
  const editingProfile = location.state?.editingProfile || null;
  const isMainProfile = location.state?.isMainProfile || false;
  
  const [isOpen, setIsOpen] = useState(true);

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

  const handleComplete = async (profileData: UnifiedProfileData) => {
    console.log('UserProfileScreen - handleComplete called with:', profileData);
    try {
      const userEmail = localStorage.getItem('userEmail') || 'user@example.com';
      
      // For new profiles
      if (isNewProfile) {
        const newProfile = addProfileToUser(userEmail, {
          name: profileData.name || 'Profile',
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
          
          // Navigate to home page after creating profile
          navigate('/home');
        }
      } 
      // For editing existing profiles
      else if (editingProfile) {
        const updatedProfile = {
          ...editingProfile,
          name: profileData.name || editingProfile.name,
          gender: profileData.gender,
          height: profileData.height,
          weight: profileData.weight,
          heightUnit: profileData.heightUnit,
          weightUnit: profileData.weightUnit,
          dateOfBirth: profileData.dateOfBirth,
          age: calculateAge(profileData.dateOfBirth),
          exerciseLevel: profileData.exerciseLevel,
          chronicMedication: profileData.chronicMedication,
          smokingStatus: profileData.smokingStatus,
          diabetesStatus: profileData.diabetesStatus,
          hasHypertension: profileData.hasHypertension,
          takingBPMedication: profileData.takingBPMedication,
          updatedAt: new Date().toISOString()
        };

        const success = updateUserProfile(userEmail, editingProfile.id, updatedProfile);
        
        if (success) {
          // Update selected profile if it's the one being edited
          const selectedProfile = localStorage.getItem('selectedProfile');
          if (selectedProfile) {
            const parsed = JSON.parse(selectedProfile);
            if (parsed.id === editingProfile.id) {
              localStorage.setItem('selectedProfile', JSON.stringify(updatedProfile));
            }
          }
          
          // If this is the main profile, update the main profile data
          if (isMainProfile) {
            localStorage.setItem('userProfile', JSON.stringify({
              ...updatedProfile,
              lastUpdated: new Date().toISOString()
            }));
          }
        }
        
        navigate('/home');
      } else {
        navigate('/home');
      }
    } catch (error) {
      console.error('Error saving profile:', error);
      alert('Failed to save profile. Please try again.');
      navigate('/home');
    }
  };

  const handleClose = () => {
    setIsOpen(false);
    navigate('/home');
  };

  // Convert editing profile to UnifiedProfileData format
  const initialData: Partial<UnifiedProfileData> | undefined = editingProfile ? {
    name: editingProfile.name || `Profile ${editingProfile.id}`,
    gender: editingProfile.gender,
    dateOfBirth: editingProfile.dateOfBirth,
    height: editingProfile.height,
    weight: editingProfile.weight,
    heightUnit: editingProfile.heightUnit || 'cm',
    weightUnit: editingProfile.weightUnit || 'kg',
    exerciseLevel: editingProfile.exerciseLevel || 'inactive',
    chronicMedication: editingProfile.chronicMedication || 'none',
    smokingStatus: editingProfile.smokingStatus || 'never',
    diabetesStatus: editingProfile.diabetesStatus || 'no',
    hasHypertension: editingProfile.hasHypertension || 'no',
    takingBPMedication: editingProfile.takingBPMedication || 'no'
  } : undefined;
  
  console.log('UserProfileScreen - editingProfile:', editingProfile);
  console.log('UserProfileScreen - initialData:', initialData);

  const modalTitle = isNewProfile 
    ? 'Create New Profile' 
    : editingProfile 
      ? `Edit Profile: ${editingProfile.name}`
      : 'Profile Information';

  const submitButtonText = isNewProfile
    ? 'Create Profile'
    : editingProfile
      ? 'Update Profile'
      : 'Save & Continue';

  return (
    <UnifiedProfileModal
      isOpen={isOpen}
      onClose={handleClose}
      onComplete={handleComplete}
      title={modalTitle}
      submitButtonText={submitButtonText}
      initialData={initialData}
      scanType={scanType}
    />
  );
};