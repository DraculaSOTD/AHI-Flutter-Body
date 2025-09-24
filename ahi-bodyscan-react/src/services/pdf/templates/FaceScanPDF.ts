import { PDFGenerator } from '../PDFGenerator';
import { FaceScanData, PDFGenerationOptions, HealthMetric } from '../types';
import { 
  calculateHeartRateRisk,
  calculateBloodPressureRisk,
  calculateOxygenSaturationRisk,
  calculateStressRisk,
  calculateOverallFaceScanRisk,
  getHealthRecommendations 
} from '../utils/riskAssessment';
import { 
  createVitalsChart,
  createRiskGaugeChart 
} from '../utils/charts';
import { AHI_LOGO_BASE64 } from '../assets/logo';

export class FaceScanPDF {
  private generator: PDFGenerator;
  private data: FaceScanData;
  private options: PDFGenerationOptions;

  constructor(data: FaceScanData, options: PDFGenerationOptions) {
    this.generator = new PDFGenerator();
    this.data = data;
    this.options = options;
  }

  async generate(): Promise<void> {
    // Add header
    this.generator.addHeader({
      logoPath: AHI_LOGO_BASE64,
      title: 'Vital Signs Analysis',
      subtitle: 'Comprehensive Health Monitoring Report',
      date: this.data.scanDate,
      reportId: `FSC-${this.data.scanDate.getTime()}`
    });

    // Add patient information section
    this.addPatientInfo();

    // Add vital signs overview
    await this.addVitalSignsOverview();

    // Add detailed vital signs analysis
    await this.addDetailedVitals();

    // Add cardiovascular health section
    this.addCardiovascularHealth();

    // Add stress and wellness section
    this.addStressWellness();

    // Add risk assessment
    if (this.options.includeRiskAssessment) {
      await this.addRiskAssessment();
    }

    // Add recommendations
    if (this.options.includeRecommendations) {
      this.addRecommendations();
    }

    // Add historical trends if available
    if (this.options.includeCharts) {
      await this.addHistoricalTrends();
    }

    // Add disclaimer
    this.addDisclaimer();
  }

  private addPatientInfo(): void {
    this.generator.addSection({
      title: 'Patient Information',
      content: () => {
        const info = [
          ['Name', this.data.userName || 'Anonymous'],
          ['Age', this.data.age ? `${this.data.age} years` : 'Not provided'],
          ['Gender', this.data.gender ? this.data.gender.charAt(0).toUpperCase() + this.data.gender.slice(1) : 'Not specified'],
          ['Scan Date', this.data.scanDate.toLocaleDateString('en-US', {
            year: 'numeric',
            month: 'long',
            day: 'numeric',
            hour: '2-digit',
            minute: '2-digit'
          })],
          ['Scan Duration', '30 seconds'],
          ['Technology', 'rPPG (Remote Photoplethysmography)']
        ];

        this.generator.addTable(info, ['Field', 'Value'], {
          headStyles: { fillColor: [106, 0, 255] },
          columnStyles: {
            0: { cellWidth: 40, fontStyle: 'bold' },
            1: { cellWidth: 'auto' }
          }
        });
      }
    });
  }

  private async addVitalSignsOverview(): Promise<void> {
    this.generator.addSection({
      title: 'Vital Signs Summary',
      content: async () => {
        const metricsPerRow = 3;
        const metricWidth = (this.generator.pageWidth - 2 * this.generator.margin.left - 20) / metricsPerRow;
        
        const metrics: HealthMetric[] = [
          {
            name: 'Heart Rate',
            value: this.data.vitals.heartRate,
            unit: 'bpm',
            riskLevel: calculateHeartRateRisk(this.data.vitals.heartRate, this.data.age),
            normalRange: { min: 60, max: 100 }
          },
          {
            name: 'Blood Pressure',
            value: `${this.data.vitals.bloodPressure.systolic}/${this.data.vitals.bloodPressure.diastolic}`,
            unit: 'mmHg',
            riskLevel: calculateBloodPressureRisk(
              this.data.vitals.bloodPressure.systolic,
              this.data.vitals.bloodPressure.diastolic
            ),
            normalRange: { min: 90, max: 120 } // Systolic
          },
          {
            name: 'O₂ Saturation',
            value: this.data.vitals.oxygenSaturation,
            unit: '%',
            riskLevel: calculateOxygenSaturationRisk(this.data.vitals.oxygenSaturation),
            normalRange: { min: 95, max: 100 }
          },
          {
            name: 'Respiratory Rate',
            value: this.data.vitals.respiratoryRate,
            unit: '/min',
            normalRange: { min: 12, max: 20 }
          },
          {
            name: 'Stress Level',
            value: this.data.vitals.stressLevel.toUpperCase(),
            unit: '',
            riskLevel: calculateStressRisk(this.data.vitals.stressLevel)
          },
          {
            name: 'HRV',
            value: this.data.vitals.heartRateVariability || 'N/A',
            unit: this.data.vitals.heartRateVariability ? 'ms' : '',
            normalRange: this.data.vitals.heartRateVariability ? { min: 20, max: 200 } : undefined
          }
        ];

        let x = this.generator.margin.left;
        let y = this.generator.currentY;
        
        metrics.forEach((metric, index) => {
          if (index > 0 && index % metricsPerRow === 0) {
            x = this.generator.margin.left;
            y += 40;
            this.generator.checkPageBreak(40);
          }
          
          this.generator.addMetric(metric, x, y, metricWidth);
          x += metricWidth + 10;
        });
        
        this.generator.currentY = y + 45;

        // Add overall health status
        const overallHealth = this.data.analysis?.overallHealth || 'good';
        const healthColor = overallHealth === 'excellent' ? [40, 167, 69] :
                           overallHealth === 'good' ? [40, 167, 69] :
                           overallHealth === 'fair' ? [255, 193, 7] :
                           [220, 53, 69];

        this.generator.addText(
          `Overall Health Status: ${overallHealth.toUpperCase()}`,
          { 
            fontSize: 14, 
            fontStyle: 'bold', 
            color: healthColor,
            align: 'center'
          }
        );
      }
    });
  }

  private async addDetailedVitals(): Promise<void> {
    this.generator.addSection({
      title: 'Detailed Vital Signs Analysis',
      content: async () => {
        if (this.options.includeCharts) {
          // Add vitals radar chart
          const chartBase64 = await createVitalsChart({
            heartRate: this.data.vitals.heartRate,
            systolic: this.data.vitals.bloodPressure.systolic,
            diastolic: this.data.vitals.bloodPressure.diastolic,
            respiratoryRate: this.data.vitals.respiratoryRate,
            oxygenSaturation: this.data.vitals.oxygenSaturation
          });

          await this.generator.addChart({
            type: 'radar',
            data: {},
            width: 400,
            height: 400
          }, this.generator.margin.left + 50, this.generator.currentY);
        }

        // Add detailed analysis table
        const detailedData = [
          [
            'Heart Rate',
            `${this.data.vitals.heartRate} bpm`,
            '60-100 bpm',
            this.getVitalStatus(this.data.vitals.heartRate, 60, 100),
            this.getVitalInterpretation('heartRate', this.data.vitals.heartRate)
          ],
          [
            'Systolic BP',
            `${this.data.vitals.bloodPressure.systolic} mmHg`,
            '<120 mmHg',
            this.getBloodPressureStatus(this.data.vitals.bloodPressure.systolic, 'systolic'),
            this.getVitalInterpretation('systolic', this.data.vitals.bloodPressure.systolic)
          ],
          [
            'Diastolic BP',
            `${this.data.vitals.bloodPressure.diastolic} mmHg`,
            '<80 mmHg',
            this.getBloodPressureStatus(this.data.vitals.bloodPressure.diastolic, 'diastolic'),
            this.getVitalInterpretation('diastolic', this.data.vitals.bloodPressure.diastolic)
          ],
          [
            'Respiratory Rate',
            `${this.data.vitals.respiratoryRate} /min`,
            '12-20 /min',
            this.getVitalStatus(this.data.vitals.respiratoryRate, 12, 20),
            this.getVitalInterpretation('respiratory', this.data.vitals.respiratoryRate)
          ],
          [
            'Oxygen Saturation',
            `${this.data.vitals.oxygenSaturation}%`,
            '95-100%',
            this.getVitalStatus(this.data.vitals.oxygenSaturation, 95, 100),
            this.getVitalInterpretation('oxygen', this.data.vitals.oxygenSaturation)
          ]
        ];

        this.generator.addTable(detailedData, ['Vital Sign', 'Value', 'Normal Range', 'Status', 'Interpretation'], {
          headStyles: { fillColor: [106, 0, 255] },
          columnStyles: {
            3: { 
              cellWidth: 25,
              halign: 'center',
              fontStyle: 'bold'
            }
          },
          willDrawCell: (data: any) => {
            if (data.column.index === 3 && data.row.index >= 0) {
              const status = data.cell.text[0].toLowerCase();
              if (status === 'normal') {
                data.cell.styles.textColor = [40, 167, 69];
              } else if (status === 'elevated' || status === 'low') {
                data.cell.styles.textColor = [255, 193, 7];
              } else {
                data.cell.styles.textColor = [220, 53, 69];
              }
            }
          }
        });
      }
    });
  }

  private addCardiovascularHealth(): void {
    this.generator.addSection({
      title: 'Cardiovascular Health Assessment',
      content: () => {
        const bpRisk = calculateBloodPressureRisk(
          this.data.vitals.bloodPressure.systolic,
          this.data.vitals.bloodPressure.diastolic
        );
        const hrRisk = calculateHeartRateRisk(this.data.vitals.heartRate, this.data.age);

        // Blood pressure classification
        const bpClassification = this.getBloodPressureClassification(
          this.data.vitals.bloodPressure.systolic,
          this.data.vitals.bloodPressure.diastolic
        );

        this.generator.addText(
          `Blood Pressure Classification: ${bpClassification}`,
          { fontSize: 12, fontStyle: 'bold' }
        );

        // Add cardiovascular metrics table
        const cardioData = [
          ['Metric', 'Value', 'Risk Level'],
          ['Blood Pressure', `${this.data.vitals.bloodPressure.systolic}/${this.data.vitals.bloodPressure.diastolic} mmHg`, bpRisk.level.toUpperCase()],
          ['Resting Heart Rate', `${this.data.vitals.heartRate} bpm`, hrRisk.level.toUpperCase()],
          ['Heart Rate Variability', this.data.vitals.heartRateVariability ? `${this.data.vitals.heartRateVariability} ms` : 'N/A', 'N/A'],
          ['Cardiovascular Fitness', this.estimateCardioFitness(), 'N/A']
        ];

        this.generator.addTable(cardioData.slice(1), cardioData[0], {
          headStyles: { fillColor: [106, 0, 255] },
          columnStyles: {
            2: { 
              cellWidth: 30,
              halign: 'center',
              fontStyle: 'bold'
            }
          }
        });

        // Add risk factors if present
        const riskFactors = this.data.analysis?.riskFactors || [];
        if (riskFactors.length > 0) {
          this.generator.currentY += 10;
          this.generator.addText(
            'Identified Cardiovascular Risk Factors:',
            { fontSize: 11, fontStyle: 'bold', color: [220, 53, 69] }
          );
          this.generator.addBulletList(riskFactors);
        }
      }
    });
  }

  private addStressWellness(): void {
    this.generator.addSection({
      title: 'Stress & Wellness Analysis',
      content: () => {
        const stressRisk = calculateStressRisk(this.data.vitals.stressLevel);
        
        // Stress level visualization
        const stressScore = this.data.vitals.stressScore || 
          (this.data.vitals.stressLevel === 'low' ? 20 :
           this.data.vitals.stressLevel === 'moderate' ? 50 : 80);

        this.generator.addText(
          `Current Stress Level: ${this.data.vitals.stressLevel.toUpperCase()}`,
          { 
            fontSize: 14, 
            fontStyle: 'bold',
            color: stressRisk && stressRisk.level === 'low' ? [40, 167, 69] :
                   stressRisk && stressRisk.level === 'moderate' ? [255, 193, 7] :
                   [220, 53, 69]
          }
        );

        // Stress indicators table
        const stressIndicators = [
          ['Heart Rate Variability', this.data.vitals.heartRateVariability ? 'Measured' : 'Not Available', this.getHRVInterpretation()],
          ['Respiratory Pattern', 'Regular', 'Normal breathing rhythm'],
          ['Stress Score', `${stressScore}/100`, stressRisk ? stressRisk.description : 'Stress level assessment']
        ];

        this.generator.addTable(stressIndicators, ['Indicator', 'Status', 'Interpretation'], {
          headStyles: { fillColor: [106, 0, 255] }
        });

        // Stress management recommendations
        this.generator.currentY += 10;
        this.generator.addText(
          'Stress Management Recommendations:',
          { fontSize: 11, fontStyle: 'bold' }
        );

        const stressRecommendations = this.getStressRecommendations(this.data.vitals.stressLevel);
        this.generator.addBulletList(stressRecommendations);
      }
    });
  }

  private async addRiskAssessment(): Promise<void> {
    this.generator.addSection({
      title: 'Comprehensive Health Risk Assessment',
      content: async () => {
        const overallRisk = calculateOverallFaceScanRisk(this.data);
        
        // Add overall risk indicator
        this.generator.addRiskIndicator(
          overallRisk,
          this.generator.margin.left,
          this.generator.currentY,
          this.generator.pageWidth - 2 * this.generator.margin.left
        );

        if (this.options.includeCharts) {
          // Add risk gauge chart
          this.generator.currentY += 10;
          const gaugeChart = await createRiskGaugeChart(
            overallRisk.score,
            overallRisk.level
          );
          
          await this.generator.addChart({
            type: 'doughnut',
            data: {},
            width: 300,
            height: 200
          }, this.generator.margin.left + 80, this.generator.currentY);
        }

        // Individual risk factors assessment
        const riskFactors = [
          {
            factor: 'Heart Rate',
            value: `${this.data.vitals.heartRate} bpm`,
            risk: calculateHeartRateRisk(this.data.vitals.heartRate, this.data.age)
          },
          {
            factor: 'Blood Pressure',
            value: `${this.data.vitals.bloodPressure.systolic}/${this.data.vitals.bloodPressure.diastolic}`,
            risk: calculateBloodPressureRisk(
              this.data.vitals.bloodPressure.systolic,
              this.data.vitals.bloodPressure.diastolic
            )
          },
          {
            factor: 'Oxygen Saturation',
            value: `${this.data.vitals.oxygenSaturation}%`,
            risk: calculateOxygenSaturationRisk(this.data.vitals.oxygenSaturation)
          },
          {
            factor: 'Stress Level',
            value: this.data.vitals.stressLevel,
            risk: calculateStressRisk(this.data.vitals.stressLevel)
          }
        ];

        const riskData = riskFactors.map(rf => [
          rf.factor,
          rf.value,
          rf.risk.level.toUpperCase(),
          (rf.risk.score * 100).toFixed(0) + '%'
        ]);

        this.generator.addTable(riskData, ['Risk Factor', 'Value', 'Risk Level', 'Score'], {
          headStyles: { fillColor: [106, 0, 255] },
          columnStyles: {
            2: { 
              cellWidth: 30,
              halign: 'center',
              fontStyle: 'bold'
            },
            3: {
              cellWidth: 20,
              halign: 'center'
            }
          }
        });
      }
    });
  }

  private addRecommendations(): void {
    const overallRisk = calculateOverallFaceScanRisk(this.data);
    const recommendations = getHealthRecommendations(overallRisk, 'face');

    this.generator.addSection({
      title: 'Personalized Health Recommendations',
      content: () => {
        this.generator.addText(
          'Based on your vital signs analysis, here are personalized recommendations:',
          { fontSize: 11, color: [108, 117, 125] }
        );

        this.generator.addBulletList(recommendations);

        // Add specific recommendations based on vitals
        if (this.data.vitals.bloodPressure.systolic > 130) {
          this.generator.currentY += 10;
          this.generator.addText(
            'Blood Pressure Management:',
            { fontSize: 12, fontStyle: 'bold', color: [220, 53, 69] }
          );
          this.generator.addBulletList([
            'Reduce sodium intake to less than 2,300mg per day',
            'Increase physical activity to at least 150 minutes per week',
            'Maintain a healthy weight',
            'Limit alcohol consumption',
            'Monitor blood pressure daily at home'
          ]);
        }

        if (this.data.vitals.stressLevel === 'high') {
          this.generator.currentY += 10;
          this.generator.addText(
            'Stress Reduction Techniques:',
            { fontSize: 12, fontStyle: 'bold', color: [255, 193, 7] }
          );
          this.generator.addBulletList([
            'Practice deep breathing exercises for 5-10 minutes daily',
            'Try progressive muscle relaxation before bed',
            'Consider mindfulness meditation apps',
            'Maintain regular sleep schedule (7-9 hours)',
            'Engage in regular physical activity'
          ]);
        }
      }
    });
  }

  private async addHistoricalTrends(): Promise<void> {
    this.generator.addSection({
      title: 'Health Monitoring Progress',
      content: () => {
        this.generator.addText(
          'Regular vital signs monitoring helps track your cardiovascular health over time. Continue scanning weekly to build comprehensive trend data and identify patterns.',
          { fontSize: 11, color: [108, 117, 125], align: 'center' }
        );

        // Add monitoring schedule recommendation
        this.generator.currentY += 10;
        const schedule = [
          ['Condition', 'Recommended Frequency'],
          ['Normal vital signs', 'Weekly'],
          ['Elevated blood pressure', 'Daily'],
          ['High stress levels', '2-3 times per week'],
          ['Post-exercise recovery', 'Before and after workouts']
        ];

        this.generator.addTable(schedule.slice(1), schedule[0], {
          headStyles: { fillColor: [106, 0, 255] }
        });
      }
    });
  }

  private addDisclaimer(): void {
    this.generator.checkPageBreak(40);
    
    this.generator.doc.setDrawColor(220, 53, 69);
    this.generator.doc.setLineWidth(0.5);
    this.generator.doc.line(
      this.generator.margin.left,
      this.generator.currentY,
      this.generator.pageWidth - this.generator.margin.right,
      this.generator.currentY
    );
    
    this.generator.currentY += 5;
    
    this.generator.addText(
      'Medical Disclaimer',
      { fontSize: 12, fontStyle: 'bold', color: [220, 53, 69] }
    );
    
    this.generator.addText(
      'This report is for informational purposes only and should not be used as a substitute for professional medical advice, diagnosis, or treatment. The vital signs measurements are obtained using camera-based technology and may not be as accurate as medical-grade devices. Always consult with a qualified healthcare professional for medical concerns, especially if you experience symptoms such as chest pain, shortness of breath, dizziness, or irregular heartbeat. In case of medical emergency, call emergency services immediately.',
      { fontSize: 9, color: [108, 117, 125] }
    );

    this.generator.currentY += 10;
    
    this.generator.addText(
      'Technology Note',
      { fontSize: 10, fontStyle: 'bold' }
    );
    
    this.generator.addText(
      'Measurements obtained using rPPG (remote photoplethysmography) technology. Results may vary based on lighting conditions, camera quality, and user movement. For best results, ensure good lighting and remain still during scans.',
      { fontSize: 9, color: [108, 117, 125] }
    );
  }

  // Helper methods
  private getVitalStatus(value: number, min: number, max: number): string {
    if (value < min) return 'Low';
    if (value > max) return 'Elevated';
    return 'Normal';
  }

  private getBloodPressureStatus(value: number, type: 'systolic' | 'diastolic'): string {
    if (type === 'systolic') {
      if (value < 90) return 'Low';
      if (value < 120) return 'Normal';
      if (value < 130) return 'Elevated';
      if (value < 140) return 'High Stage 1';
      return 'High Stage 2';
    } else {
      if (value < 60) return 'Low';
      if (value < 80) return 'Normal';
      if (value < 90) return 'High Stage 1';
      return 'High Stage 2';
    }
  }

  private getBloodPressureClassification(systolic: number, diastolic: number): string {
    if (systolic < 120 && diastolic < 80) return 'Normal';
    if (systolic < 130 && diastolic < 80) return 'Elevated';
    if (systolic < 140 || diastolic < 90) return 'Stage 1 Hypertension';
    if (systolic >= 140 || diastolic >= 90) return 'Stage 2 Hypertension';
    return 'Unknown';
  }

  private getVitalInterpretation(type: string, value: number): string {
    switch (type) {
      case 'heartRate':
        if (value < 60) return 'Bradycardia - may indicate fitness or medical condition';
        if (value > 100) return 'Tachycardia - may indicate stress or medical condition';
        return 'Normal resting heart rate';
      
      case 'systolic':
        if (value < 90) return 'Hypotension - may cause dizziness';
        if (value < 120) return 'Optimal blood pressure';
        if (value < 130) return 'Slightly elevated - lifestyle changes recommended';
        return 'Hypertension - medical consultation advised';
      
      case 'diastolic':
        if (value < 60) return 'Low diastolic pressure';
        if (value < 80) return 'Normal diastolic pressure';
        return 'Elevated diastolic pressure';
      
      case 'respiratory':
        if (value < 12) return 'Below normal - may indicate respiratory depression';
        if (value > 20) return 'Above normal - may indicate respiratory distress';
        return 'Normal breathing rate';
      
      case 'oxygen':
        if (value < 90) return 'Hypoxemia - seek immediate medical attention';
        if (value < 95) return 'Below normal - monitor closely';
        return 'Normal oxygen saturation';
      
      default:
        return 'Within expected range';
    }
  }

  private getHRVInterpretation(): string {
    if (!this.data.vitals.heartRateVariability) {
      return 'HRV measurement not available';
    }
    
    const hrv = this.data.vitals.heartRateVariability;
    if (hrv < 20) return 'Low HRV - may indicate stress or fatigue';
    if (hrv > 100) return 'High HRV - indicates good cardiovascular fitness';
    return 'Normal HRV - healthy autonomic function';
  }

  private estimateCardioFitness(): string {
    const hr = this.data.vitals.heartRate;
    const age = this.data.age || 40;
    const maxHR = 220 - age;
    const hrReserve = maxHR - hr;
    
    if (hrReserve > 100) return 'Excellent';
    if (hrReserve > 80) return 'Good';
    if (hrReserve > 60) return 'Fair';
    return 'Needs Improvement';
  }

  private getStressRecommendations(level: 'low' | 'normal' | 'moderate' | 'high'): string[] {
    switch (level) {
      case 'low':
        return [
          'Maintain current stress management practices',
          'Continue regular physical activity',
          'Ensure adequate sleep (7-9 hours)'
        ];
      
      case 'normal':
        return [
          'Keep up healthy lifestyle habits',
          'Continue balanced work-life integration',
          'Maintain regular exercise routine',
          'Practice mindfulness occasionally'
        ];
      
      case 'moderate':
        return [
          'Practice daily relaxation techniques',
          'Consider yoga or tai chi',
          'Limit caffeine intake',
          'Take regular breaks during work',
          'Spend time in nature'
        ];
      
      case 'high':
        return [
          'Seek stress management counseling',
          'Practice meditation daily',
          'Reduce workload if possible',
          'Prioritize sleep hygiene',
          'Consider professional support',
          'Avoid alcohol and stimulants'
        ];
    }
  }

  save(filename?: string): void {
    const defaultFilename = `vital_signs_report_${this.data.scanDate.getTime()}.pdf`;
    this.generator.save(filename || defaultFilename);
  }
}