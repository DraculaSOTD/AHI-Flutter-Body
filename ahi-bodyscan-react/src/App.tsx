import React from 'react';
import { BrowserRouter as Router, Routes, Route, Navigate, useNavigate } from 'react-router-dom';
import { ThemeProvider } from 'styled-components';
import { Provider } from 'react-redux';
import { theme } from './styles/theme';
import { GlobalStyles } from './styles/GlobalStyles';
import { store } from './store/store';

// Auth screens
import { InitializingScreen } from './features/auth/InitializingScreen';
import { LoginScreen } from './features/auth/LoginScreen';
import { TermsOfServiceScreen } from './features/auth/TermsOfServiceScreen';

// Main screens
import { HomeScreen } from './features/home/HomeScreen';
import { NewScanScreen } from './features/scanning/NewScanScreen';
import { UserProfileScreen } from './features/scanning/UserProfileScreen';
import { ReportsScreen } from './features/reports/ReportsScreen';
import { SettingsScreen } from './features/settings/SettingsScreen';
import { ProfileScreen } from './features/profile/ProfileScreen';

// Face scan screens
import { FaceScanPreparationScreen } from './features/face-scan/FaceScanPreparationScreen';
import { FaceScanCameraScreen } from './features/face-scan/FaceScanCameraScreen';
import { FaceScanResultsScreen } from './features/face-scan/FaceScanResultsScreen';

// Body scan screens
import { BodyScanScreen } from './features/body-scan/screens/BodyScanScreen';
import { BodyScanPreparationScreen } from './features/body-scan/BodyScanPreparationScreen';

// Placeholder screens - to be implemented
const ProcessingScreen = () => <div>Processing Screen</div>;
const ResultsScreen = () => <div>Results Screen</div>;

// Protected Route wrapper
const ProtectedRoute: React.FC<{ children: React.ReactNode }> = ({ children }) => {
  const isAuthenticated = localStorage.getItem('isAuthenticated') === 'true';
  const termsAccepted = localStorage.getItem('termsAccepted') === 'true';
  
  if (!isAuthenticated) {
    return <Navigate to="/login" replace />;
  }
  
  if (!termsAccepted) {
    return <Navigate to="/terms" replace />;
  }
  
  return <>{children}</>;
};

// Auth route wrapper
const AuthRoute: React.FC<{ children: React.ReactNode }> = ({ children }) => {
  const navigate = useNavigate();
  const [showLogin, setShowLogin] = React.useState(true);
  const [showTerms, setShowTerms] = React.useState(false);
  
  React.useEffect(() => {
    const isAuthenticated = localStorage.getItem('isAuthenticated') === 'true';
    const termsAccepted = localStorage.getItem('termsAccepted') === 'true';
    
    if (isAuthenticated && !termsAccepted) {
      setShowLogin(false);
      setShowTerms(true);
    } else if (isAuthenticated && termsAccepted) {
      navigate('/home');
    }
  }, [navigate]);
  
  if (window.location.pathname === '/terms' || showTerms) {
    return (
      <TermsOfServiceScreen 
        isOpen={true} 
        onClose={() => navigate('/login')} 
      />
    );
  }
  
  return (
    <LoginScreen 
      isOpen={showLogin} 
      onClose={() => setShowLogin(false)}
      onSuccess={() => {
        setShowLogin(false);
        setShowTerms(true);
      }}
    />
  );
};

// Separate component for routes to use hooks inside Router context
const AppRoutes: React.FC = () => {
  return (
    <Routes>
      {/* Initial route */}
      <Route path="/" element={<InitializingScreen />} />
      
      {/* Auth routes */}
      <Route path="/login" element={<AuthRoute><div /></AuthRoute>} />
      <Route path="/terms" element={<AuthRoute><div /></AuthRoute>} />
      
      {/* Protected routes */}
      <Route path="/home" element={
        <ProtectedRoute>
          <HomeScreen />
        </ProtectedRoute>
      } />
      
      <Route path="/reports" element={
        <ProtectedRoute>
          <ReportsScreen />
        </ProtectedRoute>
      } />
      
      <Route path="/new-scan" element={
        <ProtectedRoute>
          <NewScanScreen />
        </ProtectedRoute>
      } />
      
      <Route path="/settings" element={
        <ProtectedRoute>
          <SettingsScreen />
        </ProtectedRoute>
      } />
      
      <Route path="/profile" element={
        <ProtectedRoute>
          <ProfileScreen />
        </ProtectedRoute>
      } />
      
      {/* Scan flow routes */}
      <Route path="/scan/user-profile" element={
        <ProtectedRoute>
          <UserProfileScreen />
        </ProtectedRoute>
      } />
      
      <Route path="/scan/body/preparation" element={
        <ProtectedRoute>
          <BodyScanPreparationScreen />
        </ProtectedRoute>
      } />
      
      <Route path="/scan/body/camera" element={
        <ProtectedRoute>
          <BodyScanScreen />
        </ProtectedRoute>
      } />
      
      <Route path="/scan/face/preparation" element={
        <ProtectedRoute>
          <FaceScanPreparationScreen />
        </ProtectedRoute>
      } />
      
      <Route path="/scan/face/camera" element={
        <ProtectedRoute>
          <FaceScanCameraScreen />
        </ProtectedRoute>
      } />
      
      <Route path="/processing" element={
        <ProtectedRoute>
          <ProcessingScreen />
        </ProtectedRoute>
      } />
      
      <Route path="/results" element={
        <ProtectedRoute>
          <ResultsScreen />
        </ProtectedRoute>
      } />
      
      <Route path="/face-scan-results" element={
        <ProtectedRoute>
          <FaceScanResultsScreen />
        </ProtectedRoute>
      } />
            
      {/* Catch all */}
      <Route path="*" element={<Navigate to="/" replace />} />
    </Routes>
  );
};

function App() {
  return (
    <Provider store={store}>
      <ThemeProvider theme={theme}>
        <GlobalStyles />
        <Router>
          <AppRoutes />
        </Router>
      </ThemeProvider>
    </Provider>
  );
}

export default App;