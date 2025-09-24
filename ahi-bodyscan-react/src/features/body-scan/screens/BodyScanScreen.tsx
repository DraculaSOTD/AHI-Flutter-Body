import React, { useState, useEffect } from 'react';
import { useNavigate } from 'react-router-dom';
import { CameraService } from '../../../services/camera/CameraService';
import { 
  BodyScanService, 
  BodyScanPhase, 
  BodyScanPose, 
  BodyScanResult,
  UserProfile 
} from '../../../services/bodyScan';
import { BodyScanCapture } from '../components/BodyScanCapture';
import { BodyScanResults } from '../components/BodyScanResults';
import { LoadingOverlay } from '../../../components/common/LoadingOverlay';
import { saveScanResult, getProfileScanData } from '../../../utils/scanDataHelpers';

export const BodyScanScreen: React.FC = () => {
  const navigate = useNavigate();
  const [cameraService] = useState(() => new CameraService());
  const [bodyScanService] = useState(() => new BodyScanService(cameraService));
  const [phase, setPhase] = useState<BodyScanPhase>(BodyScanPhase.IDLE);
  const [scanResult, setScanResult] = useState<BodyScanResult | null>(null);
  const [isInitializing, setIsInitializing] = useState(true);
  const [currentScanId, setCurrentScanId] = useState<string | null>(null);
  const [selectedProfileId, setSelectedProfileId] = useState<string | null>(null);
  
  const [capturedImages, setCapturedImages] = useState<{
    front?: string;
    side?: string;
  }>({});
  const [error, setError] = useState<string | null>(null);
  
  console.log('BodyScanScreen render:', { isInitializing, phase, error });

  useEffect(() => {
    // Subscribe to phase changes
    bodyScanService.onPhaseChange((newPhase) => {
      console.log('Phase changed to:', newPhase);
      setPhase(newPhase);
    });

    // Initialize camera
    console.log('Initializing camera service...');
    cameraService.initialize()
      .then(() => console.log('Camera initialized successfully'))
      .catch(err => console.error('Camera initialization failed:', err));
    
    // Auto-initialize with selected profile
    const initializeWithProfile = async () => {
      console.log('Starting profile initialization...');
      try {
        const selectedProfile = localStorage.getItem('selectedProfile');
        if (selectedProfile) {
          const profile = JSON.parse(selectedProfile);
          console.log('Found selected profile:', profile);
          setSelectedProfileId(profile.id);
          
          // Find the latest in-progress body scan for this profile
          const scanData = getProfileScanData(profile.id);
          if (scanData) {
            const inProgressBodyScan = scanData.bodyScans.find(s => s.status === 'in-progress');
            if (inProgressBodyScan) {
              setCurrentScanId(inProgressBodyScan.id);
              // Update progress to 25% when starting capture
              saveScanResult(profile.id, {
                ...inProgressBodyScan,
                progress: 25
              });
            }
          }
          
          // Convert to UserProfile format
          const userProfile: UserProfile = {
            height: profile.height || 175,
            weight: profile.weight || 70,
            gender: profile.gender || 'male',
            age: profile.age
          };
          
          // Initialize body scan service
          console.log('Initializing body scan service with profile:', userProfile);
          await bodyScanService.initialize(userProfile);
          console.log('Body scan service initialized successfully');
        } else {
          // No profile found, navigate back
          console.error('No selected profile found');
          navigate('/scan');
        }
      } catch (error) {
        console.error('Failed to initialize body scan:', error);
        setError(error instanceof Error ? error.message : 'Failed to initialize body scan');
        setPhase(BodyScanPhase.ERROR);
      } finally {
        console.log('Setting isInitializing to false');
        setIsInitializing(false);
      }
    };
    
    initializeWithProfile();

    // Cleanup
    return () => {
      bodyScanService.reset();
      cameraService.dispose();
    };
  }, [bodyScanService, cameraService, navigate]);


  const handleFrontCapture = (imageData: string) => {
    setCapturedImages(prev => ({ ...prev, front: imageData }));
    setPhase(BodyScanPhase.SIDE_CAPTURE);
    
    // Update progress to 50% after front capture
    if (currentScanId && selectedProfileId) {
      const scanData = getProfileScanData(selectedProfileId);
      if (scanData) {
        const scan = scanData.bodyScans.find(s => s.id === currentScanId);
        if (scan && scan.status === 'in-progress') {
          saveScanResult(selectedProfileId, {
            ...scan,
            progress: 50
          });
        }
      }
    }
  };

  const handleSideCapture = async (imageData: string) => {
    setCapturedImages(prev => ({ ...prev, side: imageData }));
    
    // Update progress to 75% during processing
    if (currentScanId && selectedProfileId) {
      const scanData = getProfileScanData(selectedProfileId);
      if (scanData) {
        const scan = scanData.bodyScans.find(s => s.id === currentScanId);
        if (scan && scan.status === 'in-progress') {
          saveScanResult(selectedProfileId, {
            ...scan,
            progress: 75
          });
        }
      }
    }
    
    try {
      const result = await bodyScanService.processImages();
      setScanResult(result);
    } catch (error) {
      console.error('Processing failed:', error);
      alert('Failed to process scan. Please try again.');
      handleNewScan();
    }
  };

  const handleSaveResults = () => {
    if (!scanResult) return;

    // Get profile ID - either from state or from localStorage
    const profileId = selectedProfileId || (() => {
      const selectedProfile = localStorage.getItem('selectedProfile');
      return selectedProfile ? JSON.parse(selectedProfile).id : null;
    })();
    
    if (!profileId) {
      console.error('No profile ID found for saving scan results');
      return;
    }

    // Update the scan to completed status with results
    const completedScan = {
      id: currentScanId || `body_${Date.now()}`,
      profileId,
      scanType: 'body' as const,
      status: 'completed' as const,
      progress: 100,
      startedAt: new Date().toISOString(),
      completedAt: new Date().toISOString(),
      results: {
        // Flatten the measurements from BodyScanResult
        bodyFat: scanResult.measurements.bodyFat,
        muscleMass: scanResult.measurements.muscleMass,
        bmi: scanResult.measurements.weight && bodyScanService.profile?.height 
          ? (scanResult.measurements.weight / Math.pow(bodyScanService.profile.height / 100, 2)) 
          : undefined,
        waist: scanResult.measurements.waist,
        chest: scanResult.measurements.chest,
        hip: scanResult.measurements.hip,
        thigh: scanResult.measurements.thigh,
        inseam: scanResult.measurements.inseam,
        visceralFat: scanResult.measurements.visceralFat,
        fatFreeMass: scanResult.measurements.weight - (scanResult.measurements.weight * scanResult.measurements.bodyFat / 100),
        metabolicAge: scanResult.measurements.metabolicAge,
        bodyType: scanResult.measurements.bodyType
      }
    };
    
    // Save using the profile-based scan data helper
    saveScanResult(profileId, completedScan);
    
    // Also save to bodyScanResults for backward compatibility
    const savedScans = JSON.parse(localStorage.getItem('bodyScanResults') || '[]');
    const scanWithId = {
      id: completedScan.id,
      ...scanResult,
      userProfile: bodyScanService.profile,
      profileId
    };
    
    savedScans.push(scanWithId);
    localStorage.setItem('bodyScanResults', JSON.stringify(savedScans));
    
    // Also save the latest scan separately for easy access
    localStorage.setItem('latestBodyScan', JSON.stringify(scanWithId));
    
    // Save to completedAssessments for reports page
    const completedAssessments = JSON.parse(localStorage.getItem('completedAssessments') || '[]');
    const selectedProfile = localStorage.getItem('selectedProfile');
    const profileData = selectedProfile ? JSON.parse(selectedProfile) : null;
    
    const assessment = {
      id: completedScan.id,
      type: 'body' as const,
      date: completedScan.completedAt,
      status: 'completed' as const,
      name: profileData?.name || 'User',
      profileName: profileData?.name,
      results: completedScan.results
    };
    
    completedAssessments.push(assessment);
    localStorage.setItem('completedAssessments', JSON.stringify(completedAssessments));
    
    console.log('Body scan saved:', scanWithId);
    navigate('/reports');
  };

  const handleNewScan = () => {
    bodyScanService.reset();
    setPhase(BodyScanPhase.IDLE);
    setScanResult(null);
    setCapturedImages({});
  };

  const handleCancel = () => {
    console.log('handleCancel called - navigating to /new-scan');
    bodyScanService.reset();
    navigate('/new-scan');
  };

  // Render based on current phase
  const renderContent = () => {
    console.log('renderContent called:', { isInitializing, phase, error });
    
    // Show loading during initialization or if service is not ready
    if (isInitializing || !bodyScanService.isInitialized) {
      return <LoadingOverlay isVisible={true} />;
    }
    
    console.log('Rendering phase:', phase);
    switch (phase) {
      case BodyScanPhase.IDLE:
      case BodyScanPhase.SETUP:
        return <LoadingOverlay isVisible={true} />;
        
      case BodyScanPhase.FRONT_CAPTURE:
        return (
          <BodyScanCapture
            bodyScanService={bodyScanService}
            pose={BodyScanPose.FRONT}
            onCapture={handleFrontCapture}
            onCancel={handleCancel}
          />
        );

      case BodyScanPhase.SIDE_CAPTURE:
        return (
          <BodyScanCapture
            bodyScanService={bodyScanService}
            pose={BodyScanPose.SIDE}
            onCapture={handleSideCapture}
            onCancel={handleCancel}
          />
        );

      case BodyScanPhase.PROCESSING:
        return <LoadingOverlay isVisible={true} />;

      case BodyScanPhase.COMPLETE:
        return scanResult ? (
          <BodyScanResults
            result={scanResult}
            onSave={handleSaveResults}
            onNewScan={handleNewScan}
          />
        ) : null;

      case BodyScanPhase.ERROR:
        return (
          <div className="flex flex-col items-center justify-center h-full p-4">
            <h2 className="text-xl font-semibold mb-4">Something went wrong</h2>
            <p className="text-gray-600 mb-6">{error || 'Please try again'}</p>
            <button
              onClick={handleNewScan}
              className="px-6 py-2 bg-blue-600 text-white rounded-lg"
            >
              Try Again
            </button>
          </div>
        );

      default:
        return null;
    }
  };

  return (
    <div className="min-h-screen bg-gray-50">
      {renderContent()}
    </div>
  );
};