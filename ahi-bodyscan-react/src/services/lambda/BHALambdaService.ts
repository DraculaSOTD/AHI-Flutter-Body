export interface BHARequestData {
  // User demographics
  enum_ent_sex: string;
  date_ent_dob: string;
  date_ent_assessment: string; // Current date when assessment happens
  cm_ent_height: number;
  kg_ent_weight: number;
  
  // Activity and health info
  enum_ent_activityLevel?: string;
  enum_ent_chronicMedication?: string;
  enum_ent_smoker?: string;
  bool_ent_bpMedication?: boolean;
  
  // Vital signs from face scan
  mmHg_ent_systolicBP: number;
  mmHg_ent_diastolicBP: number;
  bpm_ent_restingHeartRate: number;
  
  // Additional metrics
  int_raw_rr?: number;
  int_raw_oxygen?: number;
  flt_raw_ibi?: number;
  flt_raw_sdnn?: number;
  flt_raw_rmssd?: number;
  flt_raw_stressIndex?: number;
}

export interface BHAResponse {
  health_score?: number;
  risk_adj_bloodPressure?: string;
  risk_adj_heartRate?: string;
  risk_adj_respiratoryRate?: string;
  risk_adj_oxygenSaturation?: string;
  risk_adj_stressLevel?: string;
  risk_adj_arterialStiffness?: string;
  risk_adj_bmi?: string;
  
  // Calculated values
  kgsm_raw_bmi?: number;
  cms_raw_arterialStiffness?: number;
  yr_raw_heartAge?: number;
  yr_raw_biologicalAge?: number;
  
  // Risk scores
  risk_cardiovascular?: string;
  risk_diabetes?: string;
  risk_metabolic?: string;
  
  // Recommendations
  recommendations?: string[];
  insights?: string[];
}

export class BHALambdaService {
  // Always use local proxy URL - nginx handles forwarding to AWS Lambda
  private static readonly API_URL = '/bha/';
  
  /**
   * Submit face scan results to AWS Lambda for health assessment
   */
  static async submitForHealthAssessment(data: BHARequestData): Promise<BHAResponse> {
    console.log('🚀 Submitting to Lambda API:', this.API_URL);
    console.log('📦 Request data:', JSON.stringify(data, null, 2));
    
    try {
      const response = await fetch(this.API_URL, {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
          'Accept': 'application/json',
        },
        body: JSON.stringify(data),
      });
      
      const responseText = await response.text();
      console.log('📨 Response status:', response.status);
      console.log('📨 Response text:', responseText);
      
      if (!response.ok) {
        console.error('❌ Lambda request failed:', response.status, responseText);
        throw new Error(`Lambda request failed: ${response.status} - ${responseText}`);
      }
      
      const result = JSON.parse(responseText);
      console.log('✅ Lambda response parsed:', result);
      
      // Check if it's an error response
      if (result.errorType || result.errorMessage) {
        console.error('❌ Lambda returned error:', result);
        throw new Error(`Lambda error: ${result.errorMessage || result.errorType}`);
      }
      
      return result;
    } catch (error) {
      console.error('❌ Error submitting to Lambda:', error);
      throw error;
    }
  }
  
  /**
   * Prepare face scan data for Lambda submission
   */
  static prepareScanData(
    scanResults: any,
    userData: any
  ): BHARequestData {
    // Calculate age from DOB
    const birthDate = new Date(userData.dateOfBirth || userData.dob);
    const age = new Date().getFullYear() - birthDate.getFullYear();
    
    // Validate and clamp values to Lambda's requirements
    const clamp = (value: number, min: number, max: number): number => {
      return Math.min(Math.max(value, min), max);
    };
    
    // Convert height to cm if needed
    let height = userData.height || userData.heightCm || 170;
    
    // Check if height needs conversion from feet/inches
    if (userData.heightUnit === 'ft' && userData.heightFeet !== undefined) {
      const feet = parseFloat(userData.heightFeet) || 0;
      const inches = parseFloat(userData.heightInches) || 0;
      height = (feet * 30.48) + (inches * 2.54);
    }
    
    // Ensure height is within valid range (100-250 cm)
    if (height < 100) height = 170; // Use default if too small
    height = clamp(height, 100, 250);
    
    // Convert weight to kg if needed
    let weight = userData.weight || userData.weightKg || 70;
    
    // Check if weight needs conversion from lbs
    if (userData.weightUnit === 'lbs') {
      weight = weight * 0.453592;
    }
    
    // Ensure weight is reasonable
    weight = clamp(weight, 30, 250);
    
    // Ensure heart rate is within valid range (20-120 bpm)
    let heartRate = scanResults.heartRate || 70;
    if (heartRate <= 0) heartRate = 70; // Use default if invalid
    heartRate = clamp(heartRate, 20, 120);
    
    console.log('🔍 Validated data for Lambda:', {
      height,
      weight,
      heartRate,
      originalHR: scanResults.heartRate,
      originalHeight: userData.height,
    });
    
    return {
      // User demographics
      enum_ent_sex: userData.sex || userData.gender || 'male',
      date_ent_dob: userData.dateOfBirth || userData.dob || new Date(new Date().getFullYear() - 30, 0, 1).toISOString().split('T')[0],
      date_ent_assessment: new Date().toISOString().split('T')[0], // Current date in YYYY-MM-DD format
      cm_ent_height: Math.round(height),
      kg_ent_weight: Math.round(weight),
      
      // Activity and health (defaults if not provided)
      // Lambda expects specific enum values from the BHA API:
      // activityLevel: inactive, exercise10Mins, exercise20to60Mins, exercise1to3Hours, exerciseOver3Hours
      // chronicMedication: none, oneOrTwoDiseases, threeOrMoreDiseases
      // smoker: never, former, current
      enum_ent_activityLevel: userData.exerciseLevel || userData.activityLevel || 'inactive',
      enum_ent_chronicMedication: userData.chronicMedication || 'none',
      // Smoking status should already be 'never', 'former', or 'current' from the UI
      enum_ent_smoker: userData.smokingStatus || userData.smoker || 'never',
      // Convert takingBPMedication from 'yes'/'no' to boolean
      bool_ent_bpMedication: userData.takingBPMedication === 'yes' || userData.bpMedication || false,
      
      // Vital signs from scan - with validation
      mmHg_ent_systolicBP: Math.round(clamp(scanResults.systolicBP || 120, 70, 200)),
      mmHg_ent_diastolicBP: Math.round(clamp(scanResults.diastolicBP || 80, 40, 130)),
      bpm_ent_restingHeartRate: Math.round(heartRate),
      
      // Additional metrics
      int_raw_rr: Math.round(clamp(scanResults.respiratoryRate || 16, 8, 40)),
      int_raw_oxygen: Math.round(clamp(scanResults.oxygenSaturation || 98, 70, 100)),
      flt_raw_ibi: scanResults.ibi || 800,
      flt_raw_sdnn: scanResults.sdnn || scanResults.heartRateVariability || 50,
      flt_raw_rmssd: scanResults.rmssd || 40,
      flt_raw_stressIndex: scanResults.stressIndex || this.calculateStressIndex(scanResults.stressLevel),
    };
  }
  
  /**
   * Calculate stress index from stress level string
   */
  private static calculateStressIndex(stressLevel?: string): number {
    switch (stressLevel?.toLowerCase()) {
      case 'low': return 2;
      case 'normal': return 5;
      case 'moderate': return 7;
      case 'elevated': return 8;
      case 'high': return 9;
      default: return 5;
    }
  }
  
  /**
   * Process Lambda response and merge with scan results
   */
  static processBHAResponse(
    scanResults: any,
    bhaResponse: any
  ): any {
    console.log('🔄 Processing BHA response...');
    console.log('Original scan results:', scanResults);
    console.log('BHA response:', bhaResponse);
    
    // Lambda returns data in different structure
    const outputs = bhaResponse.outputs || {};
    const risks = bhaResponse.risks || {};
    
    // Calculate health score based on risk factors
    const calculateHealthScore = () => {
      const riskValues = Object.values(risks);
      const lowCount = riskValues.filter((r: any) => r === 'low').length;
      const totalCount = riskValues.length || 1;
      return Math.round((lowCount / totalCount) * 100);
    };
    
    // Fix oxygen saturation if it's 0
    if (scanResults.oxygenSaturation === 0) {
      scanResults.oxygenSaturation = outputs.int_raw_oxygen || 98;
    }
    
    const enhancedResults = {
      ...scanResults,
      // Add health assessment data from Lambda
      healthScore: calculateHealthScore(),
      heartAge: outputs.yr_raw_age ? outputs.yr_raw_age + 5 : undefined, // Estimate heart age
      biologicalAge: outputs.yr_raw_age || undefined,
      arterialStiffness: outputs.cms_adj_bapwv || undefined,
      bmi: outputs.kgsm_raw_bmi || undefined,
      
      // Cholesterol and Lipid Panel
      totalCholesterol: outputs.mmoll_raw_corrTotalCholesterolMean || undefined,
      ldlCholesterol: outputs.mmoll_raw_corrLdlMean || undefined,
      hdlCholesterol: outputs.mmoll_raw_corrHdlcMean || undefined,
      triglycerides: outputs.mmoll_raw_corrTriglyceridesMean || undefined,
      
      // Fitness and Cardiovascular Metrics
      fitnessScore: outputs.num_raw_fitnessAvg || undefined,
      vo2Max: outputs.num_raw_fitnessAvg || undefined, // VO2 Max is same as fitness score
      cvdRisk10Year: outputs.num_raw_tenYrCvd || undefined,
      framinghamScore: outputs.int_raw_framinghamScore || undefined,
      maxHeartRate: outputs.bpm_raw_maxHR || undefined,
      meanArterialPressure: outputs.mmhg_raw_map || undefined,
      
      // Risk assessments from Lambda risks object
      bloodPressureRisk: risks.risk_adj_bloodPressure,
      heartRateRisk: risks.risk_adj_restingHeartRate,
      cardiovascularRisk: risks.risk_adj_tenYrCvd,
      diabetesRisk: risks.risk_adj_metSComp,
      metabolicRisk: risks.risk_adj_metSComp,
      cholesterolRisk: risks.risk_adj_totalCholesterol,
      ldlRisk: risks.risk_adj_ldlC,
      hdlRisk: risks.risk_adj_hdlC,
      triglyceridesRisk: risks.risk_adj_triglycerides,
      
      // Generate insights based on risks
      recommendations: this.generateRecommendations(risks),
      insights: this.generateInsights(outputs, risks),
    };
    
    console.log('✨ Enhanced results with Lambda data:', enhancedResults);
    return enhancedResults;
  }
  
  /**
   * Generate recommendations based on risk assessment
   */
  private static generateRecommendations(risks: any): string[] {
    const recommendations = [];
    
    if (risks.risk_adj_activityLevel === 'high') {
      recommendations.push('Increase physical activity to at least 150 minutes of moderate exercise per week');
    }
    if (risks.risk_adj_restingHeartRate === 'high') {
      recommendations.push('Consider cardiovascular exercise to improve resting heart rate');
    }
    if (risks.risk_adj_bapwv === 'high') {
      recommendations.push('Focus on cardiovascular health to improve arterial stiffness');
    }
    if (risks.risk_adj_metSComp === 'high') {
      recommendations.push('Monitor metabolic health markers and consider dietary changes');
    }
    
    return recommendations;
  }
  
  /**
   * Generate insights based on Lambda data
   */
  private static generateInsights(outputs: any, risks: any): string[] {
    const insights = [];
    
    if (outputs.kgsm_raw_bmi) {
      const bmi = outputs.kgsm_raw_bmi;
      if (bmi < 18.5) insights.push('Your BMI indicates you are underweight');
      else if (bmi < 25) insights.push('Your BMI is in the healthy range');
      else if (bmi < 30) insights.push('Your BMI indicates you are overweight');
      else insights.push('Your BMI indicates obesity');
    }
    
    const highRisks = Object.entries(risks).filter(([_, value]) => value === 'high').length;
    if (highRisks === 0) {
      insights.push('All health risk indicators are within acceptable ranges');
    } else {
      insights.push(`${highRisks} health indicators require attention`);
    }
    
    return insights;
  }
}