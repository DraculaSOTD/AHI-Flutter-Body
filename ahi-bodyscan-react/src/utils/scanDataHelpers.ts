// Scan Data Helper Functions for Managing Profile Scan Progress and Results

export interface ScanResult {
  id: string;
  profileId: string;
  scanType: 'body' | 'face';
  status: 'completed' | 'in-progress';
  progress: number;
  startedAt: string;
  completedAt?: string;
  results?: {
    // Body scan results
    bodyFat?: number;
    muscleMass?: number;
    bmi?: number;
    waist?: number;
    chest?: number;
    hip?: number;
    thigh?: number;
    inseam?: number;
    visceralFat?: number;
    fatFreeMass?: number;
    // Face scan results
    heartRate?: number;
    bloodPressure?: string;
    systolicBP?: number;
    diastolicBP?: number;
    stressLevel?: string;
    respiratoryRate?: number;
    oxygenSaturation?: number;
    heartRateVariability?: number;
  };
}

export interface ProfileScanData {
  profileId: string;
  bodyScans: ScanResult[];
  faceScans: ScanResult[];
  lastUpdated: string;
}

export interface ScanProgress {
  profileId: string;
  profileName: string;
  totalScansRequired: number;
  completedScans: number;
  progressPercentage: number;
  completedScanTypes: ('body' | 'face')[];
  pendingScanTypes: ('body' | 'face')[];
  hasCompletedAllScans: boolean;
  latestBodyScan?: ScanResult;
  latestFaceScan?: ScanResult;
}

// Required scan types for a complete health assessment
const REQUIRED_SCAN_TYPES: ('body' | 'face')[] = ['body', 'face'];

// Get all scan data for a specific profile
export const getProfileScanData = (profileId: string): ProfileScanData | null => {
  try {
    const key = `scanData_${profileId}`;
    const storedData = localStorage.getItem(key);
    
    if (storedData) {
      return JSON.parse(storedData);
    }
    
    return null;
  } catch (error) {
    console.error('Error getting profile scan data:', error);
    return null;
  }
};

// Save scan data for a profile
export const saveProfileScanData = (profileId: string, scanData: ProfileScanData): void => {
  try {
    const key = `scanData_${profileId}`;
    scanData.lastUpdated = new Date().toISOString();
    localStorage.setItem(key, JSON.stringify(scanData));
  } catch (error) {
    console.error('Error saving profile scan data:', error);
  }
};

// Add or update a scan result for a profile
export const saveScanResult = (profileId: string, scanResult: ScanResult): void => {
  try {
    let scanData = getProfileScanData(profileId);
    
    if (!scanData) {
      scanData = {
        profileId,
        bodyScans: [],
        faceScans: [],
        lastUpdated: new Date().toISOString()
      };
    }
    
    // Update the scan result with profileId
    scanResult.profileId = profileId;
    
    // Add or update the scan in the appropriate array
    if (scanResult.scanType === 'body') {
      const existingIndex = scanData.bodyScans.findIndex(s => s.id === scanResult.id);
      if (existingIndex >= 0) {
        scanData.bodyScans[existingIndex] = scanResult;
      } else {
        scanData.bodyScans.push(scanResult);
      }
    } else {
      const existingIndex = scanData.faceScans.findIndex(s => s.id === scanResult.id);
      if (existingIndex >= 0) {
        scanData.faceScans[existingIndex] = scanResult;
      } else {
        scanData.faceScans.push(scanResult);
      }
    }
    
    saveProfileScanData(profileId, scanData);
  } catch (error) {
    console.error('Error saving scan result:', error);
  }
};

// Get scan progress for a profile
export const getProfileScanProgress = (profileId: string, profileName: string): ScanProgress => {
  const scanData = getProfileScanData(profileId);
  
  const completedBodyScans = scanData?.bodyScans.filter(s => s.status === 'completed') || [];
  const completedFaceScans = scanData?.faceScans.filter(s => s.status === 'completed') || [];
  
  const completedScanTypes: ('body' | 'face')[] = [];
  if (completedBodyScans.length > 0) completedScanTypes.push('body');
  if (completedFaceScans.length > 0) completedScanTypes.push('face');
  
  const pendingScanTypes = REQUIRED_SCAN_TYPES.filter(type => !completedScanTypes.includes(type));
  
  const totalScansRequired = REQUIRED_SCAN_TYPES.length;
  const completedScans = completedScanTypes.length;
  const progressPercentage = Math.round((completedScans / totalScansRequired) * 100);
  
  // Get latest completed scans
  const latestBodyScan = completedBodyScans.sort((a, b) => 
    new Date(b.completedAt || b.startedAt).getTime() - new Date(a.completedAt || a.startedAt).getTime()
  )[0];
  
  const latestFaceScan = completedFaceScans.sort((a, b) => 
    new Date(b.completedAt || b.startedAt).getTime() - new Date(a.completedAt || a.startedAt).getTime()
  )[0];
  
  return {
    profileId,
    profileName,
    totalScansRequired,
    completedScans,
    progressPercentage,
    completedScanTypes,
    pendingScanTypes,
    hasCompletedAllScans: completedScans === totalScansRequired,
    latestBodyScan,
    latestFaceScan
  };
};

// Get all in-progress scans for a profile
export const getInProgressScans = (profileId: string): ScanResult[] => {
  const scanData = getProfileScanData(profileId);
  if (!scanData) return [];
  
  const inProgressScans = [
    ...scanData.bodyScans.filter(s => s.status === 'in-progress'),
    ...scanData.faceScans.filter(s => s.status === 'in-progress')
  ];
  
  return inProgressScans.sort((a, b) => 
    new Date(b.startedAt).getTime() - new Date(a.startedAt).getTime()
  );
};

// Delete all scan data for a profile
export const deleteProfileScanData = (profileId: string): void => {
  try {
    const key = `scanData_${profileId}`;
    localStorage.removeItem(key);
  } catch (error) {
    console.error('Error deleting profile scan data:', error);
  }
};

// Migrate legacy scan data to profile-based storage
export const migrateLegacyScanData = (userEmail: string): void => {
  try {
    // Check for legacy assessments data
    const assessments = localStorage.getItem('assessments');
    if (!assessments) return;
    
    const parsedAssessments = JSON.parse(assessments);
    
    // Get main profile ID
    const userData = localStorage.getItem(`userData_${userEmail}`);
    if (!userData) return;
    
    const parsedUserData = JSON.parse(userData);
    const mainProfileId = parsedUserData.mainProfile.id;
    
    // Convert legacy assessments to new format
    const scanData: ProfileScanData = {
      profileId: mainProfileId,
      bodyScans: [],
      faceScans: [],
      lastUpdated: new Date().toISOString()
    };
    
    parsedAssessments.forEach((assessment: any) => {
      const scanResult: ScanResult = {
        id: assessment.id,
        profileId: mainProfileId,
        scanType: assessment.type,
        status: assessment.status === 'completed' ? 'completed' : 'in-progress',
        progress: assessment.progress || 0,
        startedAt: assessment.startedAt,
        completedAt: assessment.completedAt,
        results: assessment.results
      };
      
      if (assessment.type === 'body') {
        scanData.bodyScans.push(scanResult);
      } else {
        scanData.faceScans.push(scanResult);
      }
    });
    
    // Save migrated data
    saveProfileScanData(mainProfileId, scanData);
    
    // Remove legacy data
    localStorage.removeItem('assessments');
  } catch (error) {
    console.error('Error migrating legacy scan data:', error);
  }
};