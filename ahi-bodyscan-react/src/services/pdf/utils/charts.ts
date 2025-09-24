import { Chart, ChartConfiguration, registerables } from 'chart.js';
import { ChartConfig } from '../types';
import { PDF_COLORS, hexToRgb } from './colors';

// Register Chart.js components
Chart.register(...registerables);

export const createChart = async (
  config: ChartConfig
): Promise<string> => {
  // Create a canvas element
  const canvas = document.createElement('canvas');
  canvas.width = config.width;
  canvas.height = config.height;
  
  const ctx = canvas.getContext('2d');
  if (!ctx) {
    throw new Error('Failed to get canvas context');
  }

  // Default chart options
  const defaultOptions = {
    responsive: false,
    maintainAspectRatio: false,
    plugins: {
      legend: {
        position: 'bottom' as const,
        labels: {
          padding: 10,
          font: {
            size: 12
          }
        }
      },
      title: {
        display: false
      }
    },
    scales: config.type === 'radar' || config.type === 'doughnut' ? {} : {
      y: {
        beginAtZero: true,
        grid: {
          color: 'rgba(0, 0, 0, 0.1)'
        }
      },
      x: {
        grid: {
          display: false
        }
      }
    }
  };

  // Merge with provided options
  const chartOptions = {
    ...defaultOptions,
    ...config.options
  };

  // Create the chart
  const chartConfig: ChartConfiguration = {
    type: config.type,
    data: config.data,
    options: chartOptions
  };

  const chart = new Chart(ctx, chartConfig);

  // Wait for chart to render
  await new Promise(resolve => setTimeout(resolve, 100));

  // Convert to base64
  const base64 = canvas.toDataURL('image/png');
  
  // Destroy chart to free memory
  chart.destroy();
  
  return base64;
};

export const createBodyCompositionChart = async (
  bodyFat: number,
  muscleMass: number,
  fatFreeMass?: number
): Promise<string> => {
  const data = {
    labels: ['Body Fat %', 'Muscle Mass %', 'Other %'],
    datasets: [{
      data: [
        bodyFat,
        (muscleMass / 100) * (100 - bodyFat), // Convert kg to percentage
        100 - bodyFat - ((muscleMass / 100) * (100 - bodyFat))
      ],
      backgroundColor: [
        `rgb(${PDF_COLORS.danger.join(',')})`,
        `rgb(${PDF_COLORS.success.join(',')})`,
        `rgb(${PDF_COLORS.gray[400].join(',')})`
      ],
      borderWidth: 0
    }]
  };

  return createChart({
    type: 'doughnut',
    data,
    width: 400,
    height: 300,
    options: {
      plugins: {
        legend: {
          position: 'right'
        }
      }
    }
  });
};

export const createMeasurementsChart = async (
  measurements: {
    waist: number;
    chest: number;
    hip: number;
    thigh?: number;
  }
): Promise<string> => {
  const labels = ['Chest', 'Waist', 'Hip'];
  const data = [measurements.chest, measurements.waist, measurements.hip];
  
  if (measurements.thigh) {
    labels.push('Thigh');
    data.push(measurements.thigh);
  }

  return createChart({
    type: 'bar',
    data: {
      labels,
      datasets: [{
        label: 'Measurements (cm)',
        data,
        backgroundColor: `rgba(${PDF_COLORS.primary.join(',')}, 0.7)`,
        borderColor: `rgb(${PDF_COLORS.primary.join(',')})`,
        borderWidth: 2
      }]
    },
    width: 500,
    height: 300,
    options: {
      plugins: {
        legend: {
          display: false
        }
      }
    }
  });
};

export const createVitalsChart = async (
  vitals: {
    heartRate: number;
    systolic: number;
    diastolic: number;
    respiratoryRate: number;
    oxygenSaturation: number;
  }
): Promise<string> => {
  // Normalize values to 0-100 scale for radar chart
  const normalizeValue = (value: number, min: number, max: number): number => {
    return ((value - min) / (max - min)) * 100;
  };

  const data = {
    labels: [
      'Heart Rate',
      'Blood Pressure',
      'Respiratory Rate',
      'O₂ Saturation',
      'Overall Health'
    ],
    datasets: [{
      label: 'Current',
      data: [
        normalizeValue(vitals.heartRate, 40, 100),
        normalizeValue((vitals.systolic + vitals.diastolic) / 2, 60, 140),
        normalizeValue(vitals.respiratoryRate, 8, 25),
        vitals.oxygenSaturation,
        // Calculate overall health score
        (normalizeValue(vitals.heartRate, 40, 100) +
         normalizeValue((vitals.systolic + vitals.diastolic) / 2, 60, 140) +
         normalizeValue(vitals.respiratoryRate, 8, 25) +
         vitals.oxygenSaturation) / 4
      ],
      backgroundColor: `rgba(${PDF_COLORS.primary.join(',')}, 0.2)`,
      borderColor: `rgb(${PDF_COLORS.primary.join(',')})`,
      borderWidth: 2,
      pointBackgroundColor: `rgb(${PDF_COLORS.primary.join(',')})`,
      pointBorderColor: '#fff',
      pointHoverBackgroundColor: '#fff',
      pointHoverBorderColor: `rgb(${PDF_COLORS.primary.join(',')})`
    }, {
      label: 'Normal Range',
      data: [75, 75, 75, 97, 85], // Ideal values
      backgroundColor: `rgba(${PDF_COLORS.success.join(',')}, 0.1)`,
      borderColor: `rgb(${PDF_COLORS.success.join(',')})`,
      borderWidth: 1,
      borderDash: [5, 5],
      pointRadius: 0
    }]
  };

  return createChart({
    type: 'radar',
    data,
    width: 400,
    height: 400,
    options: {
      scales: {
        r: {
          beginAtZero: true,
          max: 100,
          ticks: {
            stepSize: 20
          }
        }
      }
    }
  });
};

export const createTrendChart = async (
  historicalData: Array<{
    date: string;
    value: number;
  }>,
  label: string,
  unit: string
): Promise<string> => {
  return createChart({
    type: 'line',
    data: {
      labels: historicalData.map(d => d.date),
      datasets: [{
        label: `${label} (${unit})`,
        data: historicalData.map(d => d.value),
        borderColor: `rgb(${PDF_COLORS.primary.join(',')})`,
        backgroundColor: `rgba(${PDF_COLORS.primary.join(',')}, 0.1)`,
        tension: 0.4,
        fill: true
      }]
    },
    width: 600,
    height: 300,
    options: {
      plugins: {
        legend: {
          display: false
        }
      }
    }
  });
};

export const createRiskGaugeChart = async (
  riskScore: number, // 0-1
  riskLabel: string
): Promise<string> => {
  const gaugeValue = riskScore * 270; // Convert to degrees (270° arc)
  
  // Create gauge segments
  const segments = [
    { value: 90, color: PDF_COLORS.success }, // Low risk
    { value: 90, color: PDF_COLORS.warning }, // Moderate risk
    { value: 90, color: PDF_COLORS.danger }   // High risk
  ];

  const data = {
    datasets: [{
      data: segments.map(s => s.value),
      backgroundColor: segments.map(s => `rgb(${s.color.join(',')})`),
      borderWidth: 0,
      circumference: 270,
      rotation: 225
    }]
  };

  return createChart({
    type: 'doughnut',
    data,
    width: 300,
    height: 200,
    options: {
      cutout: '75%',
      plugins: {
        legend: {
          display: false
        },
        tooltip: {
          enabled: false
        }
      }
    }
  });
};