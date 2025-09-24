import { RiskLevel, HealthMetric, BodyScanData, FaceScanData } from '../types';

export const calculateBMIRisk = (bmi: number): RiskLevel => {
  if (bmi < 18.5) {
    return {
      level: 'moderate',
      score: 0.6,
      color: '#FFC107',
      description: 'Underweight - May indicate nutritional deficiency'
    };
  } else if (bmi >= 18.5 && bmi < 25) {
    return {
      level: 'low',
      score: 0.2,
      color: '#28A745',
      description: 'Normal weight - Healthy BMI range'
    };
  } else if (bmi >= 25 && bmi < 30) {
    return {
      level: 'moderate',
      score: 0.6,
      color: '#FFC107',
      description: 'Overweight - Increased health risks'
    };
  } else {
    return {
      level: 'high',
      score: 0.8,
      color: '#DC3545',
      description: 'Obese - Significant health risks'
    };
  }
};

export const calculateBodyFatRisk = (bodyFat: number, gender?: 'male' | 'female'): RiskLevel => {
  const thresholds = gender === 'female' 
    ? { low: 21, moderate: 33, high: 39 }
    : { low: 14, moderate: 25, high: 31 };

  if (bodyFat < thresholds.low) {
    return {
      level: 'low',
      score: 0.2,
      color: '#28A745',
      description: 'Athletic body fat percentage'
    };
  } else if (bodyFat < thresholds.moderate) {
    return {
      level: 'low',
      score: 0.3,
      color: '#28A745',
      description: 'Healthy body fat percentage'
    };
  } else if (bodyFat < thresholds.high) {
    return {
      level: 'moderate',
      score: 0.6,
      color: '#FFC107',
      description: 'Above average body fat'
    };
  } else {
    return {
      level: 'high',
      score: 0.8,
      color: '#DC3545',
      description: 'High body fat - Health risks present'
    };
  }
};

export const calculateBloodPressureRisk = (systolic: number, diastolic: number): RiskLevel => {
  if (systolic < 120 && diastolic < 80) {
    return {
      level: 'low',
      score: 0.2,
      color: '#28A745',
      description: 'Normal blood pressure'
    };
  } else if (systolic < 130 && diastolic < 80) {
    return {
      level: 'moderate',
      score: 0.4,
      color: '#FFC107',
      description: 'Elevated blood pressure'
    };
  } else if (systolic < 140 || diastolic < 90) {
    return {
      level: 'high',
      score: 0.7,
      color: '#FF8C00',
      description: 'Stage 1 Hypertension'
    };
  } else {
    return {
      level: 'critical',
      score: 0.9,
      color: '#DC3545',
      description: 'Stage 2 Hypertension - Seek medical attention'
    };
  }
};

export const calculateHeartRateRisk = (heartRate: number, age?: number): RiskLevel => {
  const maxHR = age ? 220 - age : 200;
  const restingHRThresholds = {
    excellent: 60,
    good: 70,
    average: 80,
    poor: 90
  };

  if (heartRate < 50) {
    return {
      level: 'moderate',
      score: 0.5,
      color: '#FFC107',
      description: 'Bradycardia - Low heart rate'
    };
  } else if (heartRate < restingHRThresholds.excellent) {
    return {
      level: 'low',
      score: 0.1,
      color: '#28A745',
      description: 'Excellent resting heart rate'
    };
  } else if (heartRate < restingHRThresholds.good) {
    return {
      level: 'low',
      score: 0.2,
      color: '#28A745',
      description: 'Good resting heart rate'
    };
  } else if (heartRate < restingHRThresholds.average) {
    return {
      level: 'moderate',
      score: 0.4,
      color: '#FFC107',
      description: 'Average resting heart rate'
    };
  } else if (heartRate < restingHRThresholds.poor) {
    return {
      level: 'moderate',
      score: 0.6,
      color: '#FFC107',
      description: 'Below average resting heart rate'
    };
  } else {
    return {
      level: 'high',
      score: 0.8,
      color: '#DC3545',
      description: 'Tachycardia - Elevated heart rate'
    };
  }
};

export const calculateOxygenSaturationRisk = (spo2: number): RiskLevel => {
  if (spo2 >= 95) {
    return {
      level: 'low',
      score: 0.1,
      color: '#28A745',
      description: 'Normal oxygen saturation'
    };
  } else if (spo2 >= 90) {
    return {
      level: 'moderate',
      score: 0.5,
      color: '#FFC107',
      description: 'Below normal oxygen saturation'
    };
  } else {
    return {
      level: 'critical',
      score: 0.9,
      color: '#DC3545',
      description: 'Hypoxemia - Seek immediate medical attention'
    };
  }
};

export const calculateStressRisk = (stressLevel: 'low' | 'normal' | 'moderate' | 'high'): RiskLevel => {
  switch (stressLevel) {
    case 'low':
      return {
        level: 'low',
        score: 0.2,
        color: '#28A745',
        description: 'Low stress levels - Good mental health'
      };
    case 'normal':
      return {
        level: 'low',
        score: 0.3,
        color: '#28A745',
        description: 'Normal stress levels - Healthy range'
      };
    case 'moderate':
      return {
        level: 'moderate',
        score: 0.5,
        color: '#FFC107',
        description: 'Moderate stress - Consider stress management'
      };
    case 'high':
      return {
        level: 'high',
        score: 0.8,
        color: '#DC3545',
        description: 'High stress - Stress reduction recommended'
      };
    default:
      // Default to moderate if unknown value
      return {
        level: 'moderate',
        score: 0.5,
        color: '#FFC107',
        description: 'Stress level unknown'
      };
  }
};

export const calculateOverallBodyScanRisk = (data: BodyScanData): RiskLevel => {
  const risks: RiskLevel[] = [];
  
  // Calculate individual risks
  risks.push(calculateBMIRisk(data.measurements.bmi));
  risks.push(calculateBodyFatRisk(data.measurements.bodyFat, data.gender));
  
  // Calculate waist-to-hip ratio risk if available
  if (data.measurements.waist && data.measurements.hip) {
    const whr = data.measurements.waist / data.measurements.hip;
    const whrThreshold = data.gender === 'female' ? 0.85 : 0.95;
    
    if (whr > whrThreshold) {
      risks.push({
        level: 'moderate',
        score: 0.6,
        color: '#FFC107',
        description: 'Elevated waist-to-hip ratio'
      });
    }
  }
  
  // Filter out any undefined risks and calculate average risk score
  const validRisks = risks.filter(risk => risk && risk.score !== undefined);
  const avgScore = validRisks.length > 0 
    ? validRisks.reduce((sum, risk) => sum + risk.score, 0) / validRisks.length
    : 0.5; // Default to moderate if no valid risks
  
  if (avgScore < 0.3) {
    return {
      level: 'low',
      score: avgScore,
      color: '#28A745',
      description: 'Overall low health risk'
    };
  } else if (avgScore < 0.6) {
    return {
      level: 'moderate',
      score: avgScore,
      color: '#FFC107',
      description: 'Overall moderate health risk'
    };
  } else {
    return {
      level: 'high',
      score: avgScore,
      color: '#DC3545',
      description: 'Overall high health risk - Lifestyle changes recommended'
    };
  }
};

export const calculateOverallFaceScanRisk = (data: FaceScanData): RiskLevel => {
  const risks: RiskLevel[] = [];
  
  // Calculate individual risks
  risks.push(calculateHeartRateRisk(data.vitals.heartRate, data.age));
  risks.push(calculateBloodPressureRisk(
    data.vitals.bloodPressure.systolic,
    data.vitals.bloodPressure.diastolic
  ));
  risks.push(calculateOxygenSaturationRisk(data.vitals.oxygenSaturation));
  risks.push(calculateStressRisk(data.vitals.stressLevel));
  
  // Filter out any undefined risks and calculate average risk score
  const validRisks = risks.filter(risk => risk && risk.score !== undefined);
  const avgScore = validRisks.length > 0 
    ? validRisks.reduce((sum, risk) => sum + risk.score, 0) / validRisks.length
    : 0.5; // Default to moderate if no valid risks
  
  // Check for critical conditions
  const hasCritical = risks.some(risk => risk.level === 'critical');
  
  if (hasCritical) {
    return {
      level: 'critical',
      score: 0.9,
      color: '#DC3545',
      description: 'Critical health indicators detected - Seek medical attention'
    };
  } else if (avgScore < 0.3) {
    return {
      level: 'low',
      score: avgScore,
      color: '#28A745',
      description: 'Excellent vital signs'
    };
  } else if (avgScore < 0.6) {
    return {
      level: 'moderate',
      score: avgScore,
      color: '#FFC107',
      description: 'Some vital signs need attention'
    };
  } else {
    return {
      level: 'high',
      score: avgScore,
      color: '#DC3545',
      description: 'Multiple vital signs elevated - Medical consultation recommended'
    };
  }
};

export const getHealthRecommendations = (
  riskLevel: RiskLevel,
  scanType: 'body' | 'face'
): string[] => {
  const recommendations: string[] = [];
  
  if (scanType === 'body') {
    switch (riskLevel.level) {
      case 'low':
        recommendations.push('Maintain current healthy lifestyle');
        recommendations.push('Continue regular physical activity');
        recommendations.push('Keep balanced nutrition habits');
        break;
      case 'moderate':
        recommendations.push('Increase physical activity to 150 minutes per week');
        recommendations.push('Review and improve dietary habits');
        recommendations.push('Consider consulting a nutritionist');
        recommendations.push('Monitor progress with regular scans');
        break;
      case 'high':
      case 'critical':
        recommendations.push('Consult healthcare provider for personalized plan');
        recommendations.push('Implement structured exercise program');
        recommendations.push('Work with registered dietitian');
        recommendations.push('Set realistic weight loss goals');
        recommendations.push('Track progress weekly');
        break;
    }
  } else {
    switch (riskLevel.level) {
      case 'low':
        recommendations.push('Continue healthy lifestyle habits');
        recommendations.push('Maintain stress management practices');
        recommendations.push('Regular health check-ups annually');
        break;
      case 'moderate':
        recommendations.push('Practice stress reduction techniques');
        recommendations.push('Improve sleep quality and duration');
        recommendations.push('Consider meditation or yoga');
        recommendations.push('Monitor vital signs regularly');
        break;
      case 'high':
        recommendations.push('Schedule medical consultation');
        recommendations.push('Daily vital signs monitoring');
        recommendations.push('Implement stress reduction program');
        recommendations.push('Review medications with doctor');
        break;
      case 'critical':
        recommendations.push('Seek immediate medical attention');
        recommendations.push('Do not delay medical consultation');
        recommendations.push('Bring this report to your doctor');
        break;
    }
  }
  
  return recommendations;
};