import React, { useState } from 'react';
import styled from 'styled-components';
import { useNavigate } from 'react-router-dom';
import { theme } from '../../styles/theme';
import { Modal, ModalBody, ModalFooter } from '../../components/common/Modal';
import { PrimaryGradientButton } from '../../components/common/Button';
import { ProfileCreationModal } from '../../components/profile/ProfileCreationModal';

interface TermsOfServiceScreenProps {
  isOpen: boolean;
  onClose: () => void;
}

const ScrollableContent = styled.div`
  max-height: 400px;
  overflow-y: auto;
  padding: ${theme.dimensions.paddingMedium};
  background: ${theme.colors.backgroundLight};
  border-radius: ${theme.dimensions.radiusMedium};
  margin-bottom: ${theme.dimensions.paddingLarge};
  
  h3 {
    font-size: ${theme.typography.headingSmall.fontSize};
    font-weight: ${theme.typography.headingSmall.fontWeight};
    color: ${theme.colors.textPrimary};
    margin: ${theme.dimensions.paddingLarge} 0 ${theme.dimensions.paddingMedium} 0;
    
    &:first-child {
      margin-top: 0;
    }
  }
  
  p {
    font-size: ${theme.typography.bodyMedium.fontSize};
    line-height: ${theme.typography.bodyMedium.lineHeight};
    color: ${theme.colors.textSecondary};
    margin-bottom: ${theme.dimensions.paddingMedium};
  }
  
  ul {
    margin: ${theme.dimensions.paddingMedium} 0;
    padding-left: ${theme.dimensions.paddingLarge};
    
    li {
      font-size: ${theme.typography.bodyMedium.fontSize};
      line-height: ${theme.typography.bodyMedium.lineHeight};
      color: ${theme.colors.textSecondary};
      margin-bottom: ${theme.dimensions.paddingSmall};
    }
  }
`;

const CheckboxContainer = styled.label`
  display: flex;
  align-items: flex-start;
  gap: ${theme.dimensions.paddingMedium};
  cursor: pointer;
  
  input[type="checkbox"] {
    width: 20px;
    height: 20px;
    margin-top: 2px;
    cursor: pointer;
    accent-color: ${theme.colors.primaryPurple};
  }
  
  span {
    flex: 1;
    font-size: ${theme.typography.bodyMedium.fontSize};
    color: ${theme.colors.textPrimary};
    
    a {
      color: ${theme.colors.primaryBlue};
      text-decoration: underline;
      
      &:hover {
        text-decoration: none;
      }
    }
  }
`;

const termsContent = `
TERMS OF SERVICE

Last updated: ${new Date().toLocaleDateString()}

1. ACCEPTANCE OF TERMS

By accessing and using the AHI BodyScan application ("Service"), you accept and agree to be bound by the terms and provision of this agreement.

2. USE LICENSE

Permission is granted to temporarily download one copy of the AHI BodyScan app for personal, non-commercial transitory viewing only. This is the grant of a license, not a transfer of title, and under this license you may not:
• modify or copy the materials
• use the materials for any commercial purpose or for any public display
• attempt to reverse engineer any software contained in AHI BodyScan
• remove any copyright or other proprietary notations from the materials

3. HEALTH INFORMATION DISCLAIMER

The information provided by AHI BodyScan is for general informational purposes only. All information is provided in good faith, however we make no representation or warranty of any kind regarding the accuracy, adequacy, validity, reliability, availability, or completeness of any information.

The Service is not intended to be a substitute for professional medical advice, diagnosis, or treatment. Always seek the advice of your physician or other qualified health provider with any questions you may have regarding a medical condition.

4. PRIVACY POLICY

Your use of our Service is also governed by our Privacy Policy. Please review our Privacy Policy, which also governs the Site and informs users of our data collection practices.

5. DATA COLLECTION AND USE

By using the Service, you consent to:
• Camera access for body scanning functionality
• Processing of body measurements and health metrics
• Storage of scan results and personal health data
• Anonymous usage analytics to improve the service

6. LIMITATIONS

In no event shall AHI or its suppliers be liable for any damages (including, without limitation, damages for loss of data or profit, or due to business interruption) arising out of the use or inability to use the AHI BodyScan service.

7. REVISIONS AND ERRATA

The materials appearing in AHI BodyScan could include technical, typographical, or photographic errors. AHI does not warrant that any of the materials are accurate, complete, or current.

8. TERMINATION

We may terminate or suspend your account and bar access to the Service immediately, without prior notice or liability, under our sole discretion, for any reason whatsoever, including without limitation if you breach the Terms.

9. GOVERNING LAW

These Terms shall be governed and construed in accordance with the laws of the United States, without regard to its conflict of law provisions.

10. CONTACT INFORMATION

If you have any questions about these Terms, please contact us at:
Email: support@ahibodyscan.com
`;

export const TermsOfServiceScreen: React.FC<TermsOfServiceScreenProps> = ({ isOpen, onClose }) => {
  const navigate = useNavigate();
  const [accepted, setAccepted] = useState(false);
  const [loading, setLoading] = useState(false);
  const [showProfileModal, setShowProfileModal] = useState(false);
  
  const handleAccept = async () => {
    if (!accepted) return;
    
    setLoading(true);
    
    try {
      // Simulate API call
      await new Promise(resolve => setTimeout(resolve, 1000));
      
      // Store acceptance
      localStorage.setItem('termsAccepted', 'true');
      localStorage.setItem('termsAcceptedDate', new Date().toISOString());
      
      // Show profile creation modal
      setShowProfileModal(true);
    } catch (error) {
      console.error('Error accepting terms:', error);
    } finally {
      setLoading(false);
    }
  };
  
  const handlePrivacyPolicy = (e: React.MouseEvent) => {
    e.preventDefault();
    // Open privacy policy in new tab or modal
    window.open('/privacy-policy', '_blank');
  };
  
  return (
    <>
      <Modal isOpen={isOpen && !showProfileModal} onClose={onClose} title="Terms of Service" size="medium">
        <ModalBody>
          <ScrollableContent>
            <pre style={{ 
              fontFamily: theme.typography.fontFamily, 
              whiteSpace: 'pre-wrap',
              margin: 0 
            }}>
              {termsContent}
            </pre>
          </ScrollableContent>
          
          <CheckboxContainer>
            <input
              type="checkbox"
              checked={accepted}
              onChange={(e) => setAccepted(e.target.checked)}
            />
            <span>
              I have read and agree to the Terms of Service and{' '}
              <a href="/privacy-policy" onClick={handlePrivacyPolicy}>
                Privacy Policy
              </a>
            </span>
          </CheckboxContainer>
        </ModalBody>
        
        <ModalFooter>
          <PrimaryGradientButton
            size="large"
            fullWidth
            onClick={handleAccept}
            disabled={!accepted || loading}
            loading={loading}
          >
            Accept and Continue
          </PrimaryGradientButton>
        </ModalFooter>
      </Modal>
      
      <ProfileCreationModal
        isOpen={showProfileModal}
        onClose={() => {
          setShowProfileModal(false);
          navigate('/home');
        }}
        onSkip={() => {
          setShowProfileModal(false);
          navigate('/home');
        }}
      />
    </>
  );
};