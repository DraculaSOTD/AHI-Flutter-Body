import React, { useState, useEffect } from 'react';
import styled from 'styled-components';
import { useNavigate, useLocation } from 'react-router-dom';
import { theme } from '../../styles/theme';
import { Layout, PageHeader, PageContent } from '../../components/layout/Layout';
import { Card } from '../../components/common/Card';
import { PrimaryGradientButton, SecondaryButton } from '../../components/common/Button';
import { FaceScanResult } from '../../services/rppg/RPPGTypes';
import { SaveReportModal } from '../../components/SaveReportModal';
import { saveScanResult } from '../../utils/scanDataHelpers';

const ResultsContainer = styled.div`
  max-width: 1200px;
  margin: 0 auto;
`;

const MetricsGrid = styled.div`
  display: grid;
  grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
  gap: ${theme.dimensions.paddingLarge};
  margin-bottom: ${theme.dimensions.paddingXLarge};
`;

const MetricCard = styled(Card)<{ $highlight?: boolean }>`
  text-align: center;
  border: ${props => props.$highlight ? `2px solid ${theme.colors.primaryBlue}` : 'none'};
`;

const MetricIcon = styled.div<{ color: string }>`
  width: 64px;
  height: 64px;
  margin: 0 auto ${theme.dimensions.paddingMedium};
  border-radius: 50%;
  background: ${props => props.color};
  display: flex;
  align-items: center;
  justify-content: center;
  
  svg {
    width: 32px;
    height: 32px;
    color: ${theme.colors.textWhite};
  }
`;

const MetricValue = styled.h2`
  font-size: ${theme.typography.headingLarge.fontSize};
  font-weight: ${theme.typography.headingLarge.fontWeight};
  color: ${theme.colors.textPrimary};
  margin: 0 0 ${theme.dimensions.paddingSmall};
  
  span {
    font-size: ${theme.typography.bodyLarge.fontSize};
    font-weight: 400;
    color: ${theme.colors.textSecondary};
  }
`;

const MetricLabel = styled.p`
  font-size: ${theme.typography.bodyMedium.fontSize};
  color: ${theme.colors.textSecondary};
  margin: 0 0 ${theme.dimensions.paddingSmall};
`;

const MetricStatus = styled.p<{ $status: 'normal' | 'warning' | 'critical' }>`
  font-size: ${theme.typography.bodySmall.fontSize};
  font-weight: 600;
  margin: 0;
  color: ${props => {
    switch (props.$status) {
      case 'critical': return theme.colors.error;
      case 'warning': return theme.colors.warning;
      default: return theme.colors.success;
    }
  }};
`;

const InsightsSection = styled.div`
  margin-bottom: ${theme.dimensions.paddingXLarge};
`;

const InsightCard = styled(Card)`
  background: linear-gradient(135deg, rgba(106, 0, 255, 0.1), rgba(0, 178, 255, 0.1));
  border: 1px solid ${theme.colors.divider};
  margin-bottom: ${theme.dimensions.paddingMedium};
`;

const InsightTitle = styled.h3`
  font-size: ${theme.typography.headingSmall.fontSize};
  font-weight: ${theme.typography.headingSmall.fontWeight};
  color: ${theme.colors.textPrimary};
  margin: 0 0 ${theme.dimensions.paddingMedium};
  display: flex;
  align-items: center;
  gap: ${theme.dimensions.paddingSmall};
  
  svg {
    width: 24px;
    height: 24px;
    color: ${theme.colors.primaryBlue};
  }
`;

const InsightContent = styled.p`
  font-size: ${theme.typography.bodyMedium.fontSize};
  line-height: ${theme.typography.bodyMedium.lineHeight};
  color: ${theme.colors.textSecondary};
  margin: 0;
`;

const HealthScoreCard = styled(Card)`
  background: linear-gradient(135deg, rgba(106, 0, 255, 0.15), rgba(0, 178, 255, 0.15));
  border: 2px solid ${theme.colors.primaryBlue};
  text-align: center;
  margin-bottom: ${theme.dimensions.paddingXLarge};
`;

const HealthScoreValue = styled.h1`
  font-size: 4rem;
  font-weight: 700;
  margin: ${theme.dimensions.paddingMedium} 0;
  background: linear-gradient(135deg, ${theme.colors.primaryPurple}, ${theme.colors.primaryBlue});
  -webkit-background-clip: text;
  -webkit-text-fill-color: transparent;
`;

const RiskBadge = styled.span<{ $risk: 'low' | 'moderate' | 'high' }>`
  display: inline-block;
  padding: 4px 12px;
  border-radius: 12px;
  font-size: ${theme.typography.bodySmall.fontSize};
  font-weight: 600;
  margin-left: ${theme.dimensions.paddingSmall};
  background: ${props => {
    switch (props.$risk) {
      case 'high': return theme.colors.error;
      case 'moderate': return theme.colors.warning;
      default: return theme.colors.success;
    }
  }};
  color: ${theme.colors.textWhite};
`;

const RecommendationsList = styled.ul`
  margin: 0;
  padding: 0 0 0 ${theme.dimensions.paddingLarge};
  
  li {
    margin-bottom: ${theme.dimensions.paddingSmall};
    color: ${theme.colors.textSecondary};
    line-height: 1.6;
  }
`;

const ActionButtons = styled.div`
  display: flex;
  gap: ${theme.dimensions.paddingMedium};
  justify-content: center;
  flex-wrap: wrap;
`;

const Disclaimer = styled.div`
  background: rgba(255, 149, 0, 0.1);
  border: 1px solid ${theme.colors.warning};
  border-radius: ${theme.dimensions.radiusMedium};
  padding: ${theme.dimensions.paddingLarge};
  margin-bottom: ${theme.dimensions.paddingXLarge};
  text-align: center;
  
  p {
    margin: 0;
    font-size: ${theme.typography.bodySmall.fontSize};
    color: ${theme.colors.textSecondary};
  }
`;

interface LocationState {
  scanResults: FaceScanResult;
  userData: any;
  measurementCount: number;
  scanId?: string;
  profileId?: string;
}

export const FaceScanResultsScreen: React.FC = () => {
  const navigate = useNavigate();
  const location = useLocation();
  const { scanResults, userData, measurementCount, scanId, profileId } = location.state as LocationState || {};
  const [saveModalOpen, setSaveModalOpen] = useState(false);
  
  // Get profile name for report saving
  const selectedProfile = localStorage.getItem('selectedProfile');
  const parsedProfile = selectedProfile ? JSON.parse(selectedProfile) : null;
  const currentProfileName = parsedProfile?.name || 'User';
  
  // Validate scan results have actual data
  useEffect(() => {
    if (!scanResults) {
      navigate('/home');
      return;
    }
    
    const hasValidData = scanResults.heartRate > 0 && 
                        scanResults.systolicBP > 0 && 
                        scanResults.diastolicBP > 0 && 
                        scanResults.respiratoryRate > 0 && 
                        // Oxygen saturation is optional as some scans don't capture it
                        (scanResults.oxygenSaturation >= 0) &&
                        scanResults.stressLevel && 
                        ['low', 'normal', 'moderate', 'high'].includes(scanResults.stressLevel);
                        
    if (!hasValidData) {
      console.error('Invalid scan results detected, redirecting to home');
      navigate('/home', { 
        state: { 
          error: 'Scan results were incomplete. Please try again with better lighting and positioning.' 
        } 
      });
    }
  }, [scanResults, navigate]);
  
  if (!scanResults) {
    return null;
  }
  
  const getHeartRateStatus = (hr?: number): 'normal' | 'warning' | 'critical' => {
    if (!hr) return 'normal';
    if (hr < 50 || hr > 100) return 'warning';
    if (hr < 40 || hr > 120) return 'critical';
    return 'normal';
  };
  
  const getBloodPressureStatus = (systolic?: number, diastolic?: number): 'normal' | 'warning' | 'critical' => {
    if (!systolic || !diastolic) return 'normal';
    if (systolic > 140 || diastolic > 90) return 'critical';
    if (systolic > 130 || diastolic > 80) return 'warning';
    return 'normal';
  };
  
  
  const handleSaveResults = (reportName: string) => {
    // Get profile ID and name - either from navigation state or from localStorage
    const selectedProfile = localStorage.getItem('selectedProfile');
    const parsedProfile = selectedProfile ? JSON.parse(selectedProfile) : null;
    const currentProfileId = profileId || parsedProfile?.id;
    const currentProfileName = parsedProfile?.name || 'User';
    
    if (!currentProfileId) {
      console.error('No profile ID found for saving scan results');
      return;
    }
    
    // Update the scan to completed status with results
    const completedScan = {
      id: scanId || `face_${Date.now()}`,
      profileId: currentProfileId,
      scanType: 'face' as const,
      status: 'completed' as const,
      progress: 100,
      startedAt: new Date().toISOString(),
      completedAt: new Date().toISOString(),
      name: reportName,
      results: {
        ...scanResults,
        // Ensure we save both the combined string and separate values
        bloodPressure: scanResults.systolicBP && scanResults.diastolicBP 
          ? `${scanResults.systolicBP}/${scanResults.diastolicBP}`
          : undefined,
        systolicBP: scanResults.systolicBP,
        diastolicBP: scanResults.diastolicBP
      }
    };
    
    // Save using the profile-based scan data helper
    saveScanResult(currentProfileId, completedScan);
    
    // Also save to completedAssessments for backward compatibility
    const assessment = {
      id: completedScan.id,
      type: 'face',
      date: new Date().toISOString(),
      name: reportName,
      results: completedScan.results,
      userData,
      status: 'completed',
      profileId: currentProfileId
    };
    
    const completedAssessments = JSON.parse(localStorage.getItem('completedAssessments') || '[]');
    completedAssessments.unshift(assessment);
    localStorage.setItem('completedAssessments', JSON.stringify(completedAssessments));
    
    setSaveModalOpen(false);
    navigate('/reports');
  };
  
  return (
    <Layout>
      <PageHeader
        title="Face Scan Results"
        subtitle="Your vital signs analysis is complete"
        showBackButton
        onBack={() => navigate('/home')}
      />
      
      <PageContent>
        <ResultsContainer>
          {/* Lambda Error Message - Show if Lambda failed */}
          {(scanResults as any).lambdaError && (
            <Disclaimer style={{ background: 'rgba(255, 0, 0, 0.1)', borderColor: theme.colors.error }}>
              <p style={{ color: theme.colors.error, fontWeight: 'bold' }}>
                ⚠️ Advanced health assessment unavailable
              </p>
              <p style={{ fontSize: '0.9rem', marginTop: '8px' }}>
                The detailed health analysis could not be completed. Basic vital signs are shown below.
              </p>
              <details style={{ marginTop: '8px', fontSize: '0.8rem' }}>
                <summary style={{ cursor: 'pointer' }}>Technical details</summary>
                <p style={{ marginTop: '8px', fontFamily: 'monospace' }}>
                  {(scanResults as any).lambdaError}
                </p>
              </details>
            </Disclaimer>
          )}
          
          {/* Health Score - Only show if Lambda data is available */}
          {scanResults.healthScore && (
            <HealthScoreCard>
              <MetricLabel>Overall Health Score</MetricLabel>
              <HealthScoreValue>{Math.round(scanResults.healthScore)}</HealthScoreValue>
              <MetricStatus $status={scanResults.healthScore >= 80 ? 'normal' : scanResults.healthScore >= 60 ? 'warning' : 'critical'}>
                {scanResults.healthScore >= 80 ? 'Excellent' : scanResults.healthScore >= 60 ? 'Good' : 'Needs Attention'}
              </MetricStatus>
            </HealthScoreCard>
          )}
          
          <MetricsGrid>
            {/* Heart Rate */}
            <MetricCard $highlight>
              <MetricIcon color={theme.colors.error}>
                <svg viewBox="0 0 24 24" fill="currentColor">
                  <path d="M12 21.35l-1.45-1.32C5.4 15.36 2 12.28 2 8.5 2 5.42 4.42 3 7.5 3c1.74 0 3.41.81 4.5 2.09C13.09 3.81 14.76 3 16.5 3 19.58 3 22 5.42 22 8.5c0 3.78-3.4 6.86-8.55 11.54L12 21.35z" />
                </svg>
              </MetricIcon>
              <MetricValue>
                {scanResults.heartRate ? Math.round(scanResults.heartRate) : '--'} <span>bpm</span>
              </MetricValue>
              <MetricLabel>Heart Rate</MetricLabel>
              <MetricStatus $status={getHeartRateStatus(scanResults.heartRate)}>
                {getHeartRateStatus(scanResults.heartRate) === 'normal' ? 'Normal Range' : 'Check with doctor'}
              </MetricStatus>
            </MetricCard>
            
            {/* HRV */}
            <MetricCard>
              <MetricIcon color={theme.colors.primaryPurple}>
                <svg viewBox="0 0 24 24" fill="currentColor">
                  <path d="M22,12h-4l-3,9L9,3l-3,9H2" />
                </svg>
              </MetricIcon>
              <MetricValue>
                {scanResults.heartRateVariability ? Math.round(scanResults.heartRateVariability) : '--'} <span>ms</span>
              </MetricValue>
              <MetricLabel>Heart Rate Variability</MetricLabel>
              <MetricStatus $status="normal">Good Balance</MetricStatus>
            </MetricCard>
            
            {/* Blood Pressure */}
            <MetricCard>
              <MetricIcon color={theme.colors.primaryBlue}>
                <svg viewBox="0 0 24 24" fill="currentColor">
                  <path d="M12,2A10,10 0 0,0 2,12A10,10 0 0,0 12,22A10,10 0 0,0 22,12A10,10 0 0,0 12,2M12,4A8,8 0 0,1 20,12A8,8 0 0,1 12,20A8,8 0 0,1 4,12A8,8 0 0,1 12,4M11,7V13L15.25,15.15L16,13.75L12.5,12V7H11Z" />
                </svg>
              </MetricIcon>
              <MetricValue>
                {scanResults.systolicBP && scanResults.diastolicBP 
                  ? `${scanResults.systolicBP}/${scanResults.diastolicBP}` 
                  : '--'} <span>mmHg</span>
              </MetricValue>
              <MetricLabel>Blood Pressure</MetricLabel>
              <MetricStatus $status={getBloodPressureStatus(scanResults.systolicBP, scanResults.diastolicBP)}>
                {scanResults.systolicBP && scanResults.diastolicBP 
                  ? (getBloodPressureStatus(scanResults.systolicBP, scanResults.diastolicBP) === 'normal' ? 'Normal' : 'Elevated')
                  : 'No Data'}
              </MetricStatus>
            </MetricCard>
            
            {/* Respiratory Rate */}
            <MetricCard>
              <MetricIcon color={theme.colors.success}>
                <svg viewBox="0 0 24 24" fill="currentColor">
                  <path d="M3,12V6.75L9,5.43V11.91L3,12M20,3V11.75L10,11.9V5.21L20,3M3,13L9,13.09V19.9L3,18V13M20,13.25V22L10,20.09V13.1L20,13.25Z" />
                </svg>
              </MetricIcon>
              <MetricValue>
                {scanResults.respiratoryRate || '--'} <span>brpm</span>
              </MetricValue>
              <MetricLabel>Respiratory Rate</MetricLabel>
              <MetricStatus $status="normal">Normal Range</MetricStatus>
            </MetricCard>
            
            {/* Oxygen Saturation */}
            <MetricCard>
              <MetricIcon color={theme.colors.primaryBlue}>
                <svg viewBox="0 0 24 24" fill="currentColor">
                  <path d="M12,2A3,3 0 0,1 15,5V11A3,3 0 0,1 12,14A3,3 0 0,1 9,11V5A3,3 0 0,1 12,2M19,11C19,14.53 16.39,17.44 13,17.93V21H11V17.93C7.61,17.44 5,14.53 5,11H7A5,5 0 0,0 12,16A5,5 0 0,0 17,11H19Z" />
                </svg>
              </MetricIcon>
              <MetricValue>
                {scanResults.oxygenSaturation || '--'} <span>%</span>
              </MetricValue>
              <MetricLabel>Oxygen Saturation</MetricLabel>
              <MetricStatus $status="normal">Normal Range</MetricStatus>
            </MetricCard>
            
            {/* Stress Level */}
            <MetricCard>
              <MetricIcon color={theme.colors.warning}>
                <svg viewBox="0 0 24 24" fill="currentColor">
                  <path d="M12,2A10,10 0 0,0 2,12A10,10 0 0,0 12,22A10,10 0 0,0 22,12A10,10 0 0,0 12,2M12,6.5A1.5,1.5 0 0,1 13.5,8A1.5,1.5 0 0,1 12,9.5A1.5,1.5 0 0,1 10.5,8A1.5,1.5 0 0,1 12,6.5M12,17C9.67,17 7.69,15.46 6.89,13.5H17.11C16.3,15.46 14.33,17 12,17Z" />
                </svg>
              </MetricIcon>
              <MetricValue style={{ textTransform: 'capitalize' }}>{scanResults.stressLevel}</MetricValue>
              <MetricLabel>Stress Level</MetricLabel>
              <MetricStatus $status={scanResults.stressScore && scanResults.stressScore > 0.7 ? 'warning' : 'normal'}>
                {scanResults.stressScore && scanResults.stressScore > 0.7 ? 'Consider relaxation' : 'Well managed'}
              </MetricStatus>
            </MetricCard>
            
            {/* Heart Age - Lambda data */}
            {scanResults.heartAge && (
              <MetricCard>
                <MetricIcon color={theme.colors.primaryPurple}>
                  <svg viewBox="0 0 24 24" fill="currentColor">
                    <path d="M12,2C6.48,2 2,6.48 2,12s4.48,10 10,10 10-4.48 10-10S17.52,2 12,2zm0,18c-4.41,0-8-3.59-8-8s3.59-8 8-8 8,3.59 8,8-3.59,8-8,8zm.5-13H11v6l5.25,3.15.75-1.23-4.5-2.67z" />
                  </svg>
                </MetricIcon>
                <MetricValue>
                  {Math.round(scanResults.heartAge)} <span>years</span>
                </MetricValue>
                <MetricLabel>Heart Age</MetricLabel>
                <MetricStatus $status='normal'>
                  {userData?.age && scanResults.heartAge < userData.age ? 'Younger than chronological' : 'Monitor cardiovascular health'}
                </MetricStatus>
              </MetricCard>
            )}
            
            {/* Biological Age - Lambda data */}
            {scanResults.biologicalAge && (
              <MetricCard>
                <MetricIcon color={theme.colors.primaryBlue}>
                  <svg viewBox="0 0 24 24" fill="currentColor">
                    <path d="M12,2A10,10 0 0,0 2,12A10,10 0 0,0 12,22A10,10 0 0,0 22,12A10,10 0 0,0 12,2M16.2,16.2L11,13V7H12.5V12.2L17,14.9L16.2,16.2Z" />
                  </svg>
                </MetricIcon>
                <MetricValue>
                  {Math.round(scanResults.biologicalAge)} <span>years</span>
                </MetricValue>
                <MetricLabel>Biological Age</MetricLabel>
                <MetricStatus $status='normal'>
                  {userData?.age && scanResults.biologicalAge < userData.age ? 'Aging well' : 'Focus on lifestyle improvements'}
                </MetricStatus>
              </MetricCard>
            )}
            
            {/* BMI - Lambda data */}
            {scanResults.bmi && (
              <MetricCard>
                <MetricIcon color={theme.colors.success}>
                  <svg viewBox="0 0 24 24" fill="currentColor">
                    <path d="M12,2A10,10 0 0,0 2,12A10,10 0 0,0 12,22A10,10 0 0,0 22,12A10,10 0 0,0 12,2M9,17H7V10H9V17M13,17H11V7H13V17M17,17H15V13H17V17Z" />
                  </svg>
                </MetricIcon>
                <MetricValue>
                  {scanResults.bmi.toFixed(1)} <span>kg/m²</span>
                </MetricValue>
                <MetricLabel>Body Mass Index</MetricLabel>
                <MetricStatus $status={scanResults.bmi < 18.5 || scanResults.bmi > 30 ? 'warning' : 'normal'}>
                  {scanResults.bmi < 18.5 ? 'Underweight' : scanResults.bmi < 25 ? 'Healthy Range' : scanResults.bmi < 30 ? 'Overweight' : 'Obese'}
                </MetricStatus>
              </MetricCard>
            )}
          </MetricsGrid>
          
          {/* Metabolic Panel - Lambda data */}
          {(scanResults.totalCholesterol || scanResults.ldlCholesterol || scanResults.hdlCholesterol || scanResults.triglycerides) && (
            <>
              <h2 style={{ marginBottom: theme.dimensions.paddingLarge, marginTop: theme.dimensions.paddingXLarge }}>Metabolic Panel</h2>
              <MetricsGrid>
                {scanResults.totalCholesterol && (
                  <MetricCard>
                    <MetricIcon color={theme.colors.primaryPurple}>
                      <svg viewBox="0 0 24 24" fill="currentColor">
                        <path d="M9,20H6V12H9V20M13,20H10V4H13V20M17,20H14V8H17V20Z" />
                      </svg>
                    </MetricIcon>
                    <MetricValue>
                      {scanResults.totalCholesterol.toFixed(2)} <span>mmol/L</span>
                    </MetricValue>
                    <MetricLabel>Total Cholesterol</MetricLabel>
                    <MetricStatus $status={scanResults.cholesterolRisk === 'high' ? 'warning' : 'normal'}>
                      {scanResults.cholesterolRisk === 'high' ? 'Elevated' : 'Normal Range'}
                    </MetricStatus>
                  </MetricCard>
                )}
                
                {scanResults.ldlCholesterol && (
                  <MetricCard>
                    <MetricIcon color={theme.colors.error}>
                      <svg viewBox="0 0 24 24" fill="currentColor">
                        <path d="M13,9H11V7H13M13,17H11V11H13M12,2A10,10 0 0,0 2,12A10,10 0 0,0 12,22A10,10 0 0,0 22,12A10,10 0 0,0 12,2Z" />
                      </svg>
                    </MetricIcon>
                    <MetricValue>
                      {scanResults.ldlCholesterol.toFixed(2)} <span>mmol/L</span>
                    </MetricValue>
                    <MetricLabel>LDL (Bad) Cholesterol</MetricLabel>
                    <MetricStatus $status={scanResults.ldlRisk === 'high' ? 'warning' : 'normal'}>
                      {scanResults.ldlRisk === 'high' ? 'Elevated' : 'Normal Range'}
                    </MetricStatus>
                  </MetricCard>
                )}
                
                {scanResults.hdlCholesterol && (
                  <MetricCard>
                    <MetricIcon color={theme.colors.success}>
                      <svg viewBox="0 0 24 24" fill="currentColor">
                        <path d="M23,12L20.56,9.22L20.9,5.54L17.29,4.72L15.4,1.54L12,3L8.6,1.54L6.71,4.72L3.1,5.53L3.44,9.21L1,12L3.44,14.78L3.1,18.47L6.71,19.29L8.6,22.47L12,21L15.4,22.46L17.29,19.28L20.9,18.46L20.56,14.78L23,12M10,17L6,13L7.41,11.59L10,14.17L16.59,7.58L18,9L10,17Z" />
                      </svg>
                    </MetricIcon>
                    <MetricValue>
                      {scanResults.hdlCholesterol.toFixed(2)} <span>mmol/L</span>
                    </MetricValue>
                    <MetricLabel>HDL (Good) Cholesterol</MetricLabel>
                    <MetricStatus $status={scanResults.hdlRisk === 'high' ? 'warning' : 'normal'}>
                      {scanResults.hdlRisk === 'high' ? 'Low' : 'Normal Range'}
                    </MetricStatus>
                  </MetricCard>
                )}
                
                {scanResults.triglycerides && (
                  <MetricCard>
                    <MetricIcon color={theme.colors.warning}>
                      <svg viewBox="0 0 24 24" fill="currentColor">
                        <path d="M7.5,5.6L5,7L6.4,4.5L5,2L7.5,3.4L10,2L8.6,4.5L10,7L7.5,5.6M19.5,15.4L22,14L20.6,16.5L22,19L19.5,17.6L17,19L18.4,16.5L17,14L19.5,15.4M22,2L20.6,4.5L22,7L19.5,5.6L17,7L18.4,4.5L17,2L19.5,3.4L22,2M13.34,12.78L15.78,10.34L13.66,8.22L11.22,10.66L13.34,12.78M14.37,7.29L16.71,9.63C17.1,10 17.1,10.65 16.71,11.04L5.04,22.71C4.65,23.1 4,23.1 3.63,22.71L1.29,20.37C0.9,20 0.9,19.35 1.29,18.96L12.96,7.29C13.35,6.9 14,6.9 14.37,7.29Z" />
                      </svg>
                    </MetricIcon>
                    <MetricValue>
                      {scanResults.triglycerides.toFixed(2)} <span>mmol/L</span>
                    </MetricValue>
                    <MetricLabel>Triglycerides</MetricLabel>
                    <MetricStatus $status={scanResults.triglyceridesRisk === 'high' ? 'warning' : 'normal'}>
                      {scanResults.triglyceridesRisk === 'high' ? 'Elevated' : 'Normal Range'}
                    </MetricStatus>
                  </MetricCard>
                )}
              </MetricsGrid>
            </>
          )}
          
          {/* Advanced Cardiovascular Metrics - Lambda data */}
          {(scanResults.fitnessScore || scanResults.arterialStiffness || scanResults.cvdRisk10Year) && (
            <>
              <h2 style={{ marginBottom: theme.dimensions.paddingLarge, marginTop: theme.dimensions.paddingXLarge }}>Advanced Metrics</h2>
              <MetricsGrid>
                {scanResults.fitnessScore && (
                  <MetricCard>
                    <MetricIcon color={theme.colors.primaryBlue}>
                      <svg viewBox="0 0 24 24" fill="currentColor">
                        <path d="M5,2H7V4L3.5,7.5L5,9L7,7L9,9L11,7L13,9L15,7L17,9L18.5,7.5L15,4V2H19V4L22,7V11L20,9L18,11L16,9L14,11L12,9L10,11L8,9L6,11L4,9L2,11V7L5,4V2M7,16V14H17V16L19,17V20H5V17L7,16Z" />
                      </svg>
                    </MetricIcon>
                    <MetricValue>
                      {scanResults.fitnessScore.toFixed(1)} <span>ml/kg/min</span>
                    </MetricValue>
                    <MetricLabel>VO₂ Max (Fitness)</MetricLabel>
                    <MetricStatus $status='normal'>
                      {scanResults.fitnessScore >= 35 ? 'Good Fitness' : scanResults.fitnessScore >= 30 ? 'Average Fitness' : 'Below Average'}
                    </MetricStatus>
                  </MetricCard>
                )}
                
                {scanResults.arterialStiffness && (
                  <MetricCard>
                    <MetricIcon color={theme.colors.primaryPurple}>
                      <svg viewBox="0 0 24 24" fill="currentColor">
                        <path d="M3,13H5.79L10.1,4.79L11.28,13.75L14.5,9.66L17.83,13H21V15H17L14.67,12.67L9.92,18.73L8.94,11.31L7,15H3V13Z" />
                      </svg>
                    </MetricIcon>
                    <MetricValue>
                      {Math.round(scanResults.arterialStiffness)} <span>cm/s</span>
                    </MetricValue>
                    <MetricLabel>Arterial Stiffness</MetricLabel>
                    <MetricStatus $status={scanResults.arterialStiffness > 1800 ? 'warning' : 'normal'}>
                      {scanResults.arterialStiffness > 1800 ? 'Above Normal' : 'Normal Range'}
                    </MetricStatus>
                  </MetricCard>
                )}
                
                {scanResults.cvdRisk10Year !== undefined && (
                  <MetricCard>
                    <MetricIcon color={theme.colors.warning}>
                      <svg viewBox="0 0 24 24" fill="currentColor">
                        <path d="M12 2C6.48 2 2 6.48 2 12s4.48 10 10 10 10-4.48 10-10S17.52 2 12 2zm0 18c-4.41 0-8-3.59-8-8s3.59-8 8-8 8 3.59 8 8-3.59 8-8 8zm1-13h-2v6h2V7zm0 8h-2v2h2v-2z" />
                      </svg>
                    </MetricIcon>
                    <MetricValue>
                      {scanResults.cvdRisk10Year} <span>%</span>
                    </MetricValue>
                    <MetricLabel>10-Year CVD Risk</MetricLabel>
                    <MetricStatus $status={scanResults.cvdRisk10Year > 10 ? 'warning' : 'normal'}>
                      {scanResults.cvdRisk10Year > 10 ? 'Elevated Risk' : 'Low Risk'}
                    </MetricStatus>
                  </MetricCard>
                )}
                
                {scanResults.framinghamScore !== undefined && (
                  <MetricCard>
                    <MetricIcon color={theme.colors.primaryBlue}>
                      <svg viewBox="0 0 24 24" fill="currentColor">
                        <path d="M19,3H5C3.89,3 3,3.89 3,5V19A2,2 0 0,0 5,21H19A2,2 0 0,0 21,19V5C21,3.89 20.1,3 19,3M19,5V19H5V5H19M13.09,8.31H10.91L10.97,9.4C11.4,9.22 11.85,9.13 12.33,9.13C14.06,9.13 15.46,10.58 15.46,12.36C15.46,14.15 14.06,15.6 12.33,15.6C11.7,15.6 11.09,15.38 10.58,14.97L11.44,13.88C11.74,14.09 12.07,14.19 12.42,14.19C13.3,14.19 14.02,13.47 14.02,12.58C14.02,11.71 13.3,11 12.42,11C11.96,11 11.54,11.16 11.21,11.42L10.23,10.78L10,8.31H13.09Z" />
                      </svg>
                    </MetricIcon>
                    <MetricValue>
                      {scanResults.framinghamScore}
                    </MetricValue>
                    <MetricLabel>Framingham Score</MetricLabel>
                    <MetricStatus $status={scanResults.framinghamScore > 10 ? 'warning' : 'normal'}>
                      {scanResults.framinghamScore > 10 ? 'Elevated Risk' : 'Low Risk'}
                    </MetricStatus>
                  </MetricCard>
                )}
              </MetricsGrid>
            </>
          )}
          
          {/* Risk Assessment Section - Lambda data */}
          {(scanResults.cardiovascularRisk || scanResults.diabetesRisk || scanResults.metabolicRisk) && (
            <InsightsSection>
              <h2 style={{ marginBottom: theme.dimensions.paddingLarge }}>Risk Assessment</h2>
              
              <InsightCard>
                <InsightTitle>
                  <svg viewBox="0 0 24 24" fill="currentColor">
                    <path d="M12 21.35l-1.45-1.32C5.4 15.36 2 12.28 2 8.5 2 5.42 4.42 3 7.5 3c1.74 0 3.41.81 4.5 2.09C13.09 3.81 14.76 3 16.5 3 19.58 3 22 5.42 22 8.5c0 3.78-3.4 6.86-8.55 11.54L12 21.35z" />
                  </svg>
                  Cardiovascular Risk
                  {scanResults.cardiovascularRisk && (
                    <RiskBadge $risk={scanResults.cardiovascularRisk === 'high' ? 'high' : scanResults.cardiovascularRisk === 'moderate' ? 'moderate' : 'low'}>
                      {scanResults.cardiovascularRisk}
                    </RiskBadge>
                  )}
                </InsightTitle>
                <InsightContent>
                  Based on your vital signs and health metrics, your cardiovascular system shows {scanResults.cardiovascularRisk || 'normal'} risk indicators.
                </InsightContent>
              </InsightCard>
              
              {scanResults.diabetesRisk && (
                <InsightCard>
                  <InsightTitle>
                    <svg viewBox="0 0 24 24" fill="currentColor">
                      <path d="M12,2C17.52,2 22,6.48 22,12C22,17.52 17.52,22 12,22C6.48,22 2,17.52 2,12C2,6.48 6.48,2 12,2M12,4C7.58,4 4,7.58 4,12C4,16.42 7.58,20 12,20C16.42,20 20,16.42 20,12C20,7.58 16.42,4 12,4M11,7H13V13H11V7M11,15H13V17H11V15Z" />
                    </svg>
                    Diabetes Risk
                    <RiskBadge $risk={scanResults.diabetesRisk === 'high' ? 'high' : scanResults.diabetesRisk === 'moderate' ? 'moderate' : 'low'}>
                      {scanResults.diabetesRisk}
                    </RiskBadge>
                  </InsightTitle>
                  <InsightContent>
                    Your metabolic indicators suggest {scanResults.diabetesRisk} risk for diabetes development.
                  </InsightContent>
                </InsightCard>
              )}
              
              {scanResults.metabolicRisk && (
                <InsightCard>
                  <InsightTitle>
                    <svg viewBox="0 0 24 24" fill="currentColor">
                      <path d="M12,2A10,10 0 0,0 2,12A10,10 0 0,0 12,22A10,10 0 0,0 22,12A10,10 0 0,0 12,2M12,4A8,8 0 0,1 20,12A8,8 0 0,1 12,20A8,8 0 0,1 4,12A8,8 0 0,1 12,4M11,7V13L15.25,15.15L16,13.75L12.5,12V7H11Z" />
                    </svg>
                    Metabolic Health
                    <RiskBadge $risk={scanResults.metabolicRisk === 'high' ? 'high' : scanResults.metabolicRisk === 'moderate' ? 'moderate' : 'low'}>
                      {scanResults.metabolicRisk}
                    </RiskBadge>
                  </InsightTitle>
                  <InsightContent>
                    Your metabolic health markers indicate {scanResults.metabolicRisk} risk levels.
                  </InsightContent>
                </InsightCard>
              )}
            </InsightsSection>
          )}
          
          <InsightsSection>
            <h2 style={{ marginBottom: theme.dimensions.paddingLarge }}>Health Insights</h2>
            
            <InsightCard>
              <InsightTitle>
                <svg viewBox="0 0 24 24" fill="currentColor">
                  <path d="M12 2C6.48 2 2 6.48 2 12s4.48 10 10 10 10-4.48 10-10S17.52 2 12 2zm1 15h-2v-6h2v6zm0-8h-2V7h2v2z" />
                </svg>
                Cardiovascular Health
              </InsightTitle>
              <InsightContent>
                Your heart rate and blood pressure readings are within expected ranges for your age and profile. 
                Regular monitoring can help track trends over time.
              </InsightContent>
            </InsightCard>
            
            <InsightCard>
              <InsightTitle>
                <svg viewBox="0 0 24 24" fill="currentColor">
                  <path d="M12 2C6.48 2 2 6.48 2 12s4.48 10 10 10 10-4.48 10-10S17.52 2 12 2zm1 15h-2v-6h2v6zm0-8h-2V7h2v2z" />
                </svg>
                Stress Management
              </InsightTitle>
              <InsightContent>
                Your stress indicators suggest {scanResults.stressLevel} stress levels. 
                {scanResults.stressScore && scanResults.stressScore > 0.6 ? 
                  'Consider incorporating relaxation techniques or mindfulness exercises into your daily routine.' :
                  'Keep up your current stress management practices.'}
              </InsightContent>
            </InsightCard>
            
            {/* Lambda Recommendations */}
            {scanResults.recommendations && scanResults.recommendations.length > 0 && (
              <InsightCard>
                <InsightTitle>
                  <svg viewBox="0 0 24 24" fill="currentColor">
                    <path d="M9,20.42L2.79,14.21L5.62,11.38L9,14.77L18.88,4.88L21.71,7.71L9,20.42Z" />
                  </svg>
                  Personalized Recommendations
                </InsightTitle>
                <RecommendationsList>
                  {scanResults.recommendations.map((rec, index) => (
                    <li key={index}>{rec}</li>
                  ))}
                </RecommendationsList>
              </InsightCard>
            )}
            
            {/* Lambda Insights */}
            {scanResults.insights && scanResults.insights.length > 0 && (
              <InsightCard>
                <InsightTitle>
                  <svg viewBox="0 0 24 24" fill="currentColor">
                    <path d="M12,2C6.48,2 2,6.48 2,12C2,17.52 6.48,22 12,22C17.52,22 22,17.52 22,12C22,6.48 17.52,2 12,2M13,17H11V15H13M13,13H11V7H13V13Z" />
                  </svg>
                  Additional Insights
                </InsightTitle>
                {scanResults.insights.map((insight, index) => (
                  <InsightContent key={index} style={{ marginBottom: index < scanResults.insights!.length - 1 ? theme.dimensions.paddingSmall : 0 }}>
                    {insight}
                  </InsightContent>
                ))}
              </InsightCard>
            )}
          </InsightsSection>
          
          <Disclaimer>
            <p>
              These results are for informational purposes only and should not be used for medical diagnosis. 
              Always consult with a healthcare professional for medical advice.
            </p>
          </Disclaimer>
          
          <ActionButtons>
            <PrimaryGradientButton size="large" onClick={() => setSaveModalOpen(true)}>
              Save Results
            </PrimaryGradientButton>
            <PrimaryGradientButton size="large" onClick={() => navigate('/scan/face/camera', { state: { userData } })}>
              Scan Again
            </PrimaryGradientButton>
            <PrimaryGradientButton size="large" onClick={() => navigate('/home')}>
              Back to Home
            </PrimaryGradientButton>
          </ActionButtons>
        </ResultsContainer>
      </PageContent>
      
      <SaveReportModal
        open={saveModalOpen}
        onClose={() => setSaveModalOpen(false)}
        onSave={handleSaveResults}
        scanType="face"
        profileName={currentProfileName}
      />
    </Layout>
  );
};