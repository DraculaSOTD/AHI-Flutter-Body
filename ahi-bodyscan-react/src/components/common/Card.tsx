import styled from 'styled-components';
import { theme } from '../../styles/theme';

interface CardProps {
  padding?: string;
  margin?: string;
  elevation?: 'small' | 'medium' | 'large';
  clickable?: boolean;
}

export const Card = styled.div<CardProps>`
  background: ${theme.colors.backgroundWhite};
  border-radius: ${theme.dimensions.radiusLarge};
  padding: ${props => props.padding || theme.dimensions.paddingLarge};
  margin: ${props => props.margin || '0'};
  box-shadow: ${props => theme.shadows[props.elevation || 'small']};
  transition: all ${theme.transitions.fast};
  
  ${props => props.clickable && `
    cursor: pointer;
    
    &:hover {
      box-shadow: ${theme.shadows.medium};
      transform: translateY(-2px);
    }
    
    &:active {
      transform: translateY(0);
    }
  `}
`;

export const CardHeader = styled.div`
  display: flex;
  align-items: center;
  justify-content: space-between;
  margin-bottom: ${theme.dimensions.paddingMedium};
  
  h3 {
    font-size: ${theme.typography.headingSmall.fontSize};
    font-weight: ${theme.typography.headingSmall.fontWeight};
    color: ${theme.colors.textPrimary};
  }
`;

export const CardBody = styled.div`
  color: ${theme.colors.textSecondary};
  font-size: ${theme.typography.bodyMedium.fontSize};
  line-height: ${theme.typography.bodyMedium.lineHeight};
`;

export const CardFooter = styled.div`
  margin-top: ${theme.dimensions.paddingLarge};
  padding-top: ${theme.dimensions.paddingMedium};
  border-top: 1px solid ${theme.colors.divider};
  display: flex;
  align-items: center;
  justify-content: space-between;
  gap: ${theme.dimensions.paddingMedium};
`;

// Action Card for home screen
export const ActionCard = styled(Card)`
  background: ${theme.colors.backgroundWhite};
  padding: ${theme.dimensions.paddingLarge};
  display: flex;
  flex-direction: column;
  gap: ${theme.dimensions.paddingMedium};
  
  .icon-container {
    width: 56px;
    height: 56px;
    background: ${theme.gradients.primary};
    border-radius: ${theme.dimensions.radiusMedium};
    display: flex;
    align-items: center;
    justify-content: center;
    color: ${theme.colors.textWhite};
    font-size: 24px;
  }
  
  .content {
    flex: 1;
    
    h3 {
      font-size: ${theme.typography.headingSmall.fontSize};
      font-weight: ${theme.typography.headingSmall.fontWeight};
      color: ${theme.colors.textPrimary};
      margin-bottom: ${theme.dimensions.paddingSmall};
    }
    
    p {
      color: ${theme.colors.textSecondary};
      font-size: ${theme.typography.bodyMedium.fontSize};
      line-height: ${theme.typography.bodyMedium.lineHeight};
    }
  }
  
  .progress-container {
    width: 100%;
    
    .progress-bar {
      width: 100%;
      height: 8px;
      background: ${theme.colors.borderLight};
      border-radius: ${theme.dimensions.radiusSmall};
      overflow: hidden;
      margin-bottom: ${theme.dimensions.paddingSmall};
      
      .progress-fill {
        height: 100%;
        background: ${theme.colors.primaryPurple};
        transition: width ${theme.transitions.normal};
      }
    }
    
    .progress-text {
      font-size: ${theme.typography.bodySmall.fontSize};
      color: ${theme.colors.textTertiary};
    }
  }
`;