import { BodyScanPDF } from './templates/BodyScanPDF';
import { FaceScanPDF } from './templates/FaceScanPDF';
import { HealthAssessmentPDF, HealthAssessmentData } from './templates/HealthAssessmentPDF';
import { 
  BodyScanData, 
  FaceScanData, 
  PDFGenerationOptions, 
  PDFMetadata 
} from './types';
import { AHI_LOGO_BASE64 } from './assets/logo';

export class PDFService {
  private static instance: PDFService;

  private constructor() {}

  static getInstance(): PDFService {
    if (!PDFService.instance) {
      PDFService.instance = new PDFService();
    }
    return PDFService.instance;
  }

  /**
   * Generate a body scan PDF report
   */
  async generateBodyScanPDF(
    data: BodyScanData,
    options?: Partial<PDFGenerationOptions>
  ): Promise<void> {
    const defaultOptions: PDFGenerationOptions = {
      metadata: {
        title: 'Body Composition Analysis Report',
        subject: 'Health Assessment',
        author: 'AHI BodyScan',
        keywords: ['body composition', 'health', 'fitness', 'BMI', 'body fat'],
        creator: 'AHI BodyScan System'
      },
      headerConfig: {
        logoPath: AHI_LOGO_BASE64,
        title: 'Body Composition Analysis',
        subtitle: 'Comprehensive Health Assessment Report',
        date: data.scanDate,
        reportId: `BSC-${data.scanDate.getTime()}`
      },
      includeCharts: true,
      includeRecommendations: true,
      includeRiskAssessment: true,
      language: 'en'
    };

    const mergedOptions = { ...defaultOptions, ...options };
    const pdfGenerator = new BodyScanPDF(data, mergedOptions);
    
    await pdfGenerator.generate();
    pdfGenerator.save();
  }

  /**
   * Generate a face scan PDF report
   */
  async generateFaceScanPDF(
    data: FaceScanData,
    options?: Partial<PDFGenerationOptions>
  ): Promise<void> {
    const defaultOptions: PDFGenerationOptions = {
      metadata: {
        title: 'Vital Signs Analysis Report',
        subject: 'Health Monitoring',
        author: 'AHI BodyScan',
        keywords: ['vital signs', 'heart rate', 'blood pressure', 'health monitoring'],
        creator: 'AHI BodyScan System'
      },
      headerConfig: {
        logoPath: AHI_LOGO_BASE64,
        title: 'Vital Signs Analysis',
        subtitle: 'Comprehensive Health Monitoring Report',
        date: data.scanDate,
        reportId: `FSC-${data.scanDate.getTime()}`
      },
      includeCharts: true,
      includeRecommendations: true,
      includeRiskAssessment: true,
      language: 'en'
    };

    const mergedOptions = { ...defaultOptions, ...options };
    const pdfGenerator = new FaceScanPDF(data, mergedOptions);
    
    await pdfGenerator.generate();
    pdfGenerator.save();
  }

  /**
   * Normalize stress level to ensure it's a valid value
   */
  private normalizeStressLevel(stressLevel?: string): 'low' | 'normal' | 'moderate' | 'high' {
    if (!stressLevel) return 'moderate';
    
    const normalized = stressLevel.toLowerCase();
    if (normalized === 'low' || normalized === 'normal' || normalized === 'moderate' || normalized === 'high') {
      return normalized as 'low' | 'normal' | 'moderate' | 'high';
    }
    
    // Handle variations
    if (normalized.includes('low') || normalized === 'minimal') return 'low';
    if (normalized.includes('normal')) return 'normal';
    if (normalized.includes('high') || normalized === 'severe' || normalized === 'elevated') return 'high';
    
    // Default to moderate for unknown values
    return 'moderate';
  }

  /**
   * Convert assessment data from ReportsScreen format to PDF format
   */
  convertAssessmentToBodyScanData(assessment: any): BodyScanData {
    // Try to get profile data from localStorage
    let profileData: any = null;
    const selectedProfile = localStorage.getItem('selectedProfile');
    if (selectedProfile) {
      profileData = JSON.parse(selectedProfile);
    }
    
    // If no selected profile or it doesn't match, try to find by profile name
    if (!profileData || profileData.name !== assessment.profileName) {
      const userEmail = localStorage.getItem('userEmail') || 'user@example.com';
      const profilesKey = `userProfiles_${userEmail}`;
      const profiles = JSON.parse(localStorage.getItem(profilesKey) || '[]');
      profileData = profiles.find((p: any) => p.name === assessment.profileName);
    }
    
    // Use profile data if available, otherwise use defaults
    const height = profileData?.height || 175;
    const weight = profileData?.weight || 70;
    
    return {
      scanDate: new Date(assessment.date),
      userId: assessment.id,
      userName: assessment.profileName || assessment.name || 'Anonymous',
      height: height,
      weight: weight,
      age: profileData?.age,
      gender: profileData?.gender as 'male' | 'female' | undefined,
      measurements: {
        bodyFat: assessment.results.bodyFat || 0,
        muscleMass: assessment.results.muscleMass || 0,
        bmi: assessment.results.bmi || (weight / Math.pow(height / 100, 2)),
        waist: assessment.results.waist || 0,
        chest: assessment.results.chest || 0,
        hip: assessment.results.hip || 0,
        thigh: assessment.results.thigh,
        inseam: assessment.results.inseam,
        visceralFat: assessment.results.visceralFat,
        fatFreeMass: assessment.results.fatFreeMass
      },
      analysis: {
        bodyType: assessment.results.bodyType,
        metabolicAge: assessment.results.metabolicAge,
        recommendations: []
      }
    };
  }

  /**
   * Convert assessment data from ReportsScreen format to PDF format
   */
  convertAssessmentToFaceScanData(assessment: any): FaceScanData {
    // Parse blood pressure if it's a string
    let systolic = assessment.results.systolicBP;
    let diastolic = assessment.results.diastolicBP;
    
    if (!systolic && !diastolic && assessment.results.bloodPressure) {
      const bpMatch = assessment.results.bloodPressure.match(/(\d+)\/(\d+)/);
      if (bpMatch) {
        systolic = parseInt(bpMatch[1]);
        diastolic = parseInt(bpMatch[2]);
      }
    }

    return {
      scanDate: new Date(assessment.date),
      userId: assessment.id,
      userName: assessment.profileName || assessment.name || 'Anonymous',
      age: undefined,
      gender: undefined,
      vitals: {
        heartRate: assessment.results.heartRate || 0,
        heartRateVariability: assessment.results.heartRateVariability,
        bloodPressure: {
          systolic: systolic || 120,
          diastolic: diastolic || 80
        },
        respiratoryRate: assessment.results.respiratoryRate || 16,
        oxygenSaturation: assessment.results.oxygenSaturation || 98,
        stressLevel: this.normalizeStressLevel(assessment.results.stressLevel),
        stressScore: assessment.results.stressScore
      },
      analysis: {
        overallHealth: 'good',
        riskFactors: [],
        recommendations: []
      }
    };
  }

  /**
   * Generate PDF from assessment data (used by ReportsScreen)
   */
  async generatePDFFromAssessment(assessment: any): Promise<void> {
    if (assessment.type === 'body') {
      const bodyScanData = this.convertAssessmentToBodyScanData(assessment);
      await this.generateBodyScanPDF(bodyScanData);
    } else if (assessment.type === 'face') {
      const faceScanData = this.convertAssessmentToFaceScanData(assessment);
      await this.generateFaceScanPDF(faceScanData);
    } else {
      throw new Error(`Unknown assessment type: ${assessment.type}`);
    }
  }

  /**
   * Generate a comprehensive health assessment PDF for a profile
   */
  async generateHealthAssessmentPDF(
    data: HealthAssessmentData,
    options?: Partial<PDFGenerationOptions>
  ): Promise<void> {
    const defaultOptions: PDFGenerationOptions = {
      metadata: {
        title: 'Comprehensive Health Assessment Report',
        subject: 'Complete Health Analysis',
        author: 'AHI BodyScan',
        keywords: ['health assessment', 'body composition', 'vital signs', 'health report'],
        creator: 'AHI BodyScan System'
      },
      headerConfig: {
        logoPath: AHI_LOGO_BASE64,
        title: 'Comprehensive Health Assessment',
        subtitle: `${data.profile.name}'s Complete Health Report`,
        date: data.generatedDate,
        reportId: `HA-${data.profile.id}-${data.generatedDate.getTime()}`
      },
      includeCharts: true,
      includeRecommendations: true,
      includeRiskAssessment: true,
      language: 'en'
    };

    const mergedOptions = { ...defaultOptions, ...options };
    const pdfGenerator = new HealthAssessmentPDF(data, mergedOptions);
    
    await pdfGenerator.generate();
    pdfGenerator.save();
  }
}

// Export singleton instance
export const pdfService = PDFService.getInstance();