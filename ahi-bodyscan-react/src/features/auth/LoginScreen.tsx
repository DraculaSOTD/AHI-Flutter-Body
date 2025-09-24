import React, { useState } from 'react';
import styled from 'styled-components';
import { useNavigate } from 'react-router-dom';
import { theme } from '../../styles/theme';
import { Modal, ModalBody } from '../../components/common/Modal';
import { PrimaryGradientButton } from '../../components/common/Button';
import { Input, PasswordInput } from '../../components/forms/Input';

interface LoginScreenProps {
  isOpen: boolean;
  onClose: () => void;
  onSuccess: () => void;
}

const LoginForm = styled.form`
  display: flex;
  flex-direction: column;
  gap: ${theme.dimensions.paddingLarge};
`;

const Title = styled.h2`
  font-size: ${theme.typography.headingMedium.fontSize};
  font-weight: ${theme.typography.headingMedium.fontWeight};
  color: ${theme.colors.textPrimary};
  text-align: center;
  margin-bottom: ${theme.dimensions.paddingMedium};
`;

const ErrorMessage = styled.div`
  background: ${theme.colors.error}10;
  border: 1px solid ${theme.colors.error}30;
  border-radius: ${theme.dimensions.radiusMedium};
  padding: ${theme.dimensions.paddingMedium};
  color: ${theme.colors.error};
  font-size: ${theme.typography.bodyMedium.fontSize};
  text-align: center;
`;

const ForgotPasswordLink = styled.button`
  color: ${theme.colors.primaryBlue};
  font-size: ${theme.typography.bodyMedium.fontSize};
  text-align: right;
  margin-top: -${theme.dimensions.paddingMedium};
  
  &:hover {
    text-decoration: underline;
  }
`;

const SignUpPrompt = styled.div`
  text-align: center;
  color: ${theme.colors.textSecondary};
  font-size: ${theme.typography.bodyMedium.fontSize};
  
  button {
    color: ${theme.colors.primaryBlue};
    font-weight: 600;
    margin-left: 4px;
    
    &:hover {
      text-decoration: underline;
    }
  }
`;

const TestAccountsInfo = styled.div`
  background: ${theme.colors.backgroundLight};
  border-radius: ${theme.dimensions.radiusMedium};
  padding: ${theme.dimensions.paddingLarge};
  margin-top: ${theme.dimensions.paddingLarge};
  
  h4 {
    font-size: ${theme.typography.bodyMedium.fontSize};
    font-weight: 600;
    color: ${theme.colors.textPrimary};
    margin-bottom: ${theme.dimensions.paddingMedium};
  }
  
  .account {
    margin-bottom: ${theme.dimensions.paddingSmall};
    font-size: ${theme.typography.bodySmall.fontSize};
    color: ${theme.colors.textSecondary};
    
    code {
      background: ${theme.colors.backgroundWhite};
      padding: 2px 6px;
      border-radius: 4px;
      font-family: monospace;
      color: ${theme.colors.primaryPurple};
    }
  }
`;

export const LoginScreen: React.FC<LoginScreenProps> = ({ isOpen, onClose, onSuccess }) => {
  const navigate = useNavigate();
  const [email, setEmail] = useState('');
  const [password, setPassword] = useState('');
  const [error, setError] = useState('');
  const [loading, setLoading] = useState(false);
  
  const handleSubmit = async (e: React.FormEvent) => {
    e.preventDefault();
    setError('');
    setLoading(true);
    
    try {
      // Simulate API call
      await new Promise(resolve => setTimeout(resolve, 1500));
      
      // Test user accounts
      if (email === 'newuser@test.com' && password === 'test123') {
        // New user - clear any existing data
        localStorage.clear();
        localStorage.setItem('isAuthenticated', 'true');
        localStorage.setItem('userEmail', email);
        // Explicitly ensure no demo data is set
        localStorage.removeItem('completedAssessments');
        localStorage.removeItem('inProgressAssessments');
        localStorage.removeItem('selectedProfile');
        localStorage.removeItem('userProfile');
        localStorage.removeItem('profileCreated');
        onSuccess();
        navigate('/terms');
      } else if (email === 'returninguser@test.com' && password === 'test123') {
        // Returning user - set up profile and past scans
        localStorage.clear();
        localStorage.setItem('isAuthenticated', 'true');
        localStorage.setItem('userEmail', email);
        
        // Set up profile data
        const profileData = {
          biologicalSex: 'male',
          height: 1.78, // 5'10"
          heightUnit: 'feet',
          dateOfBirth: '1985-03-15',
          smokingStatus: 'never',
          diabetesStatus: 'no',
          hasHypertension: 'no',
          takingBPMedication: 'no',
          lastUpdated: new Date().toISOString()
        };
        localStorage.setItem('userProfile', JSON.stringify(profileData));
        localStorage.setItem('profileCreated', 'true');
        
        // Set up selected profile
        const selectedProfile = {
          id: '1',
          name: 'John Doe',
          age: 38,
          gender: 'male',
          height: 178,
          weight: 75
        };
        localStorage.setItem('selectedProfile', JSON.stringify(selectedProfile));
        
        // Set up past assessments
        const pastAssessments = [
          {
            id: 'body-1',
            type: 'body',
            date: new Date(Date.now() - 30 * 24 * 60 * 60 * 1000).toISOString(), // 30 days ago
            status: 'completed',
            profileName: 'John Doe',
            results: {
              bodyFat: 18.5,
              muscleMass: 31.2,
              bmi: 23.7,
              waist: 86,
              chest: 102,
              hip: 96
            }
          },
          {
            id: 'body-2',
            type: 'body',
            date: new Date(Date.now() - 60 * 24 * 60 * 60 * 1000).toISOString(), // 60 days ago
            status: 'completed',
            profileName: 'John Doe',
            results: {
              bodyFat: 19.2,
              muscleMass: 30.8,
              bmi: 24.1,
              waist: 88,
              chest: 101,
              hip: 97
            }
          },
          {
            id: 'body-3',
            type: 'body',
            date: new Date(Date.now() - 90 * 24 * 60 * 60 * 1000).toISOString(), // 90 days ago
            status: 'completed',
            profileName: 'John Doe',
            results: {
              bodyFat: 20.1,
              muscleMass: 30.2,
              bmi: 24.5,
              waist: 90,
              chest: 100,
              hip: 98
            }
          },
          {
            id: 'face-1',
            type: 'face',
            date: new Date(Date.now() - 7 * 24 * 60 * 60 * 1000).toISOString(), // 7 days ago
            status: 'completed',
            profileName: 'John Doe',
            results: {
              heartRate: 68,
              bloodPressure: '118/78',
              stressLevel: 'Low',
              respiratoryRate: 14,
              oxygenSaturation: 98
            }
          },
          {
            id: 'face-2',
            type: 'face',
            date: new Date(Date.now() - 14 * 24 * 60 * 60 * 1000).toISOString(), // 14 days ago
            status: 'completed',
            profileName: 'John Doe',
            results: {
              heartRate: 72,
              bloodPressure: '122/80',
              stressLevel: 'Medium',
              respiratoryRate: 16,
              oxygenSaturation: 97
            }
          }
        ];
        localStorage.setItem('completedAssessments', JSON.stringify(pastAssessments));
        
        // Mark terms as already accepted
        localStorage.setItem('termsAccepted', 'true');
        
        onSuccess();
        navigate('/home');
      } else if (email && password) {
        // Regular login - accept any email/password for demo
        localStorage.setItem('isAuthenticated', 'true');
        localStorage.setItem('userEmail', email);
        onSuccess();
        navigate('/terms');
      } else {
        setError('Please enter your email and password');
      }
    } catch (err) {
      setError('Invalid email or password');
    } finally {
      setLoading(false);
    }
  };
  
  const handleForgotPassword = () => {
    // Handle forgot password flow
    console.log('Forgot password clicked');
  };
  
  const handleSignUp = () => {
    // Handle sign up flow
    console.log('Sign up clicked');
  };
  
  return (
    <Modal isOpen={isOpen} onClose={onClose} title="Sign In" size="small" showCloseButton={false}>
      <ModalBody>
        <LoginForm onSubmit={handleSubmit}>
          <div>
            <Title>Welcome Back</Title>
            {error && <ErrorMessage>{error}</ErrorMessage>}
          </div>
          
          <Input
            type="email"
            label="Email"
            placeholder="Enter your email"
            value={email}
            onChange={(e) => setEmail(e.target.value)}
            required
            showClear
            onClear={() => setEmail('')}
          />
          
          <div>
            <PasswordInput
              label="Password"
              placeholder="Enter your password"
              value={password}
              onChange={(e) => setPassword(e.target.value)}
              required
            />
            <ForgotPasswordLink type="button" onClick={handleForgotPassword}>
              Forgot Password?
            </ForgotPasswordLink>
          </div>
          
          <PrimaryGradientButton
            type="submit"
            size="large"
            fullWidth
            loading={loading}
            disabled={loading}
          >
            Sign In
          </PrimaryGradientButton>
          
          <SignUpPrompt>
            Don't have an account?
            <button type="button" onClick={handleSignUp}>
              Sign Up
            </button>
          </SignUpPrompt>
        </LoginForm>
      </ModalBody>
    </Modal>
  );
};