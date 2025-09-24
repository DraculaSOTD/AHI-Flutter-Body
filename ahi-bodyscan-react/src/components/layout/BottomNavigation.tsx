import React, { useState } from 'react';
import { NavLink } from 'react-router-dom';
import styled from 'styled-components';
import { theme } from '../../styles/theme';

interface NavItem {
  path: string;
  label: string;
  icon: React.ReactNode;
}

const navItems: NavItem[] = [
  {
    path: '/home',
    label: 'Home',
    icon: (
      <svg viewBox="0 0 24 24" fill="currentColor">
        <path d="M10 20v-6h4v6h5v-8h3L12 3 2 12h3v8z" />
      </svg>
    ),
  },
  {
    path: '/reports',
    label: 'Reports',
    icon: (
      <svg viewBox="0 0 24 24" fill="currentColor">
        <path d="M9 17H7v-7h2v7zm4 0h-2V7h2v10zm4 0h-2v-4h2v4zm2.5 2.1h-15V5h15v14.1zm0-16.1h-15c-1.1 0-2 .9-2 2v14c0 1.1.9 2 2 2h15c1.1 0 2-.9 2-2V5c0-1.1-.9-2-2-2z" />
      </svg>
    ),
  },
  {
    path: '/new-scan',
    label: 'Scan',
    icon: (
      <svg viewBox="0 0 24 24" fill="currentColor">
        <path d="M19 13h-6v6h-2v-6H5v-2h6V5h2v6h6v2z" />
      </svg>
    ),
  },
  {
    path: '/settings',
    label: 'Settings',
    icon: (
      <svg viewBox="0 0 24 24" fill="currentColor">
        <path d="M19.14,12.94c0.04-0.3,0.06-0.61,0.06-0.94c0-0.32-0.02-0.64-0.07-0.94l2.03-1.58c0.18-0.14,0.23-0.41,0.12-0.61 l-1.92-3.32c-0.12-0.22-0.37-0.29-0.59-0.22l-2.39,0.96c-0.5-0.38-1.03-0.7-1.62-0.94L14.4,2.81c-0.04-0.24-0.24-0.41-0.48-0.41 h-3.84c-0.24,0-0.43,0.17-0.47,0.41L9.25,5.35C8.66,5.59,8.12,5.92,7.63,6.29L5.24,5.33c-0.22-0.08-0.47,0-0.59,0.22L2.74,8.87 C2.62,9.08,2.66,9.34,2.86,9.48l2.03,1.58C4.84,11.36,4.8,11.69,4.8,12s0.02,0.64,0.07,0.94l-2.03,1.58 c-0.18,0.14-0.23,0.41-0.12,0.61l1.92,3.32c0.12,0.22,0.37,0.29,0.59,0.22l2.39-0.96c0.5,0.38,1.03,0.7,1.62,0.94l0.36,2.54 c0.05,0.24,0.24,0.41,0.48,0.41h3.84c0.24,0,0.44-0.17,0.47-0.41l0.36-2.54c0.59-0.24,1.13-0.56,1.62-0.94l2.39,0.96 c0.22,0.08,0.47,0,0.59-0.22l1.92-3.32c0.12-0.22,0.07-0.47-0.12-0.61L19.14,12.94z M12,15.6c-1.98,0-3.6-1.62-3.6-3.6 s1.62-3.6,3.6-3.6s3.6,1.62,3.6,3.6S13.98,15.6,12,15.6z" />
      </svg>
    ),
  },
];

const NavContainer = styled.nav`
  position: fixed;
  bottom: 0;
  left: 0;
  right: 0;
  background: ${theme.colors.backgroundWhite};
  border-top: 1px solid ${theme.colors.divider};
  z-index: 100;
  
  @media (min-width: ${theme.breakpoints.tablet}) {
    display: none;
  }
`;

const NavList = styled.div`
  display: flex;
  align-items: center;
  justify-content: space-around;
  height: 60px;
  padding: 0 ${theme.dimensions.paddingSmall};
`;

const NavLinkStyled = styled(NavLink)`
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  gap: 4px;
  flex: 1;
  height: 100%;
  color: ${theme.colors.inactive};
  transition: all ${theme.transitions.fast};
  position: relative;
  
  &.active {
    color: ${theme.colors.activeTab};
    
    svg {
      transform: scale(1.1);
    }
  }
  
  &:hover {
    color: ${theme.colors.primaryPurple};
  }
  
  svg {
    width: 24px;
    height: 24px;
    transition: transform ${theme.transitions.fast};
  }
  
  span {
    font-size: 11px;
    font-weight: 500;
  }
  
`;

const IconWrapper = styled.div`
  display: flex;
  align-items: center;
  justify-content: center;
`;

export const BottomNavigation: React.FC = () => {
  return (
    <NavContainer>
      <NavList>
        {navItems.map((item) => (
          <NavLinkStyled
            key={item.path}
            to={item.path}
            className={({ isActive }) => isActive ? 'active' : ''}
          >
            <IconWrapper>
              {item.icon}
            </IconWrapper>
            <span>{item.label}</span>
          </NavLinkStyled>
        ))}
      </NavList>
    </NavContainer>
  );
};

// Desktop Navigation Sidebar
const SidebarContainer = styled.aside`
  position: fixed;
  left: 0;
  top: 0;
  bottom: 0;
  width: 240px;
  background: ${theme.colors.backgroundWhite};
  border-right: 1px solid ${theme.colors.divider};
  padding: ${theme.dimensions.paddingLarge};
  display: none;
  
  @media (min-width: ${theme.breakpoints.tablet}) {
    display: block;
  }
`;

const Logo = styled.div`
  display: flex;
  align-items: center;
  gap: ${theme.dimensions.paddingSmall};
  margin-bottom: ${theme.dimensions.paddingXLarge};
`;

const LogoImage = styled.img`
  max-height: 32px;
  max-width: 120px;
  width: auto;
  height: auto;
  object-fit: contain;
`;

const LogoText = styled.span`
  font-size: ${theme.typography.headingMedium.fontSize};
  font-weight: ${theme.typography.headingMedium.fontWeight};
  color: ${theme.colors.primaryPurple};
`;

const SidebarNavLink = styled(NavLink)`
  display: flex;
  align-items: center;
  gap: ${theme.dimensions.paddingMedium};
  padding: ${theme.dimensions.paddingMedium};
  margin-bottom: ${theme.dimensions.paddingSmall};
  border-radius: ${theme.dimensions.radiusMedium};
  color: ${theme.colors.textSecondary};
  transition: all ${theme.transitions.fast};
  
  &.active {
    background: ${theme.colors.backgroundLight};
    color: ${theme.colors.primaryPurple};
    font-weight: 600;
  }
  
  &:hover {
    background: ${theme.colors.backgroundLight};
    color: ${theme.colors.primaryPurple};
  }
  
  svg {
    width: 20px;
    height: 20px;
  }
`;

export const DesktopSidebar: React.FC = () => {
  const [logoError, setLogoError] = useState(false);

  return (
    <SidebarContainer>
      <Logo>
        <LogoImage 
          src={logoError ? "/logo192.png" : "/company-logo.png"} 
          alt={logoError ? "Default Logo" : "Company Logo"}
          onError={() => {
            if (!logoError) {
              setLogoError(true);
            }
          }}
        />
        <LogoText>AHI</LogoText>
      </Logo>
      {navItems.map((item) => (
        <SidebarNavLink
          key={item.path}
          to={item.path}
          className={({ isActive }) => isActive ? 'active' : ''}
        >
          {item.icon}
          <span>{item.label}</span>
        </SidebarNavLink>
      ))}
    </SidebarContainer>
  );
};