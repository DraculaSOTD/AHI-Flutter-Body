import { PDFGenerator } from '../PDFGenerator';
import { BodyScanData, PDFGenerationOptions, HealthMetric } from '../types';
import { 
  calculateBMIRisk, 
  calculateBodyFatRisk, 
  calculateOverallBodyScanRisk,
  getHealthRecommendations 
} from '../utils/riskAssessment';
import { 
  createBodyCompositionChart, 
  createMeasurementsChart,
  createTrendChart 
} from '../utils/charts';
import { AHI_LOGO_BASE64 } from '../assets/logo';

export class BodyScanPDF {
  private generator: PDFGenerator;
  private data: BodyScanData;
  private options: PDFGenerationOptions;

  constructor(data: BodyScanData, options: PDFGenerationOptions) {
    this.generator = new PDFGenerator();
    this.data = data;
    this.options = options;
  }

  async generate(): Promise<void> {
    // Add header
    this.generator.addHeader({
      logoPath: AHI_LOGO_BASE64,
      title: 'Body Composition Analysis',
      subtitle: 'Comprehensive Health Assessment Report',
      date: this.data.scanDate,
      reportId: `BSC-${this.data.scanDate.getTime()}`
    });

    // Add patient information section
    this.addPatientInfo();

    // Add key metrics section
    await this.addKeyMetrics();

    // Add body composition analysis
    await this.addBodyComposition();

    // Add measurements section
    await this.addMeasurements();

    // Add risk assessment
    if (this.options.includeRiskAssessment) {
      this.addRiskAssessment();
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
          ['Height', `${this.data.height} cm`],
          ['Weight', `${this.data.weight} kg`],
          ['Scan Date', this.data.scanDate.toLocaleDateString('en-US', {
            year: 'numeric',
            month: 'long',
            day: 'numeric',
            hour: '2-digit',
            minute: '2-digit'
          })]
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

  private async addKeyMetrics(): Promise<void> {
    this.generator.addSection({
      title: 'Key Health Metrics',
      content: async () => {
        const metricsPerRow = 3;
        const metricWidth = (this.generator.pageWidth - 2 * this.generator.margin.left - 20) / metricsPerRow;
        
        const metrics: HealthMetric[] = [
          {
            name: 'Body Fat',
            value: this.data.measurements.bodyFat,
            unit: '%',
            riskLevel: calculateBodyFatRisk(this.data.measurements.bodyFat, this.data.gender),
            normalRange: this.data.gender === 'female' 
              ? { min: 21, max: 33 } 
              : { min: 14, max: 25 }
          },
          {
            name: 'BMI',
            value: this.data.measurements.bmi.toFixed(1),
            unit: '',
            riskLevel: calculateBMIRisk(this.data.measurements.bmi),
            normalRange: { min: 18.5, max: 24.9 }
          },
          {
            name: 'Muscle Mass',
            value: this.data.measurements.muscleMass,
            unit: 'kg',
            normalRange: this.data.gender === 'female'
              ? { min: 36, max: 42 }
              : { min: 49, max: 59 }
          },
          {
            name: 'Visceral Fat',
            value: this.data.measurements.visceralFat || 'N/A',
            unit: this.data.measurements.visceralFat ? '' : '',
            normalRange: { min: 1, max: 12 }
          },
          {
            name: 'Metabolic Age',
            value: this.data.analysis?.metabolicAge || 'N/A',
            unit: this.data.analysis?.metabolicAge ? 'years' : ''
          },
          {
            name: 'Body Type',
            value: this.data.analysis?.bodyType || 'Standard',
            unit: ''
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
      }
    });
  }

  private async addBodyComposition(): Promise<void> {
    this.generator.addSection({
      title: 'Body Composition Analysis',
      content: async () => {
        if (this.options.includeCharts) {
          // Add body composition chart
          const chartBase64 = await createBodyCompositionChart(
            this.data.measurements.bodyFat,
            this.data.measurements.muscleMass,
            this.data.measurements.fatFreeMass
          );

          await this.generator.addChart({
            type: 'doughnut',
            data: {},
            width: 400,
            height: 300
          }, this.generator.margin.left + 50, this.generator.currentY);
        }

        // Add detailed composition breakdown
        const compositionData = [
          ['Body Fat', `${this.data.measurements.bodyFat}%`, this.getStatusText(this.data.measurements.bodyFat, 14, 25)],
          ['Muscle Mass', `${this.data.measurements.muscleMass} kg`, this.getStatusText(this.data.measurements.muscleMass, 40, 60)],
          ['Fat-Free Mass', `${this.data.measurements.fatFreeMass || 'N/A'} kg`, 'N/A'],
          ['Body Water', 'N/A', 'N/A']
        ];

        this.generator.addTable(compositionData, ['Component', 'Value', 'Status'], {
          headStyles: { fillColor: [106, 0, 255] },
          columnStyles: {
            2: { 
              cellWidth: 40,
              halign: 'center'
            }
          },
          bodyStyles: {
            fontSize: 10
          },
          styles: {
            halign: 'center'
          }
        });
      }
    });
  }

  private async addMeasurements(): Promise<void> {
    this.generator.addSection({
      title: 'Body Measurements',
      content: async () => {
        const measurements = [
          ['Chest', `${this.data.measurements.chest} cm`],
          ['Waist', `${this.data.measurements.waist} cm`],
          ['Hip', `${this.data.measurements.hip} cm`],
          ['Waist-to-Hip Ratio', (this.data.measurements.waist / this.data.measurements.hip).toFixed(2)]
        ];

        if (this.data.measurements.thigh) {
          measurements.push(['Thigh', `${this.data.measurements.thigh} cm`]);
        }
        if (this.data.measurements.inseam) {
          measurements.push(['Inseam', `${this.data.measurements.inseam} cm`]);
        }

        this.generator.addTable(measurements, ['Measurement', 'Value'], {
          headStyles: { fillColor: [106, 0, 255] },
          columnStyles: {
            0: { cellWidth: 60, fontStyle: 'bold' },
            1: { cellWidth: 'auto', halign: 'center' }
          }
        });

        if (this.options.includeCharts) {
          this.generator.currentY += 10;
          const chartBase64 = await createMeasurementsChart(this.data.measurements);
          await this.generator.addChart({
            type: 'bar',
            data: {},
            width: 500,
            height: 300
          }, this.generator.margin.left, this.generator.currentY);
        }
      }
    });
  }

  private addRiskAssessment(): void {
    this.generator.addSection({
      title: 'Health Risk Assessment',
      content: () => {
        const overallRisk = calculateOverallBodyScanRisk(this.data);
        
        // Add overall risk indicator
        this.generator.addRiskIndicator(
          overallRisk,
          this.generator.margin.left,
          this.generator.currentY,
          this.generator.pageWidth - 2 * this.generator.margin.left
        );

        this.generator.currentY += 10;

        // Add individual risk factors
        const riskFactors = [
          {
            factor: 'BMI',
            value: this.data.measurements.bmi,
            risk: calculateBMIRisk(this.data.measurements.bmi)
          },
          {
            factor: 'Body Fat Percentage',
            value: this.data.measurements.bodyFat,
            risk: calculateBodyFatRisk(this.data.measurements.bodyFat, this.data.gender)
          }
        ];

        const riskData = riskFactors.map(rf => [
          rf.factor,
          `${rf.value}`,
          rf.risk.level.toUpperCase(),
          rf.risk.description
        ]);

        this.generator.addTable(riskData, ['Risk Factor', 'Value', 'Risk Level', 'Description'], {
          headStyles: { fillColor: [106, 0, 255] },
          columnStyles: {
            0: { fontStyle: 'bold' },
            2: { 
              cellWidth: 30,
              halign: 'center',
              fontStyle: 'bold'
            },
            3: { cellWidth: 'auto' }
          },
          willDrawCell: (data: any) => {
            if (data.column.index === 2 && data.row.index >= 0) {
              const level = data.cell.text[0].toLowerCase();
              if (level === 'low') {
                data.cell.styles.textColor = [40, 167, 69];
              } else if (level === 'moderate') {
                data.cell.styles.textColor = [255, 193, 7];
              } else if (level === 'high' || level === 'critical') {
                data.cell.styles.textColor = [220, 53, 69];
              }
            }
          }
        });
      }
    });
  }

  private addRecommendations(): void {
    const overallRisk = calculateOverallBodyScanRisk(this.data);
    const recommendations = getHealthRecommendations(overallRisk, 'body');

    this.generator.addSection({
      title: 'Personalized Recommendations',
      content: () => {
        this.generator.addText(
          'Based on your body composition analysis, here are personalized recommendations to improve your health:',
          { fontSize: 11, color: [108, 117, 125] }
        );

        this.generator.addBulletList(recommendations);

        // Add specific recommendations based on metrics
        if (this.data.measurements.bodyFat > 30) {
          this.generator.addText(
            'Additional Focus: Body Fat Reduction',
            { fontSize: 12, fontStyle: 'bold', color: [220, 53, 69] }
          );
          this.generator.addBulletList([
            'Focus on creating a caloric deficit through diet and exercise',
            'Incorporate strength training to preserve muscle mass',
            'Consider working with a registered dietitian'
          ]);
        }

        if (this.data.measurements.muscleMass < 40) {
          this.generator.addText(
            'Additional Focus: Muscle Building',
            { fontSize: 12, fontStyle: 'bold', color: [255, 193, 7] }
          );
          this.generator.addBulletList([
            'Increase protein intake to 1.6-2.2g per kg body weight',
            'Implement progressive resistance training program',
            'Ensure adequate rest and recovery between workouts'
          ]);
        }
      }
    });
  }

  private async addHistoricalTrends(): Promise<void> {
    // This would typically fetch historical data from a database
    // For now, we'll show a placeholder
    this.generator.addSection({
      title: 'Progress Tracking',
      content: () => {
        this.generator.addText(
          'Regular monitoring helps track your progress over time. Continue scanning monthly to build your trend data.',
          { fontSize: 11, color: [108, 117, 125], align: 'center' }
        );
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
      'This report is for informational purposes only and should not be considered medical advice. The measurements and recommendations provided are based on general health guidelines and may not be suitable for everyone. Please consult with a qualified healthcare professional before making any significant changes to your diet, exercise routine, or lifestyle. If you have any pre-existing health conditions or concerns, seek medical advice before acting on any recommendations in this report.',
      { fontSize: 9, color: [108, 117, 125] }
    );
  }

  private getStatusText(value: number, min: number, max: number): string {
    if (value < min) return 'Below Normal';
    if (value > max) return 'Above Normal';
    return 'Normal';
  }

  save(filename?: string): void {
    const defaultFilename = `body_scan_report_${this.data.scanDate.getTime()}.pdf`;
    this.generator.save(filename || defaultFilename);
  }
}