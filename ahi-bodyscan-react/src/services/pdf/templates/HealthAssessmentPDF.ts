import { PDFGenerator } from '../PDFGenerator';
import { BodyScanData, FaceScanData, PDFGenerationOptions } from '../types';
import { UserProfile } from '../../../utils/userDataHelpers';
import { ScanProgress } from '../../../utils/scanDataHelpers';
import { 
  calculateBMIRisk, 
  calculateBodyFatRisk, 
  calculateOverallBodyScanRisk,
  calculateHeartRateRisk,
  calculateBloodPressureRisk,
  calculateOverallFaceScanRisk,
  getHealthRecommendations 
} from '../utils/riskAssessment';
import { 
  createBodyCompositionChart, 
  createMeasurementsChart,
  createVitalsChart
} from '../utils/charts';
import { AHI_LOGO_BASE64 } from '../assets/logo';

export interface HealthAssessmentData {
  profile: UserProfile;
  scanProgress: ScanProgress;
  bodyScanData?: BodyScanData;
  faceScanData?: FaceScanData;
  generatedDate: Date;
}

export class HealthAssessmentPDF {
  private generator: PDFGenerator;
  private data: HealthAssessmentData;
  private options: PDFGenerationOptions;

  constructor(data: HealthAssessmentData, options: PDFGenerationOptions) {
    this.generator = new PDFGenerator();
    this.data = data;
    this.options = options;
  }

  async generate(): Promise<void> {
    // Add header
    this.generator.addHeader({
      logoPath: AHI_LOGO_BASE64,
      title: 'Comprehensive Health Assessment',
      subtitle: `${this.data.profile.name}'s Complete Health Report`,
      date: this.data.generatedDate,
      reportId: `HA-${this.data.profile.id}-${this.data.generatedDate.getTime()}`
    });

    // Add profile information section
    this.addProfileInfo();

    // Add scan completion summary
    this.addScanSummary();

    // Add body scan results if available
    if (this.data.bodyScanData) {
      this.generator.doc.addPage();
      this.generator.currentY = this.generator.margin.top;
      await this.addBodyScanSection();
    }

    // Add face scan results if available
    if (this.data.faceScanData) {
      this.generator.doc.addPage();
      this.generator.currentY = this.generator.margin.top;
      await this.addFaceScanSection();
    }

    // Add overall health assessment
    if (this.data.bodyScanData && this.data.faceScanData) {
      this.generator.doc.addPage();
      this.generator.currentY = this.generator.margin.top;
      await this.addOverallHealthAssessment();
    }

    // Add recommendations
    if (this.options.includeRecommendations) {
      this.addRecommendations();
    }
  }

  private addProfileInfo(): void {
    this.generator.addSection({
      title: 'Profile Information',
      content: () => {
        const info = [
          ['Name', this.data.profile.name],
          ['Age', this.data.profile.age ? `${this.data.profile.age} years` : 'Not specified'],
          ['Gender', this.data.profile.gender],
          ['Height', `${this.data.profile.height} ${this.data.profile.heightUnit || 'cm'}`],
          ['Weight', `${this.data.profile.weight} ${this.data.profile.weightUnit || 'kg'}`],
          ['Profile Type', this.data.profile.isMainProfile ? 'Main Profile' : 'Additional Profile']
        ];
        
        this.generator.addTable(info, ['Field', 'Value']);
      }
    });
  }

  private addScanSummary(): void {
    this.generator.addSection({
      title: 'Scan Completion Summary',
      content: () => {
        const summaryData = [
          ['Assessment Completion', `${this.data.scanProgress.progressPercentage}%`],
          ['Completed Scans', `${this.data.scanProgress.completedScans} of ${this.data.scanProgress.totalScansRequired}`],
          ['Body Scan', this.data.scanProgress.completedScanTypes.includes('body') ? '✓ Completed' : 'Pending'],
          ['Face Scan', this.data.scanProgress.completedScanTypes.includes('face') ? '✓ Completed' : 'Pending']
        ];
        
        this.generator.addTable(summaryData, ['Scan Type', 'Status']);
      }
    });
  }

  private async addBodyScanSection(): Promise<void> {
    if (!this.data.bodyScanData) return;

    // Add section heading
    this.generator.doc.setFontSize(16);
    this.generator.doc.setFont(this.generator.style.fontFamily, 'bold');
    this.generator.doc.text('Body Composition Analysis', this.generator.margin.left, this.generator.currentY);
    this.generator.currentY += 10;

    // Add key body metrics
    this.generator.addSection({
      title: 'Key Body Metrics',
      content: () => {
        const metrics = [
          ['Body Fat', `${this.data.bodyScanData!.measurements.bodyFat}%`],
          ['Muscle Mass', `${this.data.bodyScanData!.measurements.muscleMass} kg`],
          ['BMI', this.data.bodyScanData!.measurements.bmi?.toFixed(1) || 'N/A'],
          ['Visceral Fat', this.data.bodyScanData!.measurements.visceralFat?.toString() || 'N/A']
        ];
        this.generator.addTable(metrics, ['Metric', 'Value']);
      }
    });

    // Add body measurements
    this.generator.addSection({
      title: 'Body Measurements',
      content: () => {
        const measurements = [
          ['Waist', `${this.data.bodyScanData!.measurements.waist} cm`],
          ['Chest', `${this.data.bodyScanData!.measurements.chest} cm`],
          ['Hip', `${this.data.bodyScanData!.measurements.hip} cm`],
          ['Thigh', this.data.bodyScanData!.measurements.thigh ? `${this.data.bodyScanData!.measurements.thigh} cm` : 'N/A']
        ];
        this.generator.addTable(measurements, ['Measurement', 'Value']);
      }
    });

    // Add body composition chart
    if (this.options.includeCharts) {
      const chartDataUrl = await createBodyCompositionChart(
        this.data.bodyScanData.measurements.bodyFat,
        this.data.bodyScanData.measurements.muscleMass,
        this.data.bodyScanData.measurements.fatFreeMass
      );
      
      this.generator.doc.addImage(
        chartDataUrl,
        'PNG',
        this.generator.margin.left,
        this.generator.currentY,
        this.generator.pageWidth - this.generator.margin.left - this.generator.margin.right,
        80
      );
      this.generator.currentY += 90;
    }
  }

  private async addFaceScanSection(): Promise<void> {
    if (!this.data.faceScanData) return;

    // Add section heading
    this.generator.doc.setFontSize(16);
    this.generator.doc.setFont(this.generator.style.fontFamily, 'bold');
    this.generator.doc.text('Vital Signs Analysis', this.generator.margin.left, this.generator.currentY);
    this.generator.currentY += 10;

    // Add vital signs
    this.generator.addSection({
      title: 'Vital Signs',
      content: () => {
        const vitals = [
          ['Heart Rate', `${this.data.faceScanData!.vitals.heartRate} bpm`],
          ['Blood Pressure', `${this.data.faceScanData!.vitals.bloodPressure.systolic}/${this.data.faceScanData!.vitals.bloodPressure.diastolic} mmHg`],
          ['Respiratory Rate', `${this.data.faceScanData!.vitals.respiratoryRate} breaths/min`],
          ['Oxygen Saturation', `${this.data.faceScanData!.vitals.oxygenSaturation}%`],
          ['Stress Level', this.data.faceScanData!.vitals.stressLevel.charAt(0).toUpperCase() + this.data.faceScanData!.vitals.stressLevel.slice(1)]
        ];
        this.generator.addTable(vitals, ['Vital Sign', 'Value']);
      }
    });

    // Add vitals chart
    if (this.options.includeCharts) {
      const chartDataUrl = await createVitalsChart({
        heartRate: this.data.faceScanData.vitals.heartRate,
        systolic: this.data.faceScanData.vitals.bloodPressure.systolic,
        diastolic: this.data.faceScanData.vitals.bloodPressure.diastolic,
        respiratoryRate: this.data.faceScanData.vitals.respiratoryRate,
        oxygenSaturation: this.data.faceScanData.vitals.oxygenSaturation
      });
      
      this.generator.doc.addImage(
        chartDataUrl,
        'PNG',
        this.generator.margin.left,
        this.generator.currentY,
        this.generator.pageWidth - this.generator.margin.left - this.generator.margin.right,
        80
      );
      this.generator.currentY += 90;
    }
  }

  private async addOverallHealthAssessment(): Promise<void> {
    // Add section heading
    this.generator.doc.setFontSize(16);
    this.generator.doc.setFont(this.generator.style.fontFamily, 'bold');
    this.generator.doc.text('Overall Health Assessment', this.generator.margin.left, this.generator.currentY);
    this.generator.currentY += 10;

    const assessmentItems: { label: string; value: string; color?: string }[] = [];

    // Calculate overall risk scores
    if (this.data.bodyScanData) {
      const bodyRisk = calculateOverallBodyScanRisk(this.data.bodyScanData);
      assessmentItems.push({
        label: 'Body Composition Risk',
        value: bodyRisk.level.charAt(0).toUpperCase() + bodyRisk.level.slice(1),
        color: bodyRisk.level === 'low' ? 'green' : bodyRisk.level === 'moderate' ? 'orange' : 'red'
      });
    }

    if (this.data.faceScanData) {
      const faceRisk = calculateOverallFaceScanRisk(this.data.faceScanData);
      assessmentItems.push({
        label: 'Cardiovascular Risk',
        value: faceRisk.level.charAt(0).toUpperCase() + faceRisk.level.slice(1),
        color: faceRisk.level === 'low' ? 'green' : faceRisk.level === 'moderate' ? 'orange' : 'red'
      });
    }

    // Add overall health status
    const overallStatus = this.calculateOverallHealthStatus();
    assessmentItems.push({
      label: 'Overall Health Status',
      value: overallStatus,
      color: overallStatus === 'Excellent' ? 'green' : 
             overallStatus === 'Good' ? 'blue' :
             overallStatus === 'Fair' ? 'orange' : 'red'
    });

    this.generator.addSection({
      title: 'Health Risk Assessment',
      content: () => {
        const assessmentData = assessmentItems.map(item => [item.label, item.value]);
        this.generator.addTable(assessmentData, ['Category', 'Risk Level']);
      }
    });
  }

  private addRecommendations(): void {
    const recommendations: string[] = [];

    if (this.data.bodyScanData && this.data.faceScanData) {
      const bodyRecommendations = getHealthRecommendations(
        calculateOverallBodyScanRisk(this.data.bodyScanData),
        'body'
      );
      const faceRecommendations = getHealthRecommendations(
        calculateOverallFaceScanRisk(this.data.faceScanData),
        'face'
      );

      recommendations.push(...bodyRecommendations, ...faceRecommendations);
    } else if (this.data.bodyScanData) {
      recommendations.push(...getHealthRecommendations(
        calculateOverallBodyScanRisk(this.data.bodyScanData),
        'body'
      ));
    } else if (this.data.faceScanData) {
      recommendations.push(...getHealthRecommendations(
        calculateOverallFaceScanRisk(this.data.faceScanData),
        'face'
      ));
    }

    if (!this.data.scanProgress.hasCompletedAllScans) {
      recommendations.unshift('Complete all required scans for a comprehensive health assessment');
    }

    this.generator.addSection({
      title: 'Personalized Recommendations',
      content: () => {
        this.generator.addBulletList(recommendations);
      }
    });
  }

  private calculateOverallHealthStatus(): string {
    if (!this.data.scanProgress.hasCompletedAllScans) {
      return 'Incomplete Assessment';
    }

    let riskScore = 0;
    let factorCount = 0;

    if (this.data.bodyScanData) {
      const bodyRisk = calculateOverallBodyScanRisk(this.data.bodyScanData);
      riskScore += bodyRisk.level === 'low' ? 1 : bodyRisk.level === 'moderate' ? 2 : 3;
      factorCount++;
    }

    if (this.data.faceScanData) {
      const faceRisk = calculateOverallFaceScanRisk(this.data.faceScanData);
      riskScore += faceRisk.level === 'low' ? 1 : faceRisk.level === 'moderate' ? 2 : 3;
      factorCount++;
    }

    const avgRisk = riskScore / factorCount;

    if (avgRisk <= 1.3) return 'Excellent';
    if (avgRisk <= 2) return 'Good';
    if (avgRisk <= 2.5) return 'Fair';
    return 'Needs Attention';
  }

  save(filename?: string): void {
    const defaultFilename = `health_assessment_${this.data.profile.name.replace(/\s+/g, '_')}_${
      this.data.generatedDate.toISOString().split('T')[0]
    }.pdf`;
    
    this.generator.save(filename || defaultFilename);
  }
}