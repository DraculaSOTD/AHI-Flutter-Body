export const theme = {
  colors: {
    // Primary colors
    primaryPurple: '#6A00FF',
    primaryBlue: '#00B2FF',
    secondaryBlue: '#4A90E2',
    accentPurple: '#7B61FF',
    
    // Background colors
    backgroundLight: '#F5F5F7',
    backgroundWhite: '#FFFFFF',
    backgroundDark: '#111111',
    backgroundModal: '#F0FAFF',
    backgroundCard: '#E8EAF6',
    
    // Text colors
    textPrimary: '#111111',
    textSecondary: '#666666',
    textTertiary: '#8A8A8E',
    textPlaceholder: '#BDBDBD',
    textWhite: '#FFFFFF',
    textLabel: '#333333',
    
    // Border and divider colors
    borderLight: '#E0E0E0',
    borderAccent: '#A0E6FF',
    divider: '#EEEEEE',
    
    // Status colors
    success: '#4CAF50',
    warning: '#FFA726',
    error: '#E53935',
    info: '#29B6F6',
    
    // Shadow colors
    shadowLight: 'rgba(0, 0, 0, 0.1)',
    shadowMedium: 'rgba(0, 0, 0, 0.2)',
    
    // Special colors
    transparent: 'transparent',
    inactive: '#757575',
    activeTab: '#6A00FF',
    
    // Dark theme specific
    darkSurface: '#1C1C1E',
    darkCard: '#2C2C2E',
  },
  
  gradients: {
    primary: 'linear-gradient(90deg, #4A90E2, #7B61FF)',
    primaryReverse: 'linear-gradient(90deg, #7B61FF, #4A90E2)',
    cardBackground: 'linear-gradient(135deg, #F5F5F7 0%, #E8EAF6 100%)',
  },
  
  typography: {
    fontFamily: '-apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, "Helvetica Neue", Arial, sans-serif',
    
    headingLarge: {
      fontSize: '28px',
      fontWeight: '700',
      lineHeight: '36px',
    },
    headingMedium: {
      fontSize: '24px',
      fontWeight: '600',
      lineHeight: '32px',
    },
    headingSmall: {
      fontSize: '20px',
      fontWeight: '600',
      lineHeight: '28px',
    },
    
    bodyLarge: {
      fontSize: '16px',
      fontWeight: '400',
      lineHeight: '24px',
    },
    bodyMedium: {
      fontSize: '14px',
      fontWeight: '400',
      lineHeight: '20px',
    },
    bodySmall: {
      fontSize: '12px',
      fontWeight: '400',
      lineHeight: '16px',
    },
    
    buttonLarge: {
      fontSize: '18px',
      fontWeight: '600',
      lineHeight: '24px',
    },
    buttonMedium: {
      fontSize: '16px',
      fontWeight: '600',
      lineHeight: '20px',
    },
  },
  
  dimensions: {
    paddingXSmall: '4px',
    paddingSmall: '8px',
    paddingMedium: '16px',
    paddingLarge: '24px',
    paddingXLarge: '32px',
    
    radiusSmall: '4px',
    radiusMedium: '8px',
    radiusLarge: '16px',
    radiusXLarge: '24px',
    radiusRound: '50%',
    radiusPill: '28px',
    
    buttonHeightSmall: '36px',
    buttonHeightMedium: '44px',
    buttonHeightLarge: '56px',
    
    iconSizeSmall: '16px',
    iconSizeMedium: '24px',
    iconSizeLarge: '32px',
  },
  
  shadows: {
    small: '0 2px 4px rgba(0, 0, 0, 0.1)',
    medium: '0 4px 8px rgba(0, 0, 0, 0.1)',
    large: '0 8px 16px rgba(0, 0, 0, 0.1)',
    elevation: '0 4px 10px rgba(0, 0, 0, 0.1)',
  },
  
  transitions: {
    fast: '150ms ease-in-out',
    normal: '300ms ease-in-out',
    slow: '500ms ease-in-out',
  },
  
  breakpoints: {
    mobile: '480px',
    tablet: '768px',
    desktop: '1024px',
    wide: '1440px',
  },
};

export type Theme = typeof theme;