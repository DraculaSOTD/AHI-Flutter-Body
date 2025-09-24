import { jsPDF } from 'jspdf';
import { UserOptions } from 'jspdf-autotable';

export interface PDFMetadata {
  title: string;
  subject?: string;
  author: string;
  keywords?: string[];
  creator: string;
}

export interface PDFHeaderConfig {
  logoPath?: string;
  title: string;
  subtitle?: string;
  date: Date;
  reportId?: string;
}

export interface RiskLevel {
  level: 'low' | 'moderate' | 'high' | 'critical';
  score: number;
  color: string;
  description: string;
}

export interface HealthMetric {
  name: string;
  value: number | string;
  unit?: string;
  normalRange?: {
    min: number;
    max: number;
  };
  riskLevel?: RiskLevel;
  trend?: 'improving' | 'stable' | 'declining';
}

export interface BodyScanData {
  scanDate: Date;
  userId?: string;
  userName?: string;
  height: number; // cm
  weight: number; // kg
  age?: number;
  gender?: 'male' | 'female';
  
  measurements: {
    bodyFat: number;
    muscleMass: number;
    bmi: number;
    waist: number;
    chest: number;
    hip: number;
    thigh?: number;
    inseam?: number;
    visceralFat?: number;
    fatFreeMass?: number;
  };
  
  analysis?: {
    bodyType?: string;
    metabolicAge?: number;
    recommendations?: string[];
  };
}

export interface FaceScanData {
  scanDate: Date;
  userId?: string;
  userName?: string;
  age?: number;
  gender?: 'male' | 'female';
  
  vitals: {
    heartRate: number;
    heartRateVariability?: number;
    bloodPressure: {
      systolic: number;
      diastolic: number;
    };
    respiratoryRate: number;
    oxygenSaturation: number;
    stressLevel: 'low' | 'normal' | 'moderate' | 'high';
    stressScore?: number;
  };
  
  analysis?: {
    overallHealth?: 'excellent' | 'good' | 'fair' | 'poor';
    riskFactors?: string[];
    recommendations?: string[];
  };
}

export interface ChartConfig {
  type: 'bar' | 'line' | 'radar' | 'doughnut';
  data: any;
  options?: any;
  width: number;
  height: number;
}

export interface PDFGenerationOptions {
  metadata?: PDFMetadata;
  headerConfig: PDFHeaderConfig;
  includeCharts?: boolean;
  includeRecommendations?: boolean;
  includeRiskAssessment?: boolean;
  language?: 'en'; // Single language support as requested
}

export interface PDFStyle {
  primaryColor: string;
  secondaryColor: string;
  successColor: string;
  warningColor: string;
  dangerColor: string;
  textColor: string;
  lightGray: string;
  fontFamily: string;
}

export interface TableStyle {
  startY?: number;
  head?: string[][];
  body?: any[][];
  theme?: 'striped' | 'grid' | 'plain';
  headStyles?: any;
  bodyStyles?: any;
  columnStyles?: any;
  alternateRowStyles?: any;
  margin?: { left: number; right: number };
  styles?: any;
  didDrawCell?: any;
  willDrawCell?: any;
}

export interface PDFSection {
  title: string;
  content: () => void;
  pageBreakBefore?: boolean;
}

export interface PDFGenerator {
  doc: jsPDF;
  currentY: number;
  pageWidth: number;
  pageHeight: number;
  margin: {
    left: number;
    right: number;
    top: number;
    bottom: number;
  };
  style: PDFStyle;
  
  addHeader(config: PDFHeaderConfig): void;
  addSection(section: PDFSection): void;
  addMetric(metric: HealthMetric, x: number, y: number, width: number): number;
  addChart(config: ChartConfig, x: number, y: number): Promise<number>;
  addTable(data: any[][], headers: string[], options?: TableStyle): void;
  addRiskIndicator(risk: RiskLevel, x: number, y: number, width: number): number;
  checkPageBreak(requiredSpace: number): void;
  save(filename: string): void;
}