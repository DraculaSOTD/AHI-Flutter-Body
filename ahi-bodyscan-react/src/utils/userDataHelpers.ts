// User Data Helper Functions for Profile Management
// Designed to be easily convertible to database operations

export interface UserProfile {
  id: string;
  name: string;
  isMainProfile?: boolean;
  age?: number;
  gender: string;
  height: number;
  weight: number;
  heightUnit?: 'cm' | 'ft';
  weightUnit?: 'kg' | 'lbs';
  dateOfBirth?: string;
  smokingStatus?: string;
  diabetesStatus?: string;
  hasHypertension?: string;
  takingBPMedication?: string;
  exerciseLevel?: 'inactive' | 'exercise10Mins' | 'exercise20to60Mins' | 'exercise1to3Hours' | 'exerciseOver3Hours';
  chronicMedication?: 'none' | 'oneOrTwoDiseases' | 'threeOrMoreDiseases';
  createdAt?: string;
  updatedAt?: string;
}

export interface UserData {
  userId: string;
  mainProfile: UserProfile;
  additionalProfiles: UserProfile[];
  lastUpdated: string;
}

// Get all user data including profiles
export const getUserData = (userEmail: string): UserData | null => {
  try {
    const key = `userData_${userEmail}`;
    const storedData = localStorage.getItem(key);
    
    if (storedData) {
      return JSON.parse(storedData);
    }
    
    // If no user data exists, try to migrate from old format
    const userProfile = localStorage.getItem('userProfile');
    const userProfiles = localStorage.getItem('userProfiles');
    
    if (userProfile) {
      const profile = JSON.parse(userProfile);
      const userName = userEmail.split('@')[0];
      
      // Calculate age from date of birth if available
      let age = 30; // default
      if (profile.dateOfBirth) {
        const birthDate = new Date(profile.dateOfBirth);
        const today = new Date();
        age = today.getFullYear() - birthDate.getFullYear();
        const monthDiff = today.getMonth() - birthDate.getMonth();
        if (monthDiff < 0 || (monthDiff === 0 && today.getDate() < birthDate.getDate())) {
          age--;
        }
      }
      
      const mainProfile: UserProfile = {
        id: 'main_profile',
        name: userName,
        isMainProfile: true,
        age,
        gender: profile.gender || profile.biologicalSex || 'male',
        height: profile.height || 175,
        weight: 70, // default weight
        heightUnit: profile.heightUnit || 'cm',
        weightUnit: 'kg',
        dateOfBirth: profile.dateOfBirth,
        smokingStatus: profile.smokingStatus,
        diabetesStatus: profile.diabetesStatus,
        hasHypertension: profile.hasHypertension,
        takingBPMedication: profile.takingBPMedication,
        createdAt: profile.lastUpdated || new Date().toISOString(),
        updatedAt: new Date().toISOString()
      };
      
      const additionalProfiles = userProfiles ? JSON.parse(userProfiles) : [];
      
      const userData: UserData = {
        userId: userEmail,
        mainProfile,
        additionalProfiles,
        lastUpdated: new Date().toISOString()
      };
      
      // Save migrated data
      saveUserData(userEmail, userData);
      return userData;
    }
    
    return null;
  } catch (error) {
    console.error('Error getting user data:', error);
    return null;
  }
};

// Save user data
export const saveUserData = (userEmail: string, userData: UserData): void => {
  try {
    const key = `userData_${userEmail}`;
    userData.lastUpdated = new Date().toISOString();
    localStorage.setItem(key, JSON.stringify(userData));
  } catch (error) {
    console.error('Error saving user data:', error);
  }
};

// Add new profile to user
export const addProfileToUser = (userEmail: string, profile: Omit<UserProfile, 'id' | 'createdAt' | 'updatedAt'>): UserProfile | null => {
  try {
    const userData = getUserData(userEmail);
    if (!userData) {
      console.error('No user data found');
      return null;
    }
    
    const newProfile: UserProfile = {
      ...profile,
      name: profile.name || 'Profile',
      id: `profile_${Date.now()}`,
      createdAt: new Date().toISOString(),
      updatedAt: new Date().toISOString()
    };
    
    userData.additionalProfiles.push(newProfile);
    saveUserData(userEmail, userData);
    
    return newProfile;
  } catch (error) {
    console.error('Error adding profile:', error);
    return null;
  }
};

// Update existing profile
export const updateUserProfile = (userEmail: string, profileId: string, updates: Partial<UserProfile>): boolean => {
  try {
    const userData = getUserData(userEmail);
    if (!userData) return false;
    
    if (profileId === 'main_profile') {
      userData.mainProfile = {
        ...userData.mainProfile,
        ...updates,
        id: 'main_profile',
        isMainProfile: true,
        updatedAt: new Date().toISOString()
      };
    } else {
      const profileIndex = userData.additionalProfiles.findIndex(p => p.id === profileId);
      if (profileIndex !== -1) {
        userData.additionalProfiles[profileIndex] = {
          ...userData.additionalProfiles[profileIndex],
          ...updates,
          updatedAt: new Date().toISOString()
        };
      } else {
        return false;
      }
    }
    
    saveUserData(userEmail, userData);
    return true;
  } catch (error) {
    console.error('Error updating profile:', error);
    return false;
  }
};

// Delete profile (cannot delete main profile)
export const deleteUserProfile = (userEmail: string, profileId: string): boolean => {
  try {
    if (profileId === 'main_profile') {
      console.error('Cannot delete main profile');
      return false;
    }
    
    const userData = getUserData(userEmail);
    if (!userData) return false;
    
    userData.additionalProfiles = userData.additionalProfiles.filter(p => p.id !== profileId);
    saveUserData(userEmail, userData);
    
    // Delete associated scan data
    const { deleteProfileScanData } = require('./scanDataHelpers');
    deleteProfileScanData(profileId);
    
    // If this was the selected profile, clear selection
    const selectedProfile = localStorage.getItem('selectedProfile');
    if (selectedProfile) {
      const parsed = JSON.parse(selectedProfile);
      if (parsed.id === profileId) {
        localStorage.removeItem('selectedProfile');
        localStorage.removeItem('selectedProfileId');
      }
    }
    
    return true;
  } catch (error) {
    console.error('Error deleting profile:', error);
    return false;
  }
};

// Get all profiles for display (main + additional)
export const getAllUserProfiles = (userEmail: string): UserProfile[] => {
  const userData = getUserData(userEmail);
  if (!userData) return [];
  
  return [userData.mainProfile, ...userData.additionalProfiles];
};

// Initialize user data for new user
export const initializeUserData = (userEmail: string, profileData: any): void => {
  const userName = userEmail.split('@')[0];
  
  // Calculate age from date of birth if available
  let age = 30; // default
  if (profileData.dateOfBirth) {
    const birthDate = new Date(profileData.dateOfBirth);
    const today = new Date();
    age = today.getFullYear() - birthDate.getFullYear();
    const monthDiff = today.getMonth() - birthDate.getMonth();
    if (monthDiff < 0 || (monthDiff === 0 && today.getDate() < birthDate.getDate())) {
      age--;
    }
  }
  
  const mainProfile: UserProfile = {
    id: 'main_profile',
    name: userName,
    isMainProfile: true,
    age,
    gender: profileData.gender || 'male',
    height: profileData.height || 175,
    weight: profileData.weight || 70,
    heightUnit: profileData.heightUnit || 'cm',
    weightUnit: profileData.weightUnit || 'kg',
    dateOfBirth: profileData.dateOfBirth,
    smokingStatus: profileData.smokingStatus,
    diabetesStatus: profileData.diabetesStatus,
    hasHypertension: profileData.hasHypertension,
    takingBPMedication: profileData.takingBPMedication,
    createdAt: new Date().toISOString(),
    updatedAt: new Date().toISOString()
  };
  
  const userData: UserData = {
    userId: userEmail,
    mainProfile,
    additionalProfiles: [],
    lastUpdated: new Date().toISOString()
  };
  
  saveUserData(userEmail, userData);
};