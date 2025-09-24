import { jsPDF } from 'jspdf';
import autoTable from 'jspdf-autotable';
import { 
  PDFGenerator as IPDFGenerator,
  PDFHeaderConfig,
  PDFSection,
  HealthMetric,
  ChartConfig,
  TableStyle,
  RiskLevel,
  PDFStyle
} from './types';
import { PDF_COLORS, getRiskColor } from './utils/colors';

export class PDFGenerator implements IPDFGenerator {
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

  constructor() {
    this.doc = new jsPDF({
      orientation: 'portrait',
      unit: 'mm',
      format: 'a4'
    });

    this.pageWidth = this.doc.internal.pageSize.getWidth();
    this.pageHeight = this.doc.internal.pageSize.getHeight();
    this.margin = {
      left: 20,
      right: 20,
      top: 20,
      bottom: 20
    };
    this.currentY = this.margin.top;

    this.style = {
      primaryColor: '#6A00FF',
      secondaryColor: '#007BFF',
      successColor: '#28A745',
      warningColor: '#FFC107',
      dangerColor: '#DC3545',
      textColor: '#212529',
      lightGray: '#F8F9FA',
      fontFamily: 'helvetica'
    };

    // Set default font
    this.doc.setFont(this.style.fontFamily);
  }

  addHeader(config: PDFHeaderConfig): void {
    const headerHeight = 40;
    
    // Add background
    this.doc.setFillColor(PDF_COLORS.gray[100][0], PDF_COLORS.gray[100][1], PDF_COLORS.gray[100][2]);
    this.doc.rect(0, 0, this.pageWidth, headerHeight, 'F');

    // Add logo if provided
    if (config.logoPath) {
      try {
        this.doc.addImage(config.logoPath, 'PNG', this.margin.left, 10, 30, 20);
      } catch (error) {
        console.error('Failed to add logo:', error);
      }
    }

    // Add title and subtitle
    const textX = config.logoPath ? this.margin.left + 40 : this.margin.left;
    
    this.doc.setTextColor(PDF_COLORS.primary[0], PDF_COLORS.primary[1], PDF_COLORS.primary[2]);
    this.doc.setFontSize(20);
    this.doc.setFont(this.style.fontFamily, 'bold');
    this.doc.text(config.title, textX, 18);

    if (config.subtitle) {
      this.doc.setTextColor(PDF_COLORS.gray[600][0], PDF_COLORS.gray[600][1], PDF_COLORS.gray[600][2]);
      this.doc.setFontSize(12);
      this.doc.setFont(this.style.fontFamily, 'normal');
      this.doc.text(config.subtitle, textX, 26);
    }

    // Add date and report ID on the right
    this.doc.setTextColor(PDF_COLORS.gray[700][0], PDF_COLORS.gray[700][1], PDF_COLORS.gray[700][2]);
    this.doc.setFontSize(10);
    this.doc.setFont(this.style.fontFamily, 'normal');
    
    const dateText = config.date.toLocaleDateString('en-US', {
      year: 'numeric',
      month: 'long',
      day: 'numeric'
    });
    this.doc.text(dateText, this.pageWidth - this.margin.right, 18, { align: 'right' });

    if (config.reportId) {
      this.doc.text(`Report ID: ${config.reportId}`, this.pageWidth - this.margin.right, 26, { align: 'right' });
    }

    this.currentY = headerHeight + 10;
  }

  addSection(section: PDFSection): void {
    if (section.pageBreakBefore) {
      this.doc.addPage();
      this.currentY = this.margin.top;
    }

    // Check if we need a page break for the title
    this.checkPageBreak(15);

    // Add section title
    this.doc.setTextColor(PDF_COLORS.dark[0], PDF_COLORS.dark[1], PDF_COLORS.dark[2]);
    this.doc.setFontSize(16);
    this.doc.setFont(this.style.fontFamily, 'bold');
    this.doc.text(section.title, this.margin.left, this.currentY);
    
    // Add underline
    this.doc.setDrawColor(PDF_COLORS.primary[0], PDF_COLORS.primary[1], PDF_COLORS.primary[2]);
    this.doc.setLineWidth(0.5);
    this.doc.line(
      this.margin.left,
      this.currentY + 2,
      this.margin.left + 50,
      this.currentY + 2
    );

    this.currentY += 10;

    // Execute section content
    section.content();
  }

  addMetric(metric: HealthMetric, x: number, y: number, width: number): number {
    const height = 35;
    const padding = 5;

    // Background
    this.doc.setFillColor(PDF_COLORS.gray[100][0], PDF_COLORS.gray[100][1], PDF_COLORS.gray[100][2]);
    this.doc.roundedRect(x, y, width, height, 3, 3, 'F');

    // Value
    this.doc.setTextColor(PDF_COLORS.dark[0], PDF_COLORS.dark[1], PDF_COLORS.dark[2]);
    this.doc.setFontSize(18);
    this.doc.setFont(this.style.fontFamily, 'bold');
    
    const valueText = `${metric.value}${metric.unit || ''}`;
    this.doc.text(valueText, x + width / 2, y + 15, { align: 'center' });

    // Name
    this.doc.setTextColor(PDF_COLORS.gray[600][0], PDF_COLORS.gray[600][1], PDF_COLORS.gray[600][2]);
    this.doc.setFontSize(10);
    this.doc.setFont(this.style.fontFamily, 'normal');
    this.doc.text(metric.name, x + width / 2, y + 25, { align: 'center' });

    // Risk indicator
    if (metric.riskLevel) {
      const indicatorSize = 4;
      const color = getRiskColor(metric.riskLevel.level);
      this.doc.setFillColor(color[0], color[1], color[2]);
      this.doc.circle(x + width - indicatorSize - 2, y + indicatorSize + 2, indicatorSize, 'F');
    }

    // Trend indicator
    if (metric.trend) {
      this.doc.setFontSize(12);
      const trendSymbol = metric.trend === 'improving' ? '↑' : 
                         metric.trend === 'declining' ? '↓' : '→';
      const trendColor = metric.trend === 'improving' ? PDF_COLORS.success :
                        metric.trend === 'declining' ? PDF_COLORS.danger :
                        PDF_COLORS.gray[500];
      this.doc.setTextColor(trendColor[0], trendColor[1], trendColor[2]);
      this.doc.text(trendSymbol, x + 5, y + 10);
    }

    return height + padding;
  }

  async addChart(config: ChartConfig, x: number, y: number): Promise<number> {
    try {
      // Import chart creation dynamically
      const { createChart } = await import('./utils/charts');
      const base64Image = await createChart(config);

      // Add the chart image
      const imgWidth = config.width * 0.264583; // Convert pixels to mm
      const imgHeight = config.height * 0.264583;

      this.checkPageBreak(imgHeight);
      
      this.doc.addImage(
        base64Image,
        'PNG',
        x,
        y || this.currentY,
        imgWidth,
        imgHeight
      );

      return imgHeight + 10;
    } catch (error) {
      console.error('Failed to add chart:', error);
      return 0;
    }
  }

  addTable(data: any[][], headers: string[], options?: TableStyle): void {
    const defaultOptions: any = {
      startY: this.currentY,
      head: [headers],
      body: data,
      theme: 'grid',
      headStyles: {
        fillColor: PDF_COLORS.primary,
        textColor: PDF_COLORS.white,
        fontSize: 11,
        fontStyle: 'bold'
      },
      bodyStyles: {
        fontSize: 10,
        cellPadding: 5
      },
      alternateRowStyles: {
        fillColor: PDF_COLORS.gray[100]
      },
      margin: { left: this.margin.left, right: this.margin.right },
      ...options
    };

    autoTable(this.doc, defaultOptions);
    
    // Update current Y position
    this.currentY = (this.doc as any).lastAutoTable.finalY + 10;
  }

  addRiskIndicator(risk: RiskLevel, x: number, y: number, width: number): number {
    const height = 60;
    const padding = 10;

    // Background with risk color border
    const riskColor = getRiskColor(risk.level);
    this.doc.setDrawColor(riskColor[0], riskColor[1], riskColor[2]);
    this.doc.setLineWidth(2);
    this.doc.setFillColor(PDF_COLORS.white[0], PDF_COLORS.white[1], PDF_COLORS.white[2]);
    this.doc.roundedRect(x, y, width, height, 5, 5, 'FD');

    // Risk level badge
    this.doc.setFillColor(riskColor[0], riskColor[1], riskColor[2]);
    this.doc.roundedRect(x + padding, y + padding, width - 2 * padding, 20, 3, 3, 'F');

    // Risk level text
    this.doc.setTextColor(PDF_COLORS.white[0], PDF_COLORS.white[1], PDF_COLORS.white[2]);
    this.doc.setFontSize(12);
    this.doc.setFont(this.style.fontFamily, 'bold');
    const levelText = risk.level.toUpperCase() + ' RISK';
    this.doc.text(levelText, x + width / 2, y + padding + 13, { align: 'center' });

    // Risk score
    this.doc.setTextColor(PDF_COLORS.dark[0], PDF_COLORS.dark[1], PDF_COLORS.dark[2]);
    this.doc.setFontSize(10);
    this.doc.setFont(this.style.fontFamily, 'normal');
    const scoreText = `Score: ${(risk.score * 100).toFixed(0)}%`;
    this.doc.text(scoreText, x + padding, y + padding + 30);

    // Description
    this.doc.setFontSize(9);
    const lines = this.doc.splitTextToSize(risk.description, width - 2 * padding);
    this.doc.text(lines, x + padding, y + padding + 40);

    return height + padding;
  }

  checkPageBreak(requiredSpace: number): void {
    if (this.currentY + requiredSpace > this.pageHeight - this.margin.bottom) {
      this.doc.addPage();
      this.currentY = this.margin.top;
    }
  }

  addText(text: string, options?: {
    fontSize?: number;
    fontStyle?: 'normal' | 'bold' | 'italic';
    color?: number[];
    align?: 'left' | 'center' | 'right';
    maxWidth?: number;
  }): void {
    const {
      fontSize = 11,
      fontStyle = 'normal',
      color = PDF_COLORS.dark,
      align = 'left',
      maxWidth = this.pageWidth - this.margin.left - this.margin.right
    } = options || {};

    this.doc.setFontSize(fontSize);
    this.doc.setFont(this.style.fontFamily, fontStyle);
    this.doc.setTextColor(color[0], color[1], color[2]);

    const lines = this.doc.splitTextToSize(text, maxWidth);
    const lineHeight = fontSize * 0.5;
    
    this.checkPageBreak(lines.length * lineHeight);

    const x = align === 'center' ? this.pageWidth / 2 :
              align === 'right' ? this.pageWidth - this.margin.right :
              this.margin.left;

    this.doc.text(lines, x, this.currentY, { align });
    this.currentY += lines.length * lineHeight + 5;
  }

  addBulletList(items: string[]): void {
    const bulletSize = 2;
    const indent = 10;
    const lineHeight = 5;

    items.forEach(item => {
      this.checkPageBreak(10);

      // Bullet point
      this.doc.setFillColor(PDF_COLORS.primary[0], PDF_COLORS.primary[1], PDF_COLORS.primary[2]);
      this.doc.circle(this.margin.left + bulletSize, this.currentY - bulletSize, bulletSize / 2, 'F');

      // Text
      this.doc.setTextColor(PDF_COLORS.dark[0], PDF_COLORS.dark[1], PDF_COLORS.dark[2]);
      this.doc.setFontSize(11);
      this.doc.setFont(this.style.fontFamily, 'normal');
      
      const lines = this.doc.splitTextToSize(
        item, 
        this.pageWidth - this.margin.left - this.margin.right - indent
      );
      
      this.doc.text(lines, this.margin.left + indent, this.currentY);
      this.currentY += lines.length * lineHeight + 3;
    });

    this.currentY += 5;
  }

  save(filename: string): void {
    // Add footer to all pages
    const pageCount = this.doc.getNumberOfPages();
    
    for (let i = 1; i <= pageCount; i++) {
      this.doc.setPage(i);
      
      // Footer line
      this.doc.setDrawColor(PDF_COLORS.gray[300][0], PDF_COLORS.gray[300][1], PDF_COLORS.gray[300][2]);
      this.doc.setLineWidth(0.5);
      this.doc.line(
        this.margin.left,
        this.pageHeight - 15,
        this.pageWidth - this.margin.right,
        this.pageHeight - 15
      );

      // Footer text
      this.doc.setTextColor(PDF_COLORS.gray[600][0], PDF_COLORS.gray[600][1], PDF_COLORS.gray[600][2]);
      this.doc.setFontSize(9);
      this.doc.setFont(this.style.fontFamily, 'normal');
      
      // Page number
      this.doc.text(
        `Page ${i} of ${pageCount}`,
        this.pageWidth / 2,
        this.pageHeight - 10,
        { align: 'center' }
      );

      // Generated by text
      this.doc.text(
        'Generated by AHI BodyScan',
        this.margin.left,
        this.pageHeight - 10
      );

      // Date
      this.doc.text(
        new Date().toLocaleDateString(),
        this.pageWidth - this.margin.right,
        this.pageHeight - 10,
        { align: 'right' }
      );
    }

    this.doc.save(filename);
  }
}