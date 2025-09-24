import React, { useState } from 'react';
import styled from 'styled-components';
import { theme } from '../../styles/theme';
import { Modal, ModalBody } from '../../components/common/Modal';

interface Assessment {
  id: string;
  type: 'body' | 'face';
  date: string;
  status: 'completed';
  name?: string;
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
  };
}

interface DetailedReportModalProps {
  isOpen: boolean;
  onClose: () => void;
  assessment: Assessment | null;
}

const ModalContent = styled.div`
  padding: ${theme.dimensions.paddingLarge};
`;

const CurrentDateSection = styled.div`
  font-size: ${theme.typography.bodyMedium.fontSize};
  color: ${theme.colors.textSecondary};
  margin-bottom: ${theme.dimensions.paddingXLarge};
  text-align: center;
`;

const MetricsGrid = styled.div`
  display: grid;
  grid-template-columns: repeat(auto-fit, minmax(150px, 1fr));
  gap: ${theme.dimensions.paddingMedium};
  margin-bottom: ${theme.dimensions.paddingXLarge};
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
    
    .unit {
      font-size: ${theme.typography.bodyLarge.fontSize};
      font-weight: normal;
    }
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

export const DetailedReportModal: React.FC<DetailedReportModalProps> = ({ 
  isOpen, 
  onClose, 
  assessment 
}) => {
  const [selectedMetric, setSelectedMetric] = useState<string | null>(null);

  if (!assessment) return null;

  const formatDate = (dateString: string) => {
    const date = new Date(dateString);
    return date.toLocaleDateString('en-US', { 
      day: 'numeric',
      month: 'numeric', 
      year: 'numeric' 
    }) + ' ' + date.toLocaleTimeString('en-US', { 
      hour: 'numeric', 
      minute: '2-digit' 
    });
  };

  const isBodyScan = assessment.type === 'body';

  const handleMetricClick = (metricName: string) => {
    setSelectedMetric(selectedMetric === metricName ? null : metricName);
  };

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
    <Modal 
      isOpen={isOpen} 
      onClose={onClose} 
      title={isBodyScan ? 'Body Composition' : 'Vital Signs'}
      size="large"
    >
      <ModalBody>
        <ModalContent>
          <CurrentDateSection>
            {formatDate(assessment.date)}
          </CurrentDateSection>

          {isBodyScan ? (
            <>
              <MetricsGrid>
                <MetricCard 
                  $isSelected={selectedMetric === 'bodyFat'}
                  $color={getMetricColor(assessment.results.bodyFat, 'bodyFat')}
                  onClick={() => handleMetricClick('bodyFat')}
                >
                  <div className="label">Body Fat</div>
                  <div className="value">
                    {assessment.results.bodyFat || '--'}<span className="unit">%</span>
                  </div>
                </MetricCard>

                <MetricCard 
                  $isSelected={selectedMetric === 'muscleMass'}
                  $color={getMetricColor(assessment.results.muscleMass, 'muscleMass')}
                  onClick={() => handleMetricClick('muscleMass')}
                >
                  <div className="label">Muscle Mass</div>
                  <div className="value">
                    {assessment.results.muscleMass || '--'}<span className="unit">kg</span>
                  </div>
                </MetricCard>

                <MetricCard 
                  $isSelected={selectedMetric === 'bmi'}
                  $color={getMetricColor(assessment.results.bmi, 'bmi')}
                  onClick={() => handleMetricClick('bmi')}
                >
                  <div className="label">BMI</div>
                  <div className="value">{assessment.results.bmi || '--'}</div>
                </MetricCard>

                <MetricCard 
                  $isSelected={selectedMetric === 'waist'}
                  $color={theme.colors.primaryBlue}
                  onClick={() => handleMetricClick('waist')}
                >
                  <div className="label">Waist</div>
                  <div className="value">
                    {assessment.results.waist || '--'}<span className="unit">cm</span>
                  </div>
                </MetricCard>

                <MetricCard 
                  $isSelected={selectedMetric === 'chest'}
                  $color={theme.colors.primaryBlue}
                  onClick={() => handleMetricClick('chest')}
                >
                  <div className="label">Chest</div>
                  <div className="value">
                    {assessment.results.chest || '--'}<span className="unit">cm</span>
                  </div>
                </MetricCard>

                <MetricCard 
                  $isSelected={selectedMetric === 'hip'}
                  $color={theme.colors.primaryBlue}
                  onClick={() => handleMetricClick('hip')}
                >
                  <div className="label">Hip</div>
                  <div className="value">
                    {assessment.results.hip || '--'}<span className="unit">cm</span>
                  </div>
                </MetricCard>
              </MetricsGrid>
            </>
          ) : (
            <>
              <MetricsGrid>
                <MetricCard 
                  $isSelected={selectedMetric === 'heartRate'}
                  $color={getMetricColor(assessment.results.heartRate, 'heartRate')}
                  onClick={() => handleMetricClick('heartRate')}
                >
                  <div className="label">Heart Rate</div>
                  <div className="value">
                    {assessment.results.heartRate || '--'}<span className="unit">bpm</span>
                  </div>
                </MetricCard>

                <MetricCard 
                  $isSelected={selectedMetric === 'systolicBP'}
                  $color={getMetricColor(systolic, 'systolicBP')}
                  onClick={() => handleMetricClick('systolicBP')}
                >
                  <div className="label">Systolic BP</div>
                  <div className="value">
                    {systolic || '--'}<span className="unit">mmHg</span>
                  </div>
                </MetricCard>

                <MetricCard 
                  $isSelected={selectedMetric === 'diastolicBP'}
                  $color={getMetricColor(diastolic, 'diastolicBP')}
                  onClick={() => handleMetricClick('diastolicBP')}
                >
                  <div className="label">Diastolic BP</div>
                  <div className="value">
                    {diastolic || '--'}<span className="unit">mmHg</span>
                  </div>
                </MetricCard>

                <MetricCard 
                  $isSelected={selectedMetric === 'stressLevel'}
                  $color={theme.colors.success}
                  onClick={() => handleMetricClick('stressLevel')}
                >
                  <div className="label">Stress Level</div>
                  <div className="value">{assessment.results.stressLevel || '--'}</div>
                </MetricCard>

                <MetricCard 
                  $isSelected={selectedMetric === 'respiratoryRate'}
                  $color={getMetricColor(assessment.results.respiratoryRate, 'respiratoryRate')}
                  onClick={() => handleMetricClick('respiratoryRate')}
                >
                  <div className="label">Respiratory Rate</div>
                  <div className="value">
                    {assessment.results.respiratoryRate || '--'}<span className="unit">/min</span>
                  </div>
                </MetricCard>

                <MetricCard 
                  $isSelected={selectedMetric === 'oxygenSaturation'}
                  $color={getMetricColor(assessment.results.oxygenSaturation, 'oxygenSaturation')}
                  onClick={() => handleMetricClick('oxygenSaturation')}
                >
                  <div className="label">O₂ Saturation</div>
                  <div className="value">
                    {assessment.results.oxygenSaturation || '--'}<span className="unit">%</span>
                  </div>
                </MetricCard>

                <MetricCard 
                  $isSelected={selectedMetric === 'heartRateVariability'}
                  $color={getMetricColor(assessment.results.heartRateVariability, 'heartRateVariability')}
                  onClick={() => handleMetricClick('heartRateVariability')}
                >
                  <div className="label">HRV</div>
                  <div className="value">
                    {assessment.results.heartRateVariability ? Math.round(assessment.results.heartRateVariability) : '--'}<span className="unit">ms</span>
                  </div>
                </MetricCard>
              </MetricsGrid>
            </>
          )}

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
        </ModalContent>
      </ModalBody>
    </Modal>
  );
};