import React, { useState } from 'react';
import styled from 'styled-components';
import { useNavigate } from 'react-router-dom';
import { theme } from '../../styles/theme';
import { Layout, PageHeader, PageContent } from '../../components/layout/Layout';
import { Modal, ModalBody, ModalFooter } from '../../components/common/Modal';
import { PrimaryGradientButton, OutlinedButton, TextButton } from '../../components/common/Button';

interface SettingsItem {
  id: string;
  title: string;
  icon?: React.ReactNode;
  action: () => void;
  danger?: boolean;
}

const SettingsList = styled.div`
  background: ${theme.colors.backgroundWhite};
  border-radius: ${theme.dimensions.radiusLarge};
  overflow: hidden;
  box-shadow: ${theme.shadows.small};
`;

const SettingsListItem = styled.button<{ danger?: boolean }>`
  width: 100%;
  padding: ${theme.dimensions.paddingLarge};
  display: flex;
  align-items: center;
  justify-content: space-between;
  background: transparent;
  border: none;
  border-bottom: 1px solid ${theme.colors.divider};
  cursor: pointer;
  transition: all ${theme.transitions.fast};
  
  &:last-child {
    border-bottom: none;
  }
  
  &:hover {
    background: ${theme.colors.backgroundLight};
  }
  
  &:active {
    background: ${theme.colors.borderLight};
  }
`;

const SettingsItemContent = styled.div`
  display: flex;
  align-items: center;
  gap: ${theme.dimensions.paddingMedium};
`;

const SettingsItemIcon = styled.div<{ danger?: boolean }>`
  width: 40px;
  height: 40px;
  border-radius: ${theme.dimensions.radiusMedium};
  background: ${props => props.danger ? theme.colors.error + '20' : theme.colors.backgroundLight};
  color: ${props => props.danger ? theme.colors.error : theme.colors.textSecondary};
  display: flex;
  align-items: center;
  justify-content: center;
  
  svg {
    width: 20px;
    height: 20px;
  }
`;

const SettingsItemText = styled.span<{ danger?: boolean }>`
  font-size: ${theme.typography.bodyLarge.fontSize};
  color: ${props => props.danger ? theme.colors.error : theme.colors.textPrimary};
  font-weight: 500;
`;

const ChevronIcon = styled.div`
  color: ${theme.colors.textTertiary};
  
  svg {
    width: 20px;
    height: 20px;
  }
`;

const UserInfo = styled.div`
  text-align: center;
  padding: ${theme.dimensions.paddingXLarge} 0;
  
  .avatar {
    width: 80px;
    height: 80px;
    border-radius: 50%;
    background: ${theme.gradients.primary};
    color: ${theme.colors.textWhite};
    display: flex;
    align-items: center;
    justify-content: center;
    font-size: 32px;
    font-weight: 600;
    margin: 0 auto ${theme.dimensions.paddingMedium};
  }
  
  .name {
    font-size: ${theme.typography.headingMedium.fontSize};
    font-weight: ${theme.typography.headingMedium.fontWeight};
    color: ${theme.colors.textPrimary};
    margin-bottom: 4px;
  }
  
  .email {
    font-size: ${theme.typography.bodyMedium.fontSize};
    color: ${theme.colors.textSecondary};
  }
`;

const ModalContent = styled.div`
  text-align: center;
  padding: ${theme.dimensions.paddingLarge} 0;
  
  .icon {
    width: 64px;
    height: 64px;
    margin: 0 auto ${theme.dimensions.paddingLarge};
    color: ${theme.colors.warning};
  }
  
  h3 {
    font-size: ${theme.typography.headingMedium.fontSize};
    font-weight: ${theme.typography.headingMedium.fontWeight};
    color: ${theme.colors.textPrimary};
    margin-bottom: ${theme.dimensions.paddingMedium};
  }
  
  p {
    font-size: ${theme.typography.bodyMedium.fontSize};
    color: ${theme.colors.textSecondary};
    line-height: 1.5;
  }
`;

const DangerModalContent = styled(ModalContent)`
  .icon {
    color: ${theme.colors.error};
  }
`;

export const SettingsScreen: React.FC = () => {
  const navigate = useNavigate();
  const [showLogoutModal, setShowLogoutModal] = useState(false);
  const [showDeleteModal, setShowDeleteModal] = useState(false);
  
  const userEmail = localStorage.getItem('userEmail') || 'user@example.com';
  const userName = userEmail.split('@')[0];
  const userInitial = userName.charAt(0).toUpperCase();
  
  const handleLogout = () => {
    localStorage.removeItem('isAuthenticated');
    localStorage.removeItem('termsAccepted');
    localStorage.removeItem('userEmail');
    navigate('/login');
  };
  
  const handleDeleteAccount = () => {
    // In a real app, this would call an API to delete the account
    localStorage.clear();
    navigate('/login');
  };
  
  const settingsItems: SettingsItem[] = [
    {
      id: 'email-report',
      title: 'Example Health Assessment',
      icon: (
        <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2">
          <path d="M3 8l7.89 5.26a2 2 0 002.22 0L21 8M5 19h14a2 2 0 002-2V7a2 2 0 00-2-2H5a2 2 0 00-2 2v10a2 2 0 002 2z" />
        </svg>
      ),
      action: () => console.log('Email report'),
    },
    {
      id: 'terms',
      title: 'Terms of Service',
      icon: (
        <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2">
          <path d="M14 2H6a2 2 0 00-2 2v16a2 2 0 002 2h12a2 2 0 002-2V8z" />
          <path d="M14 2v6h6M16 13H8M16 17H8M10 9H8" />
        </svg>
      ),
      action: () => navigate('/terms'),
    },
    {
      id: 'privacy',
      title: 'Privacy Policy',
      icon: (
        <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2">
          <path d="M12 22s8-4 8-10V5l-8-3-8 3v7c0 6 8 10 8 10z" />
        </svg>
      ),
      action: () => navigate('/privacy'),
    },
    {
      id: 'about',
      title: 'About',
      icon: (
        <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2">
          <circle cx="12" cy="12" r="10" />
          <path d="M12 16v-4M12 8h.01" />
        </svg>
      ),
      action: () => navigate('/about'),
    },
    {
      id: 'logout',
      title: 'Logout',
      icon: (
        <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2">
          <path d="M9 21H5a2 2 0 01-2-2V5a2 2 0 012-2h4M16 17l5-5-5-5M21 12H9" />
        </svg>
      ),
      action: () => setShowLogoutModal(true),
    },
    {
      id: 'delete',
      title: 'Delete Account',
      icon: (
        <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2">
          <path d="M3 6h18M8 6V4a2 2 0 012-2h4a2 2 0 012 2v2m3 0v14a2 2 0 01-2 2H7a2 2 0 01-2-2V6h14M10 11v6M14 11v6" />
        </svg>
      ),
      action: () => setShowDeleteModal(true),
      danger: true,
    },
  ];
  
  return (
    <Layout>
      <PageHeader title="Settings" />
      <PageContent>
        <UserInfo>
          <div className="avatar">{userInitial}</div>
          <div className="name">{userName}</div>
          <div className="email">{userEmail}</div>
        </UserInfo>
        
        <SettingsList>
          {settingsItems.map((item) => (
            <SettingsListItem
              key={item.id}
              onClick={item.action}
              danger={item.danger}
            >
              <SettingsItemContent>
                <SettingsItemIcon danger={item.danger}>
                  {item.icon}
                </SettingsItemIcon>
                <SettingsItemText danger={item.danger}>
                  {item.title}
                </SettingsItemText>
              </SettingsItemContent>
              <ChevronIcon>
                <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2">
                  <path d="M9 18l6-6-6-6" />
                </svg>
              </ChevronIcon>
            </SettingsListItem>
          ))}
        </SettingsList>
      </PageContent>
      
      {/* Logout Confirmation Modal */}
      <Modal
        isOpen={showLogoutModal}
        onClose={() => setShowLogoutModal(false)}
        title="Logout"
        size="small"
      >
        <ModalBody>
          <ModalContent>
            <div className="icon">
              <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2">
                <path d="M9 21H5a2 2 0 01-2-2V5a2 2 0 012-2h4M16 17l5-5-5-5M21 12H9" />
              </svg>
            </div>
            <h3>Are you sure you want to logout?</h3>
            <p>You'll need to sign in again to access your health assessments.</p>
          </ModalContent>
        </ModalBody>
        <ModalFooter>
          <OutlinedButton onClick={() => setShowLogoutModal(false)}>
            Cancel
          </OutlinedButton>
          <PrimaryGradientButton onClick={handleLogout}>
            Logout
          </PrimaryGradientButton>
        </ModalFooter>
      </Modal>
      
      {/* Delete Account Modal */}
      <Modal
        isOpen={showDeleteModal}
        onClose={() => setShowDeleteModal(false)}
        title="Delete Account"
        size="small"
      >
        <ModalBody>
          <DangerModalContent>
            <div className="icon">
              <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2">
                <path d="M12 9v2m0 4h.01m-6.938 4h13.856c1.54 0 2.502-1.667 1.732-3L13.732 4c-.77-1.333-2.694-1.333-3.464 0L3.34 16c-.77 1.333.192 3 1.732 3z" />
              </svg>
            </div>
            <h3>Delete Account?</h3>
            <p>
              This action cannot be undone. All your health assessments and data will be permanently deleted.
            </p>
          </DangerModalContent>
        </ModalBody>
        <ModalFooter>
          <TextButton onClick={() => setShowDeleteModal(false)}>
            Cancel
          </TextButton>
          <PrimaryGradientButton
            onClick={handleDeleteAccount}
            style={{ background: theme.colors.error }}
          >
            Delete Account
          </PrimaryGradientButton>
        </ModalFooter>
      </Modal>
    </Layout>
  );
};