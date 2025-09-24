import React, { useEffect } from 'react';
import styled from 'styled-components';
import { useNavigate } from 'react-router-dom';
import { theme } from '../../styles/theme';
import { InitializingScreen as LoadingComponent } from '../../components/common/LoadingOverlay';

const Container = styled.div`
  width: 100%;
  height: 100vh;
  background: ${theme.colors.backgroundDark};
  position: relative;
`;

export const InitializingScreen: React.FC = () => {
  const navigate = useNavigate();
  
  useEffect(() => {
    // Check authentication status
    const checkAuth = async () => {
      try {
        // Simulate initialization delay
        await new Promise(resolve => setTimeout(resolve, 2000));
        
        const isAuthenticated = localStorage.getItem('isAuthenticated') === 'true';
        const termsAccepted = localStorage.getItem('termsAccepted') === 'true';
        
        if (!isAuthenticated) {
          navigate('/login');
        } else if (!termsAccepted) {
          navigate('/terms');
        } else {
          navigate('/home');
        }
      } catch (error) {
        console.error('Initialization error:', error);
        navigate('/login');
      }
    };
    
    checkAuth();
  }, [navigate]);
  
  return (
    <Container>
      <LoadingComponent />
    </Container>
  );
};