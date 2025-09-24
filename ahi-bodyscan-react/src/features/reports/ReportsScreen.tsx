import React, { useState, useEffect } from 'react';
import styled from 'styled-components';
import { theme } from '../../styles/theme';
import { Layout, PageHeader, PageContent } from '../../components/layout/Layout';
import { Card } from '../../components/common/Card';
import { pdfService } from '../../services/pdf/PDFService';

interface Assessment {
  id: string;
  type: 'body' | 'face';
  date: string;
  status: 'completed';
  name?: string; // Custom report name
  profileName?: string;
  results: {
    // Body scan results
    bodyFat?: number;
    muscleMass?: number;
    bmi?: number;
    waist?: number;
    chest?: number;
    hip?: number;
    // Face scan results
    heartRate?: number;
    heartRateVariability?: number;
    bloodPressure?: string;
    systolicBP?: number;
    diastolicBP?: number;
    stressLevel?: string;
    respiratoryRate?: number;
    oxygenSaturation?: number;
    
    // Lambda Health Assessment fields
    healthScore?: number;
    heartAge?: number;
    biologicalAge?: number;
    arterialStiffness?: number;
    
    // Cholesterol Panel
    totalCholesterol?: number;
    ldlCholesterol?: number;
    hdlCholesterol?: number;
    triglycerides?: number;
    
    // Fitness Metrics
    fitnessScore?: number;
    vo2Max?: number;
    cvdRisk10Year?: number;
    framinghamScore?: number;
    maxHeartRate?: number;
    meanArterialPressure?: number;
    
    // Risk Assessments
    cardiovascularRisk?: string;
    diabetesRisk?: string;
    metabolicRisk?: string;
    cholesterolRisk?: string;
  };
}

const SearchContainer = styled.div`
  margin-bottom: ${theme.dimensions.paddingLarge};
`;

const SearchInputWrapper = styled.div`
  position: relative;
  max-width: 500px;
`;

const SearchInput = styled.input`
  width: 100%;
  padding: ${theme.dimensions.paddingMedium} ${theme.dimensions.paddingLarge};
  padding-left: 48px;
  background: ${theme.colors.backgroundWhite};
  border: 1px solid ${theme.colors.borderLight};
  border-radius: ${theme.dimensions.radiusPill};
  font-size: ${theme.typography.bodyMedium.fontSize};
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

const SearchIcon = styled.div`
  position: absolute;
  left: 16px;
  top: 50%;
  transform: translateY(-50%);
  color: ${theme.colors.textTertiary};
  
  svg {
    width: 20px;
    height: 20px;
  }
`;

const FilterContainer = styled.div`
  display: flex;
  gap: ${theme.dimensions.paddingMedium};
  margin-bottom: ${theme.dimensions.paddingLarge};
  
  @media (max-width: ${theme.breakpoints.mobile}) {
    overflow-x: auto;
    -webkit-overflow-scrolling: touch;
  }
`;

const FilterButton = styled.button<{ $active: boolean }>`
  padding: ${theme.dimensions.paddingSmall} ${theme.dimensions.paddingLarge};
  border-radius: ${theme.dimensions.radiusPill};
  background: ${props => props.$active ? theme.colors.primaryPurple : theme.colors.backgroundWhite};
  color: ${props => props.$active ? theme.colors.textWhite : theme.colors.textSecondary};
  border: 1px solid ${props => props.$active ? theme.colors.primaryPurple : theme.colors.borderLight};
  font-size: ${theme.typography.bodyMedium.fontSize};
  font-weight: 500;
  transition: all ${theme.transitions.fast};
  white-space: nowrap;
  
  &:hover {
    background: ${props => props.$active ? theme.colors.primaryPurple : theme.colors.backgroundLight};
  }
`;

const AssessmentList = styled.div`
  display: flex;
  flex-direction: column;
  gap: ${theme.dimensions.paddingMedium};
`;

const AssessmentCard = styled(Card)`
  cursor: pointer;
  
  &:hover {
    transform: translateY(-2px);
  }
`;

const AssessmentHeader = styled.div`
  display: flex;
  align-items: center;
  justify-content: space-between;
  margin-bottom: ${theme.dimensions.paddingMedium};
`;

const AssessmentInfo = styled.div`
  display: flex;
  align-items: center;
  gap: ${theme.dimensions.paddingMedium};
`;

const AssessmentIcon = styled.div<{ type: 'body' | 'face' }>`
  width: 48px;
  height: 48px;
  border-radius: ${theme.dimensions.radiusMedium};
  background: ${props => props.type === 'body' ? theme.colors.primaryBlue : theme.colors.primaryPurple}20;
  color: ${props => props.type === 'body' ? theme.colors.primaryBlue : theme.colors.primaryPurple};
  display: flex;
  align-items: center;
  justify-content: center;
  font-size: 24px;
`;

const AssessmentDetails = styled.div`
  h3 {
    font-size: ${theme.typography.headingSmall.fontSize};
    font-weight: ${theme.typography.headingSmall.fontWeight};
    color: ${theme.colors.textPrimary};
    margin-bottom: 4px;
  }
  
  p {
    font-size: ${theme.typography.bodyMedium.fontSize};
    color: ${theme.colors.textSecondary};
  }
`;

const AssessmentDate = styled.div`
  display: flex;
  align-items: center;
  gap: ${theme.dimensions.paddingMedium};
  
  .date-time {
    text-align: right;
    
    .date {
      font-size: ${theme.typography.bodyMedium.fontSize};
      font-weight: 500;
      color: ${theme.colors.textPrimary};
    }
    
    .time {
      font-size: ${theme.typography.bodySmall.fontSize};
      color: ${theme.colors.textTertiary};
    }
  }
`;

const ActionIcons = styled.div`
  display: flex;
  gap: ${theme.dimensions.paddingSmall};
`;

const IconButton = styled.button`
  width: 36px;
  height: 36px;
  border-radius: ${theme.dimensions.radiusSmall};
  display: flex;
  align-items: center;
  justify-content: center;
  background: ${theme.colors.backgroundLight};
  color: ${theme.colors.textSecondary};
  transition: all ${theme.transitions.fast};
  border: none;
  cursor: pointer;
  
  &:hover {
    background: ${theme.colors.primaryPurple}20;
    color: ${theme.colors.primaryPurple};
  }
  
  svg {
    width: 20px;
    height: 20px;
  }
`;

const MetricsGrid = styled.div`
  display: grid;
  grid-template-columns: repeat(auto-fit, minmax(150px, 1fr));
  gap: ${theme.dimensions.paddingMedium};
  margin-top: ${theme.dimensions.paddingLarge};
  padding-top: ${theme.dimensions.paddingLarge};
  
  @media (max-width: ${theme.breakpoints.mobile}) {
    grid-template-columns: repeat(auto-fit, minmax(100px, 1fr));
    gap: ${theme.dimensions.paddingSmall};
  }
  border-top: 1px solid ${theme.colors.divider};
`;

const MetricCard = styled.div<{ $isSelected?: boolean; $color?: string }>`
  padding: ${theme.dimensions.paddingLarge};
  background: ${props => props.$isSelected ? theme.colors.backgroundLight : theme.colors.backgroundWhite};
  border: 1px solid ${props => props.$isSelected ? theme.colors.primaryPurple : theme.colors.borderLight};
  border-radius: ${theme.dimensions.radiusMedium};
  text-align: center;
  cursor: pointer;
  transition: all ${theme.transitions.fast};
  
  &:hover {
    transform: translateY(-2px);
    box-shadow: ${theme.shadows.small};
  }
  
  .label {
    font-size: ${theme.typography.bodySmall.fontSize};
    color: ${theme.colors.textSecondary};
    margin-bottom: ${theme.dimensions.paddingSmall};
  }
  
  .value {
    font-size: ${theme.typography.headingLarge.fontSize};
    font-weight: ${theme.typography.headingLarge.fontWeight};
    color: ${props => props.$color || theme.colors.textPrimary};
    line-height: 1.2;
  }
  
  .unit {
    font-size: ${theme.typography.bodySmall.fontSize};
    font-weight: normal;
    color: ${theme.colors.textSecondary};
    margin-top: 4px;
    display: block;
  }
`;

const RangeSection = styled.div<{ $isOpen: boolean }>`
  max-height: ${props => props.$isOpen ? '200px' : '0'};
  overflow: hidden;
  transition: max-height ${theme.transitions.normal};
  margin-top: ${theme.dimensions.paddingLarge};
`;

const RangeBar = styled.div`
  position: relative;
  height: 8px;
  background: linear-gradient(to right, 
    ${theme.colors.error} 0%, 
    ${theme.colors.warning} 30%, 
    ${theme.colors.success} 70%
  );
  border-radius: 4px;
  margin: ${theme.dimensions.paddingLarge} 0;
`;

const RangeIndicator = styled.div<{ $position: number }>`
  position: absolute;
  top: -8px;
  left: ${props => props.$position}%;
  transform: translateX(-50%);
  width: 24px;
  height: 24px;
  background: white;
  border: 3px solid ${props => {
    if (props.$position < 30) return theme.colors.error;
    if (props.$position < 70) return theme.colors.warning;
    return theme.colors.success;
  }};
  border-radius: 50%;
`;

const RangeLabels = styled.div`
  display: flex;
  justify-content: space-between;
  font-size: ${theme.typography.bodySmall.fontSize};
  color: ${theme.colors.textSecondary};
  margin-bottom: ${theme.dimensions.paddingMedium};
`;

const RangeValue = styled.div`
  text-align: center;
  font-size: ${theme.typography.bodyMedium.fontSize};
  color: ${theme.colors.textPrimary};
  font-weight: 500;
`;


const EmptyState = styled.div`
  text-align: center;
  padding: ${theme.dimensions.paddingXLarge} 0;
  
  svg {
    width: 80px;
    height: 80px;
    color: ${theme.colors.textTertiary};
    margin-bottom: ${theme.dimensions.paddingLarge};
  }
  
  h3 {
    font-size: ${theme.typography.headingMedium.fontSize};
    font-weight: ${theme.typography.headingMedium.fontWeight};
    color: ${theme.colors.textPrimary};
    margin-bottom: ${theme.dimensions.paddingSmall};
  }
  
  p {
    font-size: ${theme.typography.bodyLarge.fontSize};
    color: ${theme.colors.textSecondary};
  }
`;

// Metric calculation utilities
const calculateRangePosition = (value: number, metric: string): number => {
  switch (metric) {
    case 'heartRate':
      // 40-100 bpm range, normal 60-80
      if (value < 50) return 15;
      if (value > 100) return 85;
      return ((value - 40) / 60) * 100;
    
    case 'systolicBP':
      // Normal < 120, Elevated 120-129, High > 130
      if (value < 100) return 85;
      if (value < 120) return 70;
      if (value < 130) return 50;
      if (value < 140) return 30;
      return 15;
    
    case 'diastolicBP':
      // Normal < 80, Elevated 80-89, High > 90
      if (value < 70) return 85;
      if (value < 80) return 70;
      if (value < 90) return 50;
      return 20;
    
    case 'oxygenSaturation':
      // Normal > 95%, Low < 95%
      if (value >= 95) return 85;
      if (value >= 90) return 50;
      return 20;
    
    case 'respiratoryRate':
      // Normal 12-20 breaths/min
      if (value < 12) return 20;
      if (value > 20) return 20;
      return 70;
    
    case 'bodyFat':
      // Varies by gender, using general ranges
      if (value < 10) return 20;
      if (value < 20) return 70;
      if (value < 30) return 50;
      return 20;
    
    case 'bmi':
      // < 18.5 underweight, 18.5-24.9 normal, 25-29.9 overweight, > 30 obese
      if (value < 18.5) return 20;
      if (value < 25) return 70;
      if (value < 30) return 40;
      return 20;
    
    case 'muscleMass':
      // Generally higher is better
      return 70; // Default to good range
    
    case 'heartRateVariability':
      // HRV: Higher is generally better, 20-200ms range
      if (value < 20) return 20;
      if (value < 50) return 50;
      if (value < 100) return 70;
      return 85; // High HRV is excellent
    
    default:
      return 50;
  }
};

const getMetricColor = (value: number | undefined, metric: string): string => {
  if (value === undefined) return theme.colors.textSecondary;
  const position = calculateRangePosition(value, metric);
  if (position < 30) return theme.colors.error;
  if (position < 70) return theme.colors.warning;
  return theme.colors.success;
};

export const ReportsScreen: React.FC = () => {
  const [filter, setFilter] = useState<'all' | 'body' | 'face' | 'health'>('all');
  const [assessments, setAssessments] = useState<Assessment[]>([]);
  const [searchQuery, setSearchQuery] = useState('');
  const [selectedMetrics, setSelectedMetrics] = useState<Record<string, string | null>>({});
  
  useEffect(() => {
    // Load assessments from localStorage or API
    const loadAssessments = () => {
      const storedAssessments = localStorage.getItem('completedAssessments');
      if (storedAssessments) {
        setAssessments(JSON.parse(storedAssessments));
      } else {
        // No demo data for new users - show empty state
        setAssessments([]);
      }
    };
    
    loadAssessments();
  }, []);
  
  const formatDate = (dateString: string) => {
    const date = new Date(dateString);
    return {
      date: date.toLocaleDateString('en-US', { month: 'short', day: 'numeric', year: 'numeric' }),
      time: date.toLocaleTimeString('en-US', { hour: 'numeric', minute: '2-digit' })
    };
  };
  
  const filteredAssessments = assessments.filter(assessment => {
    // First apply type filter
    if (filter === 'health') {
      // Health filter shows only face scans with Lambda data
      return assessment.type === 'face' && 
             (assessment.results.healthScore !== undefined || 
              assessment.results.totalCholesterol !== undefined ||
              assessment.results.fitnessScore !== undefined);
    }
    if (filter !== 'all' && assessment.type !== filter) return false;
    
    // Then apply search filter
    if (searchQuery.trim() === '') return true;
    
    const query = searchQuery.toLowerCase();
    const { date, time } = formatDate(assessment.date);
    
    // Search in custom name
    if (assessment.name && assessment.name.toLowerCase().includes(query)) return true;
    
    // Search in profile name
    if (assessment.profileName && assessment.profileName.toLowerCase().includes(query)) return true;
    
    // Search in type
    if (assessment.type.toLowerCase().includes(query)) return true;
    
    // Search in date and time
    if (date.toLowerCase().includes(query) || time.toLowerCase().includes(query)) return true;
    
    // Search in scan type names
    if (assessment.type === 'body' && 'body scan'.includes(query)) return true;
    if (assessment.type === 'face' && 'face scan'.includes(query)) return true;
    
    // Search in results
    if (assessment.results) {
      // Body scan results
      if (assessment.results.bodyFat && assessment.results.bodyFat.toString().includes(query)) return true;
      if (assessment.results.muscleMass && assessment.results.muscleMass.toString().includes(query)) return true;
      if (assessment.results.bmi && assessment.results.bmi.toString().includes(query)) return true;
      if (assessment.results.waist && assessment.results.waist.toString().includes(query)) return true;
      if (assessment.results.chest && assessment.results.chest.toString().includes(query)) return true;
      if (assessment.results.hip && assessment.results.hip.toString().includes(query)) return true;
      
      // Face scan results
      if (assessment.results.heartRate && assessment.results.heartRate.toString().includes(query)) return true;
      if (assessment.results.bloodPressure && assessment.results.bloodPressure.toLowerCase().includes(query)) return true;
      if (assessment.results.stressLevel && assessment.results.stressLevel.toLowerCase().includes(query)) return true;
      if (assessment.results.respiratoryRate && assessment.results.respiratoryRate.toString().includes(query)) return true;
      if (assessment.results.oxygenSaturation && assessment.results.oxygenSaturation.toString().includes(query)) return true;
      if (assessment.results.heartRateVariability && assessment.results.heartRateVariability.toString().includes(query)) return true;
      
      // Search in metric names
      const metricNames = ['body fat', 'muscle mass', 'bmi', 'waist', 'chest', 'hip', 
                          'heart rate', 'blood pressure', 'stress level', 'respiratory rate', 'oxygen saturation', 'hrv', 'heart rate variability'];
      if (metricNames.some(name => name.includes(query))) return true;
    }
    
    return false;
  });
  
  const handleExportPDF = async (assessment: Assessment) => {
    try {
      await pdfService.generatePDFFromAssessment(assessment);
    } catch (error) {
      console.error('Failed to generate PDF:', error);
      alert('Failed to generate PDF. Please try again.');
    }
  };
  
  const handleShare = (assessment: Assessment) => {
    const { date } = formatDate(assessment.date);
    const scanType = assessment.type === 'body' ? 'Body Scan' : 'Face Scan';
    
    // Create share text
    let shareText = `My ${scanType} results from ${date}:\n\n`;
    
    if (assessment.type === 'body') {
      shareText += `Body Fat: ${assessment.results.bodyFat}%\n`;
      shareText += `Muscle Mass: ${assessment.results.muscleMass}kg\n`;
      shareText += `BMI: ${assessment.results.bmi}\n`;
      shareText += `Waist: ${assessment.results.waist}cm`;
    } else {
      shareText += `Heart Rate: ${assessment.results.heartRate} bpm\n`;
      shareText += `Blood Pressure: ${assessment.results.bloodPressure}\n`;
      shareText += `Stress Level: ${assessment.results.stressLevel}\n`;
      shareText += `O₂ Saturation: ${assessment.results.oxygenSaturation}%`;
    }
    
    // Check if Web Share API is available
    if (navigator.share) {
      navigator.share({
        title: `${scanType} Results`,
        text: shareText,
      }).catch(err => console.log('Error sharing:', err));
    } else {
      // Fallback - copy to clipboard
      navigator.clipboard.writeText(shareText).then(() => {
        alert('Results copied to clipboard!');
      }).catch(err => console.error('Failed to copy:', err));
    }
  };
  
  const handleMetricClick = (assessmentId: string, metricName: string) => {
    setSelectedMetrics(prev => ({
      ...prev,
      [assessmentId]: prev[assessmentId] === metricName ? null : metricName
    }));
  };
  
  const renderEmptyState = () => (
    <EmptyState>
      <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="1.5">
        <path d="M9 12h6m-6 4h6m2 5H7a2 2 0 01-2-2V5a2 2 0 012-2h5.586a1 1 0 01.707.293l5.414 5.414a1 1 0 01.293.707V19a2 2 0 01-2 2z" />
      </svg>
      <h3>No Reports Yet</h3>
      <p>Complete your first health assessment to see results here</p>
    </EmptyState>
  );
  
  const getStressLevelDisplay = (stressLevel?: string): { value: string; unit: string } => {
    if (!stressLevel) return { value: '--', unit: '' };
    
    const level = stressLevel.toLowerCase();
    switch (level) {
      case 'low':
        return { value: 'L', unit: 'Low' };
      case 'normal':
        return { value: 'N', unit: 'Normal' };
      case 'moderate':
        return { value: 'M', unit: 'Moderate' };
      case 'high':
        return { value: 'H', unit: 'High' };
      default:
        return { value: stressLevel.charAt(0).toUpperCase(), unit: stressLevel };
    }
  };

  const renderAssessmentCard = (assessment: Assessment) => {
    const { date, time } = formatDate(assessment.date);
    const isBodyScan = assessment.type === 'body';
    const selectedMetric = selectedMetrics[assessment.id] || null;
    
    // Extract systolic and diastolic from blood pressure string if needed
    let systolic = assessment.results.systolicBP;
    let diastolic = assessment.results.diastolicBP;
    
    if (!systolic && !diastolic && assessment.results.bloodPressure) {
      const bpMatch = assessment.results.bloodPressure.match(/(\d+)\/(\d+)/);
      if (bpMatch) {
        systolic = parseInt(bpMatch[1]);
        diastolic = parseInt(bpMatch[2]);
      }
    }
    
    return (
      <AssessmentCard key={assessment.id}>
        <AssessmentHeader>
          <AssessmentInfo>
            <AssessmentIcon type={assessment.type}>
              {isBodyScan ? (
                <svg viewBox="0 0 24 24" fill="currentColor" width="28" height="28">
                  <path d="M12 1C10.89 1 10 1.9 10 3C10 4.11 10.89 5 12 5C13.11 5 14 4.11 14 3C14 1.9 13.11 1 12 1M10 6C9.73 6 9.5 6.11 9.31 6.28H9.3L4 11.59L5.42 13L9 9.41V22H11V15H13V22H15V9.41L18.58 13L20 11.59L14.7 6.28C14.5 6.11 14.27 6 14 6" />
                </svg>
              ) : (
                <svg viewBox="0 0 24 24" fill="currentColor" width="28" height="28">
                  <path d="M9,11.75A1.25,1.25 0 0,0 7.75,13A1.25,1.25 0 0,0 9,14.25A1.25,1.25 0 0,0 10.25,13A1.25,1.25 0 0,0 9,11.75M15,11.75A1.25,1.25 0 0,0 13.75,13A1.25,1.25 0 0,0 15,14.25A1.25,1.25 0 0,0 16.25,13A1.25,1.25 0 0,0 15,11.75M12,2A10,10 0 0,0 2,12A10,10 0 0,0 12,22A10,10 0 0,0 22,12A10,10 0 0,0 12,2M12,20C7.59,20 4,16.41 4,12C4,11.71 4,11.42 4.05,11.14C6.41,10.09 8.28,8.16 9.26,5.77C11.07,8.33 14.05,10 17.42,10C18.2,10 18.95,9.91 19.67,9.74C19.88,10.45 20,11.21 20,12C20,16.41 16.41,20 12,20Z" />
                </svg>
              )}
            </AssessmentIcon>
            <AssessmentDetails>
              <h3>{assessment.name || `${assessment.profileName || 'User'} ${isBodyScan ? 'Body Scan' : 'Face Scan'}`}</h3>
              <p>{isBodyScan ? 'Body Scan' : 'Face Scan'} - Complete assessment</p>
            </AssessmentDetails>
          </AssessmentInfo>
          <AssessmentDate>
            <div className="date-time">
              <div className="date">{date}</div>
              <div className="time">{time}</div>
            </div>
            <ActionIcons>
              <IconButton 
                onClick={(e) => { 
                  e.stopPropagation(); 
                  handleExportPDF(assessment); 
                }}
                title="Download PDF"
              >
                <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2">
                  <path d="M21 15v4a2 2 0 01-2 2H5a2 2 0 01-2-2v-4M7 10l5 5 5-5M12 15V3" strokeLinecap="round" strokeLinejoin="round" />
                </svg>
              </IconButton>
              <IconButton 
                onClick={(e) => { 
                  e.stopPropagation(); 
                  handleShare(assessment); 
                }}
                title="Share"
              >
                <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2">
                  <path d="M8.59 13.51l6.83 3.98m-.01-10.98l-6.82 3.98M21 5a3 3 0 11-6 0 3 3 0 016 0zM9 12a3 3 0 11-6 0 3 3 0 016 0zm12 7a3 3 0 11-6 0 3 3 0 016 0z" strokeLinecap="round" strokeLinejoin="round" />
                </svg>
              </IconButton>
            </ActionIcons>
          </AssessmentDate>
        </AssessmentHeader>
        
        <MetricsGrid>
          {isBodyScan ? (
            <>
              <MetricCard 
                $isSelected={selectedMetric === 'bodyFat'}
                $color={getMetricColor(assessment.results.bodyFat, 'bodyFat')}
                onClick={(e) => { e.stopPropagation(); handleMetricClick(assessment.id, 'bodyFat'); }}
              >
                <div className="label">Body Fat</div>
                <div className="value">{assessment.results.bodyFat?.toFixed(1) || '--'}</div>
                <div className="unit">%</div>
              </MetricCard>

              <MetricCard 
                $isSelected={selectedMetric === 'muscleMass'}
                $color={getMetricColor(assessment.results.muscleMass, 'muscleMass')}
                onClick={(e) => { e.stopPropagation(); handleMetricClick(assessment.id, 'muscleMass'); }}
              >
                <div className="label">Muscle Mass</div>
                <div className="value">{assessment.results.muscleMass?.toFixed(1) || '--'}</div>
                <div className="unit">kg</div>
              </MetricCard>

              <MetricCard 
                $isSelected={selectedMetric === 'bmi'}
                $color={getMetricColor(assessment.results.bmi, 'bmi')}
                onClick={(e) => { e.stopPropagation(); handleMetricClick(assessment.id, 'bmi'); }}
              >
                <div className="label">BMI</div>
                <div className="value">{assessment.results.bmi?.toFixed(1) || '--'}</div>
              </MetricCard>

              <MetricCard 
                $isSelected={selectedMetric === 'waist'}
                $color={theme.colors.primaryBlue}
                onClick={(e) => { e.stopPropagation(); handleMetricClick(assessment.id, 'waist'); }}
              >
                <div className="label">Waist</div>
                <div className="value">{assessment.results.waist?.toFixed(1) || '--'}</div>
                <div className="unit">cm</div>
              </MetricCard>

              <MetricCard 
                $isSelected={selectedMetric === 'chest'}
                $color={theme.colors.primaryBlue}
                onClick={(e) => { e.stopPropagation(); handleMetricClick(assessment.id, 'chest'); }}
              >
                <div className="label">Chest</div>
                <div className="value">{assessment.results.chest?.toFixed(1) || '--'}</div>
                <div className="unit">cm</div>
              </MetricCard>

              <MetricCard 
                $isSelected={selectedMetric === 'hip'}
                $color={theme.colors.primaryBlue}
                onClick={(e) => { e.stopPropagation(); handleMetricClick(assessment.id, 'hip'); }}
              >
                <div className="label">Hip</div>
                <div className="value">{assessment.results.hip?.toFixed(1) || '--'}</div>
                <div className="unit">cm</div>
              </MetricCard>
            </>
          ) : (
            <>
              <MetricCard 
                $isSelected={selectedMetric === 'heartRate'}
                $color={getMetricColor(assessment.results.heartRate, 'heartRate')}
                onClick={(e) => { e.stopPropagation(); handleMetricClick(assessment.id, 'heartRate'); }}
              >
                <div className="label">Heart Rate</div>
                <div className="value">{assessment.results.heartRate || '--'}</div>
                <div className="unit">bpm</div>
              </MetricCard>

              <MetricCard 
                $isSelected={selectedMetric === 'systolicBP'}
                $color={getMetricColor(systolic, 'systolicBP')}
                onClick={(e) => { e.stopPropagation(); handleMetricClick(assessment.id, 'systolicBP'); }}
              >
                <div className="label">Systolic BP</div>
                <div className="value">{systolic || '--'}</div>
                <div className="unit">mmHg</div>
              </MetricCard>

              <MetricCard 
                $isSelected={selectedMetric === 'diastolicBP'}
                $color={getMetricColor(diastolic, 'diastolicBP')}
                onClick={(e) => { e.stopPropagation(); handleMetricClick(assessment.id, 'diastolicBP'); }}
              >
                <div className="label">Diastolic BP</div>
                <div className="value">{diastolic || '--'}</div>
                <div className="unit">mmHg</div>
              </MetricCard>

              <MetricCard 
                $isSelected={selectedMetric === 'heartRateVariability'}
                $color={getMetricColor(assessment.results.heartRateVariability, 'heartRateVariability')}
                onClick={(e) => { e.stopPropagation(); handleMetricClick(assessment.id, 'heartRateVariability'); }}
              >
                <div className="label">HRV</div>
                <div className="value">{assessment.results.heartRateVariability ? Math.round(assessment.results.heartRateVariability) : '--'}</div>
                <div className="unit">ms</div>
              </MetricCard>

              <MetricCard 
                $isSelected={selectedMetric === 'stressLevel'}
                $color={theme.colors.success}
                onClick={(e) => { e.stopPropagation(); handleMetricClick(assessment.id, 'stressLevel'); }}
              >
                <div className="label">Stress Level</div>
                <div className="value">{getStressLevelDisplay(assessment.results.stressLevel).value}</div>
                <div className="unit">{getStressLevelDisplay(assessment.results.stressLevel).unit}</div>
              </MetricCard>

              <MetricCard 
                $isSelected={selectedMetric === 'respiratoryRate'}
                $color={getMetricColor(assessment.results.respiratoryRate, 'respiratoryRate')}
                onClick={(e) => { e.stopPropagation(); handleMetricClick(assessment.id, 'respiratoryRate'); }}
              >
                <div className="label">Respiratory Rate</div>
                <div className="value">{assessment.results.respiratoryRate || '--'}</div>
                <div className="unit">/min</div>
              </MetricCard>

              <MetricCard 
                $isSelected={selectedMetric === 'oxygenSaturation'}
                $color={getMetricColor(assessment.results.oxygenSaturation, 'oxygenSaturation')}
                onClick={(e) => { e.stopPropagation(); handleMetricClick(assessment.id, 'oxygenSaturation'); }}
              >
                <div className="label">O₂ Saturation</div>
                <div className="value">{assessment.results.oxygenSaturation || '--'}</div>
                <div className="unit">%</div>
              </MetricCard>

              {/* Lambda Health Metrics */}
              {assessment.results.healthScore !== undefined && (
                <MetricCard 
                  $isSelected={selectedMetric === 'healthScore'}
                  $color={theme.colors.primaryPurple}
                  onClick={(e) => { e.stopPropagation(); handleMetricClick(assessment.id, 'healthScore'); }}
                >
                  <div className="label">Health Score</div>
                  <div className="value">{assessment.results.healthScore?.toFixed(0) || '--'}</div>
                </MetricCard>
              )}

              {assessment.results.bmi !== undefined && (
                <MetricCard 
                  $isSelected={selectedMetric === 'bmi'}
                  $color={getMetricColor(assessment.results.bmi, 'bmi')}
                  onClick={(e) => { e.stopPropagation(); handleMetricClick(assessment.id, 'bmi'); }}
                >
                  <div className="label">BMI</div>
                  <div className="value">{assessment.results.bmi?.toFixed(1) || '--'}</div>
                  <div className="unit">kg/m²</div>
                </MetricCard>
              )}

              {assessment.results.heartAge !== undefined && (
                <MetricCard 
                  $isSelected={selectedMetric === 'heartAge'}
                  $color={theme.colors.primaryPurple}
                  onClick={(e) => { e.stopPropagation(); handleMetricClick(assessment.id, 'heartAge'); }}
                >
                  <div className="label">Heart Age</div>
                  <div className="value">{assessment.results.heartAge ? Math.round(assessment.results.heartAge) : '--'}</div>
                  <div className="unit">yrs</div>
                </MetricCard>
              )}

              {assessment.results.totalCholesterol !== undefined && (
                <MetricCard 
                  $isSelected={selectedMetric === 'totalCholesterol'}
                  $color={theme.colors.warning}
                  onClick={(e) => { e.stopPropagation(); handleMetricClick(assessment.id, 'totalCholesterol'); }}
                >
                  <div className="label">Total Cholesterol</div>
                  <div className="value">{assessment.results.totalCholesterol?.toFixed(1) || '--'}</div>
                  <div className="unit">mmol/L</div>
                </MetricCard>
              )}

              {assessment.results.ldlCholesterol !== undefined && (
                <MetricCard 
                  $isSelected={selectedMetric === 'ldlCholesterol'}
                  $color={theme.colors.warning}
                  onClick={(e) => { e.stopPropagation(); handleMetricClick(assessment.id, 'ldlCholesterol'); }}
                >
                  <div className="label">LDL</div>
                  <div className="value">{assessment.results.ldlCholesterol?.toFixed(1) || '--'}</div>
                  <div className="unit">mmol/L</div>
                </MetricCard>
              )}

              {assessment.results.hdlCholesterol !== undefined && (
                <MetricCard 
                  $isSelected={selectedMetric === 'hdlCholesterol'}
                  $color={theme.colors.success}
                  onClick={(e) => { e.stopPropagation(); handleMetricClick(assessment.id, 'hdlCholesterol'); }}
                >
                  <div className="label">HDL</div>
                  <div className="value">{assessment.results.hdlCholesterol?.toFixed(1) || '--'}</div>
                  <div className="unit">mmol/L</div>
                </MetricCard>
              )}

              {assessment.results.fitnessScore !== undefined && (
                <MetricCard 
                  $isSelected={selectedMetric === 'fitnessScore'}
                  $color={theme.colors.success}
                  onClick={(e) => { e.stopPropagation(); handleMetricClick(assessment.id, 'fitnessScore'); }}
                >
                  <div className="label">Fitness Score</div>
                  <div className="value">
                    {assessment.results.fitnessScore?.toFixed(1) || '--'}
                  </div>
                </MetricCard>
              )}

              {assessment.results.cvdRisk10Year !== undefined && (
                <MetricCard 
                  $isSelected={selectedMetric === 'cvdRisk10Year'}
                  $color={assessment.results.cvdRisk10Year && assessment.results.cvdRisk10Year > 10 ? theme.colors.warning : theme.colors.success}
                  onClick={(e) => { e.stopPropagation(); handleMetricClick(assessment.id, 'cvdRisk10Year'); }}
                >
                  <div className="label">10-Year CVD Risk</div>
                  <div className="value">{assessment.results.cvdRisk10Year?.toFixed(1) || '--'}</div>
                  <div className="unit">%</div>
                </MetricCard>
              )}
            </>
          )}
        </MetricsGrid>
        
        {/* Range indicator for selected metric */}
        {selectedMetric && (
          <RangeSection $isOpen={true}>
            <RangeValue>
              {(() => {
                let value = assessment.results[selectedMetric as keyof typeof assessment.results];
                
                // Handle special cases for systolic and diastolic
                if (selectedMetric === 'systolicBP' && !value) {
                  value = systolic;
                } else if (selectedMetric === 'diastolicBP' && !value) {
                  value = diastolic;
                }
                
                if (typeof value !== 'number' || !value) return null;
                
                const position = calculateRangePosition(value, selectedMetric);
                
                return (
                  <>
                    <RangeBar>
                      <RangeIndicator $position={position} />
                    </RangeBar>
                    <RangeLabels>
                      <span>Inadequate</span>
                      <span>Normal</span>
                      <span>Adequate</span>
                    </RangeLabels>
                  </>
                );
              })()}
            </RangeValue>
          </RangeSection>
        )}
      </AssessmentCard>
    );
  };
  
  return (
    <Layout>
      <PageHeader 
        title="Reports"
        subtitle="View your health assessment history"
      />
      <PageContent>
        <SearchContainer>
          <SearchInputWrapper>
            <SearchIcon>
              <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2">
                <path d="M21 21l-6-6m2-5a7 7 0 11-14 0 7 7 0 0114 0z" strokeLinecap="round" strokeLinejoin="round" />
              </svg>
            </SearchIcon>
            <SearchInput
              type="text"
              placeholder="Search by name, date, type, or metric..."
              value={searchQuery}
              onChange={(e) => setSearchQuery(e.target.value)}
            />
          </SearchInputWrapper>
        </SearchContainer>
        
        <FilterContainer>
          <FilterButton 
            $active={filter === 'all'} 
            onClick={() => setFilter('all')}
          >
            All Assessments
          </FilterButton>
          <FilterButton 
            $active={filter === 'body'} 
            onClick={() => setFilter('body')}
          >
            Body Scans
          </FilterButton>
          <FilterButton 
            $active={filter === 'face'} 
            onClick={() => setFilter('face')}
          >
            Face Scans
          </FilterButton>
          <FilterButton 
            $active={filter === 'health'} 
            onClick={() => setFilter('health')}
          >
            Health Assessment
          </FilterButton>
        </FilterContainer>
        
        {filteredAssessments.length === 0 ? (
          renderEmptyState()
        ) : (
          <AssessmentList>
            {filteredAssessments.map(renderAssessmentCard)}
          </AssessmentList>
        )}
      </PageContent>
    </Layout>
  );
};