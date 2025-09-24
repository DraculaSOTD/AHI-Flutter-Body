export const PDF_COLORS = {
  primary: [106, 0, 255] as number[], // AHI Purple
  secondary: [0, 123, 255] as number[], // Blue
  success: [40, 167, 69] as number[], // Green
  warning: [255, 193, 7] as number[], // Yellow
  danger: [220, 53, 69] as number[], // Red
  dark: [33, 37, 41] as number[], // Dark gray
  light: [248, 249, 250] as number[], // Light gray
  white: [255, 255, 255] as number[],
  black: [0, 0, 0] as number[],
  gray: {
    100: [248, 249, 250] as number[],
    200: [233, 236, 239] as number[],
    300: [222, 226, 230] as number[],
    400: [206, 212, 218] as number[],
    500: [173, 181, 189] as number[],
    600: [108, 117, 125] as number[],
    700: [73, 80, 87] as number[],
    800: [52, 58, 64] as number[],
    900: [33, 37, 41] as number[]
  }
};

export const hexToRgb = (hex: string): number[] => {
  const result = /^#?([a-f\d]{2})([a-f\d]{2})([a-f\d]{2})$/i.exec(hex);
  return result ? [
    parseInt(result[1], 16),
    parseInt(result[2], 16),
    parseInt(result[3], 16)
  ] : [0, 0, 0];
};

export const getRiskColor = (level: 'low' | 'moderate' | 'high' | 'critical'): number[] => {
  switch (level) {
    case 'low':
      return PDF_COLORS.success;
    case 'moderate':
      return PDF_COLORS.warning;
    case 'high':
      return [255, 140, 0]; // Orange
    case 'critical':
      return PDF_COLORS.danger;
    default:
      return PDF_COLORS.gray[500];
  }
};

export const getMetricStatusColor = (value: number, min: number, max: number): number[] => {
  if (value < min) {
    return PDF_COLORS.warning;
  } else if (value > max) {
    return PDF_COLORS.danger;
  }
  return PDF_COLORS.success;
};

export const getGradientColors = (startColor: number[], endColor: number[], steps: number): number[][] => {
  const colors: number[][] = [];
  for (let i = 0; i < steps; i++) {
    const ratio = i / (steps - 1);
    colors.push([
      Math.round(startColor[0] + (endColor[0] - startColor[0]) * ratio),
      Math.round(startColor[1] + (endColor[1] - startColor[1]) * ratio),
      Math.round(startColor[2] + (endColor[2] - startColor[2]) * ratio)
    ]);
  }
  return colors;
};