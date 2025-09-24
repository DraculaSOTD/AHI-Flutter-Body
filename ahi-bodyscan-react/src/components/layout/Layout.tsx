import React from 'react';
import { Outlet } from 'react-router-dom';
import styled from 'styled-components';
import { theme } from '../../styles/theme';
import { BottomNavigation, DesktopSidebar } from './BottomNavigation';

interface LayoutProps {
  children?: React.ReactNode;
  showNavigation?: boolean;
}

const LayoutContainer = styled.div`
  display: flex;
  width: 100%;
  height: 100%;
  background: ${theme.colors.backgroundLight};
`;

const MainContent = styled.main<{ hasNavigation: boolean }>`
  flex: 1;
  padding-bottom: ${props => props.hasNavigation ? '60px' : '0'};
  overflow-y: auto;
  overflow-x: hidden;
  width: 100%;
  
  @media (min-width: ${theme.breakpoints.tablet}) {
    margin-left: ${props => props.hasNavigation ? '240px' : '0'};
    padding-bottom: 0;
  }
`;

const PageContainer = styled.div`
  min-height: 100%;
  display: flex;
  flex-direction: column;
`;

export const Layout: React.FC<LayoutProps> = ({ children, showNavigation = true }) => {
  return (
    <LayoutContainer>
      {showNavigation && <DesktopSidebar />}
      <MainContent hasNavigation={showNavigation}>
        <PageContainer>
          {children || <Outlet />}
        </PageContainer>
      </MainContent>
      {showNavigation && <BottomNavigation />}
    </LayoutContainer>
  );
};

// Page Header Component
interface PageHeaderProps {
  title: string;
  subtitle?: string;
  actions?: React.ReactNode;
  showBackButton?: boolean;
  onBack?: () => void;
}

const HeaderContainer = styled.header`
  background: ${theme.colors.backgroundWhite};
  padding: ${theme.dimensions.paddingLarge};
  border-bottom: 1px solid ${theme.colors.divider};
`;

const HeaderContent = styled.div`
  display: flex;
  align-items: center;
  justify-content: space-between;
  gap: ${theme.dimensions.paddingMedium};
`;

const HeaderLeft = styled.div`
  display: flex;
  align-items: center;
  gap: ${theme.dimensions.paddingMedium};
  flex: 1;
`;

const BackButton = styled.button`
  width: 40px;
  height: 40px;
  border-radius: ${theme.dimensions.radiusMedium};
  display: flex;
  align-items: center;
  justify-content: center;
  color: ${theme.colors.textSecondary};
  transition: all ${theme.transitions.fast};
  
  &:hover {
    background: ${theme.colors.backgroundLight};
    color: ${theme.colors.textPrimary};
  }
  
  svg {
    width: 24px;
    height: 24px;
  }
`;

const HeaderTitleContainer = styled.div`
  flex: 1;
`;

const HeaderTitle = styled.h1`
  font-size: ${theme.typography.headingLarge.fontSize};
  font-weight: ${theme.typography.headingLarge.fontWeight};
  color: ${theme.colors.textPrimary};
  margin: 0;
`;

const HeaderSubtitle = styled.p`
  font-size: ${theme.typography.bodyMedium.fontSize};
  color: ${theme.colors.textSecondary};
  margin: 4px 0 0 0;
`;

const HeaderActions = styled.div`
  display: flex;
  align-items: center;
  gap: ${theme.dimensions.paddingSmall};
`;

export const PageHeader: React.FC<PageHeaderProps> = ({
  title,
  subtitle,
  actions,
  showBackButton,
  onBack,
}) => {
  return (
    <HeaderContainer>
      <HeaderContent>
        <HeaderLeft>
          {showBackButton && (
            <BackButton onClick={onBack}>
              <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2">
                <path d="M15 18l-6-6 6-6" />
              </svg>
            </BackButton>
          )}
          <HeaderTitleContainer>
            <HeaderTitle>{title}</HeaderTitle>
            {subtitle && <HeaderSubtitle>{subtitle}</HeaderSubtitle>}
          </HeaderTitleContainer>
        </HeaderLeft>
        {actions && <HeaderActions>{actions}</HeaderActions>}
      </HeaderContent>
    </HeaderContainer>
  );
};

// Page Content Container
export const PageContent = styled.div`
  flex: 1;
  padding: ${theme.dimensions.paddingLarge};
  width: 100%;
  overflow-x: hidden;
  
  @media (max-width: ${theme.breakpoints.mobile}) {
    padding: ${theme.dimensions.paddingMedium};
  }
`;