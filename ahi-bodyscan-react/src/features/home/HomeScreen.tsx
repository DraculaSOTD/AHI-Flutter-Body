import React, { useState, useEffect } from 'react';
import styled from 'styled-components';
import { useNavigate } from 'react-router-dom';
import { theme } from '../../styles/theme';
import { Layout, PageHeader, PageContent } from '../../components/layout/Layout';
import { ActionCard } from '../../components/common/Card';
import { OutlinedButton } from '../../components/common/Button';
import { ScanProgressCard } from '../../components/common/ScanProgressCard';
import { LoadingModal } from '../../components/common/LoadingModal';
import { getAllUserProfiles, UserProfile, deleteUserProfile } from '../../utils/userDataHelpers';
import { 
  getProfileScanProgress, 
  getInProgressScans,
  migrateLegacyScanData,
  ScanProgress 
} from '../../utils/scanDataHelpers';
import { pdfService } from '../../services/pdf/PDFService';

interface Assessment {
  id: string;
  type: 'body' | 'face';
  status: 'in-progress' | 'completed';
  progress: number;
  startedAt: string;
  completedAt?: string;
  results?: {
    bodyFat?: number;
    muscleMass?: number;
    heartRate?: number;
    stressLevel?: string;
  };
}


const SectionTitle = styled.h2`
  font-size: ${theme.typography.headingMedium.fontSize};
  font-weight: ${theme.typography.headingMedium.fontWeight};
  color: ${theme.colors.textPrimary};
  margin-bottom: ${theme.dimensions.paddingLarge};
`;

const AssessmentGrid = styled.div`
  display: grid;
  gap: ${theme.dimensions.paddingLarge};
  margin-bottom: ${theme.dimensions.paddingXLarge};
  
  @media (min-width: ${theme.breakpoints.tablet}) {
    grid-template-columns: repeat(auto-fit, minmax(350px, 1fr));
  }
`;


const ProfilesContainer = styled.div`
  margin-bottom: ${theme.dimensions.paddingXLarge};
  margin-left: -${theme.dimensions.paddingMedium};
  margin-right: -${theme.dimensions.paddingMedium};
  padding: ${theme.dimensions.paddingMedium} ${theme.dimensions.paddingMedium};
  overflow-x: auto;
  overflow-y: hidden;
  -webkit-overflow-scrolling: touch;
  contain: layout;
  
  @media (min-width: ${theme.breakpoints.tablet}) {
    margin-left: -${theme.dimensions.paddingLarge};
    margin-right: -${theme.dimensions.paddingLarge};
    padding: ${theme.dimensions.paddingMedium} ${theme.dimensions.paddingLarge};
  }
  
  &::-webkit-scrollbar {
    height: 8px;
  }
  
  &::-webkit-scrollbar-track {
    background: ${theme.colors.backgroundLight};
    border-radius: ${theme.dimensions.radiusSmall};
  }
  
  &::-webkit-scrollbar-thumb {
    background: ${theme.colors.borderLight};
    border-radius: ${theme.dimensions.radiusSmall};
    
    &:hover {
      background: ${theme.colors.textTertiary};
    }
  }
`;

const ProfilesGrid = styled.div`
  display: grid;
  gap: ${theme.dimensions.paddingLarge};
  grid-template-rows: repeat(2, 1fr);
  grid-auto-flow: column;
  grid-auto-columns: minmax(280px, 1fr);
  
  @media (max-width: ${theme.breakpoints.mobile}) {
    grid-auto-columns: minmax(240px, 1fr);
  }
`;

const ProfileCard = styled(ActionCard)<{ $isSelected?: boolean }>`
  cursor: pointer;
  position: relative;
  border: 2px solid ${props => props.$isSelected ? 'transparent' : theme.colors.borderLight};
  background: ${props => props.$isSelected 
    ? `linear-gradient(${theme.colors.backgroundWhite}, ${theme.colors.backgroundWhite}) padding-box,
       linear-gradient(135deg, ${theme.colors.primaryPurple}, ${theme.colors.primaryBlue}) border-box`
    : theme.colors.backgroundWhite};
  transition: all ${theme.transitions.normal};
  
  ${props => props.$isSelected && `
    box-shadow: 0 0 20px rgba(138, 43, 226, 0.3);
    transform: scale(1.02);
  `}
  
  &:hover {
    transform: ${props => props.$isSelected ? 'scale(1.02)' : 'translateY(-4px)'};
    box-shadow: ${props => props.$isSelected 
      ? '0 0 25px rgba(138, 43, 226, 0.4)' 
      : theme.shadows.large};
  }
  
  .profile-avatar {
    width: 48px;
    height: 48px;
    border-radius: 50%;
    background: ${theme.gradients.primary};
    color: ${theme.colors.textWhite};
    display: flex;
    align-items: center;
    justify-content: center;
    font-size: 20px;
    font-weight: 600;
    margin-bottom: ${theme.dimensions.paddingMedium};
  }
  
  .profile-info {
    h4 {
      font-size: ${theme.typography.headingSmall.fontSize};
      font-weight: ${theme.typography.headingSmall.fontWeight};
      color: ${theme.colors.textPrimary};
      margin-bottom: ${theme.dimensions.paddingXSmall};
    }
    
    p {
      font-size: ${theme.typography.bodyMedium.fontSize};
      color: ${theme.colors.textSecondary};
    }
  }
`;

const IconColumn = styled.div`
  position: absolute;
  top: ${theme.dimensions.paddingMedium};
  right: ${theme.dimensions.paddingMedium};
  display: flex;
  flex-direction: column;
  gap: ${theme.dimensions.paddingXSmall};
`;

const IconButton = styled.button`
  background: ${theme.colors.backgroundWhite};
  border: 1px solid ${theme.colors.borderLight};
  padding: ${theme.dimensions.paddingXSmall};
  cursor: pointer;
  color: ${theme.colors.textTertiary};
  transition: all ${theme.transitions.fast};
  border-radius: ${theme.dimensions.radiusSmall};
  width: 32px;
  height: 32px;
  display: flex;
  align-items: center;
  justify-content: center;
  
  &:hover {
    background: ${theme.colors.backgroundLight};
    color: ${theme.colors.primaryPurple};
    border-color: ${theme.colors.primaryPurple};
  }
  
  svg {
    width: 16px;
    height: 16px;
  }
`;

const AddProfileCard = styled(ActionCard)`
  cursor: pointer;
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  min-height: 150px;
  border: 2px dashed ${theme.colors.borderLight};
  background: ${theme.colors.backgroundLight};
  
  &:hover {
    border-color: ${theme.colors.primaryPurple};
    background: ${theme.colors.backgroundWhite};
  }
  
  .add-icon {
    width: 48px;
    height: 48px;
    border-radius: 50%;
    background: ${theme.colors.primaryPurple}20;
    color: ${theme.colors.primaryPurple};
    display: flex;
    align-items: center;
    justify-content: center;
    margin-bottom: ${theme.dimensions.paddingSmall};
    
    svg {
      width: 24px;
      height: 24px;
    }
  }
  
  span {
    font-size: ${theme.typography.bodyMedium.fontSize};
    font-weight: 500;
    color: ${theme.colors.primaryPurple};
  }
`;

const EmptyStateMessage = styled.p`
  color: ${theme.colors.textTertiary};
  text-align: center;
  padding: ${theme.dimensions.paddingXLarge} ${theme.dimensions.paddingLarge};
  background: ${theme.colors.backgroundLight};
  border-radius: ${theme.dimensions.radiusMedium};
  font-size: ${theme.typography.bodyMedium.fontSize};
`;

const LatestResultsSection = styled.div`
  background: ${theme.colors.backgroundWhite};
  border-radius: ${theme.dimensions.radiusLarge};
  padding: ${theme.dimensions.paddingLarge};
  box-shadow: ${theme.shadows.small};
  
  h3 {
    font-size: ${theme.typography.headingSmall.fontSize};
    font-weight: ${theme.typography.headingSmall.fontWeight};
    color: ${theme.colors.textPrimary};
    margin-bottom: ${theme.dimensions.paddingMedium};
  }
  
  .results-grid {
    display: grid;
    grid-template-columns: repeat(auto-fit, minmax(150px, 1fr));
    gap: ${theme.dimensions.paddingMedium};
    
    .result-item {
      text-align: center;
      padding: ${theme.dimensions.paddingMedium};
      background: ${theme.colors.backgroundLight};
      border-radius: ${theme.dimensions.radiusMedium};
      
      .value {
        font-size: ${theme.typography.headingMedium.fontSize};
        font-weight: ${theme.typography.headingMedium.fontWeight};
        color: ${theme.colors.primaryPurple};
        margin-bottom: ${theme.dimensions.paddingXSmall};
      }
      
      .label {
        font-size: ${theme.typography.bodySmall.fontSize};
        color: ${theme.colors.textSecondary};
      }
    }
  }
  
  .view-all {
    margin-top: ${theme.dimensions.paddingLarge};
    text-align: center;
  }
`;

export const HomeScreen: React.FC = () => {
  const navigate = useNavigate();
  const [assessments, setAssessments] = useState<Assessment[]>([]);
  const [latestResults, setLatestResults] = useState<Assessment | null>(null);
  const [profiles, setProfiles] = useState<UserProfile[]>([]);
  const [profileProgress, setProfileProgress] = useState<Map<string, ScanProgress>>(new Map());
  const [isGeneratingPDF, setIsGeneratingPDF] = useState(false);
  const [pdfGenerationMessage, setPdfGenerationMessage] = useState('');
  const [selectedProfileId, setSelectedProfileId] = useState<string | null>(null);
  const [profileHealthAssessments, setProfileHealthAssessments] = useState<Map<string, any>>(new Map());
  
  useEffect(() => {
    // Load assessments from localStorage or API
    const loadAssessments = () => {
      const storedAssessments = localStorage.getItem('assessments');
      if (storedAssessments) {
        const parsed = JSON.parse(storedAssessments);
        setAssessments(parsed);
        
        // Find latest completed assessment
        const completed = parsed.filter((a: Assessment) => a.status === 'completed');
        if (completed.length > 0) {
          setLatestResults(completed[completed.length - 1]);
        }
      } else {
        // No demo data for new users
        setAssessments([]);
      }
    };
    
    // Load profiles using user data helpers
    const loadProfiles = () => {
      const userEmail = localStorage.getItem('userEmail') || 'user@example.com';
      
      // Migrate legacy scan data if needed
      migrateLegacyScanData(userEmail);
      
      const userProfiles = getAllUserProfiles(userEmail);
      setProfiles(userProfiles);
      
      // Load scan progress for each profile
      const progressMap = new Map<string, ScanProgress>();
      userProfiles.forEach(profile => {
        const progress = getProfileScanProgress(profile.id, profile.name);
        progressMap.set(profile.id, progress);
      });
      setProfileProgress(progressMap);
    };
    
    loadAssessments();
    loadProfiles();
    
    // Load selected profile ID
    const selectedProfile = localStorage.getItem('selectedProfile');
    if (selectedProfile) {
      try {
        const profile = JSON.parse(selectedProfile);
        setSelectedProfileId(profile.id);
      } catch (e) {
        console.error('Error parsing selected profile:', e);
      }
    }
  }, []);
  
  // Load health assessments when profiles change
  useEffect(() => {
    const loadHealthAssessments = () => {
      const completedAssessments = JSON.parse(localStorage.getItem('completedAssessments') || '[]');
      const assessmentsMap = new Map<string, any>();
      
      profiles.forEach(profile => {
        // Find the most recent face scan with Lambda health data for this profile
        const profileAssessments = completedAssessments.filter((a: any) => 
          a.profileName === profile.name && 
          a.type === 'face' && 
          a.status === 'completed' &&
          (a.results?.healthScore !== undefined || 
           a.results?.totalCholesterol !== undefined ||
           a.results?.fitnessScore !== undefined)
        );
        
        if (profileAssessments.length > 0) {
          // Sort by date and get the most recent
          profileAssessments.sort((a: any, b: any) => 
            new Date(b.date).getTime() - new Date(a.date).getTime()
          );
          assessmentsMap.set(profile.id, profileAssessments[0]);
        }
      });
      
      setProfileHealthAssessments(assessmentsMap);
    };
    
    if (profiles.length > 0) {
      loadHealthAssessments();
    }
  }, [profiles]);
  
  const handleResumeAssessment = (assessment: Assessment) => {
    // Make sure we have a selected profile
    const selectedProfile = localStorage.getItem('selectedProfile');
    if (!selectedProfile) {
      alert('Please select a profile before resuming the assessment');
      return;
    }
    
    const profile = JSON.parse(selectedProfile);
    const userProfile = {
      age: profile.age,
      gender: profile.gender,
      height: profile.height,
      weight: profile.weight
    };
    
    // Navigate to appropriate screen based on assessment type and progress
    if (assessment.type === 'body') {
      navigate('/scan/body/preparation', { state: { userProfile } });
    } else {
      navigate('/scan/face/preparation', { state: { userProfile } });
    }
  };
  
  const handleViewReports = () => {
    navigate('/reports');
  };
  
  const handleSelectProfile = (profile: UserProfile) => {
    // Store selected profile and update selection state
    localStorage.setItem('selectedProfile', JSON.stringify(profile));
    localStorage.setItem('selectedProfileId', profile.id);
    setSelectedProfileId(profile.id);
  };
  
  const handleAddProfile = () => {
    // Navigate to profile creation
    navigate('/scan/user-profile', { state: { isNewProfile: true } });
  };
  
  const handleDeleteProfile = (profile: UserProfile) => {
    const confirmDelete = window.confirm(`Are you sure you want to delete the profile "${profile.name}"? This will also delete all associated scan data.`);
    
    if (confirmDelete) {
      const userEmail = localStorage.getItem('userEmail') || 'user@example.com';
      const success = deleteUserProfile(userEmail, profile.id);
      
      if (success) {
        // Reload profiles
        const updatedProfiles = getAllUserProfiles(userEmail);
        setProfiles(updatedProfiles);
        
        // Update progress map
        const progressMap = new Map<string, ScanProgress>();
        updatedProfiles.forEach(p => {
          const progress = getProfileScanProgress(p.id, p.name);
          progressMap.set(p.id, progress);
        });
        setProfileProgress(progressMap);
        
        // If deleted profile was selected, clear selection
        if (selectedProfileId === profile.id) {
          setSelectedProfileId(null);
        }
      } else {
        alert('Failed to delete profile. Please try again.');
      }
    }
  };
  
  const renderProfiles = () => {
    return (
      <>
        <SectionTitle>Profiles</SectionTitle>
        {profiles.length === 0 ? (
          <EmptyStateMessage>
            No profiles yet. Add a profile to start tracking health metrics.
          </EmptyStateMessage>
        ) : null}
        <ProfilesContainer>
          <ProfilesGrid>
            {profiles.map(profile => (
              <ProfileCard 
                key={profile.id} 
                onClick={() => handleSelectProfile(profile)}
                $isSelected={selectedProfileId === profile.id}
              >
              <div className="profile-avatar">
                {profile.name.charAt(0).toUpperCase()}
              </div>
              <div className="profile-info">
                <h4>{profile.name}</h4>
                <p>
                  {profile.age ? `${profile.age} years • ` : ''}
                  {profile.gender}
                  {profile.isMainProfile ? ' • Main Profile' : ''}
                </p>
              </div>
              <IconColumn>
                <IconButton
                  onClick={(e) => {
                    e.stopPropagation();
                    navigate('/scan/user-profile', { 
                      state: { 
                        editingProfile: profile,
                        isMainProfile: profile.isMainProfile 
                      } 
                    });
                  }}
                  title="Edit Profile"
                >
                  <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2">
                    <path d="M11 4H4a2 2 0 00-2 2v14a2 2 0 002 2h14a2 2 0 002-2v-7M18.5 2.5a2.121 2.121 0 013 3L12 15l-4 1 1-4 9.5-9.5z" />
                  </svg>
                </IconButton>
                <IconButton
                  onClick={profileHealthAssessments.has(profile.id) ? async (e) => {
                    e.stopPropagation();
                    await handleDownloadHealthAssessment(profile);
                  } : undefined}
                  title={profileHealthAssessments.has(profile.id) 
                    ? "Download Health Assessment" 
                    : "Please do a scan for this profile to get a report"}
                  style={{
                    opacity: profileHealthAssessments.has(profile.id) ? 1 : 0.4,
                    cursor: profileHealthAssessments.has(profile.id) ? 'pointer' : 'not-allowed',
                    pointerEvents: profileHealthAssessments.has(profile.id) ? 'auto' : 'none'
                  }}
                >
                  <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2">
                    <path d="M21 15v4a2 2 0 01-2 2H5a2 2 0 01-2-2v-4M7 10l5 5 5-5M12 15V3" />
                  </svg>
                </IconButton>
                <IconButton
                  onClick={(e) => {
                    e.stopPropagation();
                    console.log('Share profile:', profile.id);
                  }}
                  title="Share Profile"
                >
                  <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2">
                    <circle cx="18" cy="5" r="3" />
                    <circle cx="6" cy="12" r="3" />
                    <circle cx="18" cy="19" r="3" />
                    <path d="M8.59 13.51l6.83 3.98M15.41 6.51l-6.82 3.98" />
                  </svg>
                </IconButton>
                {!profile.isMainProfile && (
                  <IconButton
                    onClick={(e) => {
                      e.stopPropagation();
                      handleDeleteProfile(profile);
                    }}
                    title="Delete Profile"
                    style={{ color: theme.colors.error }}
                  >
                    <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2">
                      <path d="M3 6h18M8 6V4a2 2 0 012-2h4a2 2 0 012 2v2m3 0v14a2 2 0 01-2 2H7a2 2 0 01-2-2V6h14zM10 11v6M14 11v6" />
                    </svg>
                  </IconButton>
                )}
              </IconColumn>
            </ProfileCard>
          ))}
          <AddProfileCard onClick={handleAddProfile}>
            <div className="add-icon">
              <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2">
                <path d="M12 5v14M5 12h14" />
              </svg>
            </div>
            <span>Add Profile</span>
          </AddProfileCard>
          </ProfilesGrid>
        </ProfilesContainer>
      </>
    );
  };
  
  const handleDownloadHealthAssessment = async (profile: UserProfile) => {
    try {
      const assessment = profileHealthAssessments.get(profile.id);
      if (!assessment) {
        alert('No health assessment found for this profile.');
        return;
      }

      setIsGeneratingPDF(true);
      setPdfGenerationMessage('Generating health assessment PDF...');

      // Use the same PDF generation method as ReportsScreen
      await pdfService.generatePDFFromAssessment(assessment);
      
      setPdfGenerationMessage('PDF generated successfully!');
      setTimeout(() => {
        setIsGeneratingPDF(false);
        setPdfGenerationMessage('');
      }, 2000);
    } catch (error) {
      console.error('Error generating health assessment PDF:', error);
      setPdfGenerationMessage('Error generating PDF. Please try again.');
      setTimeout(() => {
        setIsGeneratingPDF(false);
        setPdfGenerationMessage('');
      }, 3000);
    }
  };

  const handleStartScan = (profileId: string, scanType: 'body' | 'face') => {
    // Set the selected profile
    const profile = profiles.find(p => p.id === profileId);
    if (!profile) {
      alert('Profile not found. Please try again.');
      return;
    }
    
    handleSelectProfile(profile);
    
    // Create user profile data for navigation
    const userProfile = {
      age: profile.age,
      gender: profile.gender,
      height: profile.height,
      weight: profile.weight
    };
    
    // Navigate to appropriate scan with profile data
    if (scanType === 'body') {
      navigate('/scan/body/preparation', { state: { userProfile } });
    } else {
      navigate('/scan/face/preparation', { state: { userProfile } });
    }
  };

  const renderScanProgress = () => {
    const profilesWithProgress = profiles.filter(profile => {
      const progress = profileProgress.get(profile.id);
      return progress && (progress.completedScans > 0 || getInProgressScans(profile.id).length > 0);
    });

    if (profilesWithProgress.length === 0) {
      return null;
    }

    return (
      <>
        <SectionTitle>Scan Progress</SectionTitle>
        <AssessmentGrid>
          {profilesWithProgress.map(profile => {
            const progress = profileProgress.get(profile.id);
            if (!progress) return null;
            
            return (
              <ScanProgressCard
                key={profile.id}
                scanProgress={progress}
                onStartScan={(scanType) => handleStartScan(profile.id, scanType)}
              />
            );
          })}
        </AssessmentGrid>
      </>
    );
  };
  
  
  const renderLatestResults = () => {
    if (!latestResults) return null;
    
    return (
      <LatestResultsSection>
        <h3>Latest Assessment Results</h3>
        <div className="results-grid">
          {latestResults.type === 'body' ? (
            <>
              <div className="result-item">
                <div className="value">{latestResults.results?.bodyFat || 15.2}%</div>
                <div className="label">Body Fat</div>
              </div>
              <div className="result-item">
                <div className="value">{latestResults.results?.muscleMass || 32.1}kg</div>
                <div className="label">Muscle Mass</div>
              </div>
            </>
          ) : (
            <>
              <div className="result-item">
                <div className="value">{latestResults.results?.heartRate || 72}</div>
                <div className="label">Heart Rate</div>
              </div>
              <div className="result-item">
                <div className="value">{latestResults.results?.stressLevel || 'Low'}</div>
                <div className="label">Stress Level</div>
              </div>
            </>
          )}
        </div>
        <div className="view-all">
          <OutlinedButton onClick={handleViewReports}>
            View All Reports
          </OutlinedButton>
        </div>
      </LatestResultsSection>
    );
  };
  
  return (
    <Layout>
      <PageHeader 
        title="Health Assessment"
        subtitle="Track your body metrics and vital signs"
      />
      <PageContent>
        {renderProfiles()}
        {renderScanProgress()}
        {renderLatestResults()}
        {isGeneratingPDF && (
          <LoadingModal 
            isOpen={isGeneratingPDF}
            text={pdfGenerationMessage}
          />
        )}
      </PageContent>
    </Layout>
  );
};