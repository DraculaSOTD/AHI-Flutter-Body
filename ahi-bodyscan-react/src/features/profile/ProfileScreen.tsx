import React, { useState, useEffect } from 'react';
import styled from 'styled-components';
import { useNavigate } from 'react-router-dom';
import { theme } from '../../styles/theme';
import { Layout, PageHeader, PageContent } from '../../components/layout/Layout';
import { Input } from '../../components/forms/Input';
import { Select } from '../../components/forms/Select';
import { PrimaryGradientButton } from '../../components/common/Button';
import { GenderSelector } from '../../components/forms/SegmentedControl';
import { DateSelector } from '../../components/forms/DateSelector';

interface ProfileData {
  gender: string;
  height: number;
  heightFeet?: string;
  heightInches?: string;
  heightUnit: 'cm' | 'feet';
  dateOfBirth: string;
  smokingStatus: string;
  diabetesStatus: string;
  hasHypertension: string;
  takingBPMedication: string;
  lastUpdated: string;
}

const Form = styled.form`
  max-width: 600px;
  margin: 0 auto;
`;

const Section = styled.div`
  background: ${theme.colors.backgroundWhite};
  border-radius: ${theme.dimensions.radiusLarge};
  padding: ${theme.dimensions.paddingLarge};
  margin-bottom: ${theme.dimensions.paddingLarge};
  box-shadow: ${theme.shadows.small};
`;

const SectionTitle = styled.h3`
  font-size: ${theme.typography.headingSmall.fontSize};
  font-weight: ${theme.typography.headingSmall.fontWeight};
  color: ${theme.colors.textPrimary};
  margin-bottom: ${theme.dimensions.paddingLarge};
`;

const FormRow = styled.div`
  display: grid;
  gap: ${theme.dimensions.paddingLarge};
  align-items: start;
  
  @media (min-width: ${theme.breakpoints.mobile}) {
    grid-template-columns: 1fr 1fr;
  }
  
  & > div {
    min-height: 0;
    display: flex;
    flex-direction: column;
  }
`;

const HeightInputWrapper = styled.div`
  position: relative;
`;

const HeightInputGroup = styled.div`
  display: flex;
  gap: ${theme.dimensions.paddingSmall};
  align-items: flex-end;
`;

const Label = styled.label`
  font-size: ${theme.typography.bodyMedium.fontSize};
  font-weight: 500;
  color: ${theme.colors.textPrimary};
  margin-bottom: ${theme.dimensions.paddingSmall};
  display: block;
`;

const StyledSelect = styled.select`
  width: 100%;
  padding: ${theme.dimensions.paddingMedium};
  font-size: ${theme.typography.bodyMedium.fontSize};
  color: ${theme.colors.textPrimary};
  background: ${theme.colors.backgroundWhite};
  border: 1px solid ${theme.colors.borderLight};
  border-radius: ${theme.dimensions.radiusMedium};
  cursor: pointer;
  transition: all ${theme.transitions.fast};
  
  &:focus {
    outline: none;
    border-color: ${theme.colors.primaryBlue};
  }
  
  &:hover {
    border-color: ${theme.colors.borderAccent};
  }
`;

const UnitToggle = styled.div`
  display: flex;
  align-items: center;
  gap: ${theme.dimensions.paddingSmall};
  margin-top: ${theme.dimensions.paddingSmall};
  
  label {
    display: flex;
    align-items: center;
    gap: ${theme.dimensions.paddingXSmall};
    cursor: pointer;
    font-size: ${theme.typography.bodySmall.fontSize};
    color: ${theme.colors.textSecondary};
    
    input[type="radio"] {
      accent-color: ${theme.colors.primaryBlue};
    }
  }
`;

const SaveButtonWrapper = styled.div`
  display: flex;
  justify-content: center;
  margin-top: ${theme.dimensions.paddingXLarge};
`;

const SuccessMessage = styled.div`
  background: ${theme.colors.success}20;
  color: ${theme.colors.success};
  padding: ${theme.dimensions.paddingMedium};
  border-radius: ${theme.dimensions.radiusMedium};
  text-align: center;
  margin-bottom: ${theme.dimensions.paddingLarge};
  font-weight: 500;
`;

export const ProfileScreen: React.FC = () => {
  const navigate = useNavigate();
  const [showSuccess, setShowSuccess] = useState(false);
  const [formData, setFormData] = useState<ProfileData>({
    gender: '',
    height: 175,
    heightFeet: '5',
    heightInches: '9',
    heightUnit: 'cm',
    dateOfBirth: '',
    smokingStatus: '',
    diabetesStatus: '',
    hasHypertension: '',
    takingBPMedication: '',
    lastUpdated: ''
  });

  // Load existing data on mount
  useEffect(() => {
    const savedData = localStorage.getItem('userProfile');
    if (savedData) {
      const data = JSON.parse(savedData);
      // Handle backward compatibility
      if (data.biologicalSex && !data.gender) {
        data.gender = data.biologicalSex;
      }
      // Convert height if needed
      if (data.heightUnit === 'feet' && data.height) {
        const totalInches = data.height / 2.54;
        const feet = Math.floor(totalInches / 12);
        const inches = Math.round(totalInches % 12);
        data.heightFeet = feet.toString();
        data.heightInches = inches.toString();
      }
      setFormData(data);
    }
  }, []);

  const handleInputChange = (field: keyof ProfileData, value: any) => {
    setFormData(prev => ({ ...prev, [field]: value }));
  };

  const handleHeightUnitChange = (unit: 'cm' | 'feet') => {
    if (unit === formData.heightUnit) return;
    
    if (unit === 'feet') {
      // Convert cm to feet and inches
      const totalInches = formData.height / 2.54;
      const feet = Math.floor(totalInches / 12);
      const inches = Math.round(totalInches % 12);
      setFormData(prev => ({ 
        ...prev, 
        heightFeet: feet.toString(),
        heightInches: inches.toString(),
        heightUnit: unit 
      }));
    } else {
      // Convert feet and inches to cm
      const feet = parseFloat(formData.heightFeet || '0');
      const inches = parseFloat(formData.heightInches || '0');
      const cm = Math.round((feet * 30.48) + (inches * 2.54));
      setFormData(prev => ({ ...prev, height: cm, heightUnit: unit }));
    }
  };

  const handleSave = (e: React.FormEvent) => {
    e.preventDefault();
    
    // Calculate height in cm for storage
    let heightInCm = formData.height;
    if (formData.heightUnit === 'feet') {
      const feet = parseFloat(formData.heightFeet || '0');
      const inches = parseFloat(formData.heightInches || '0');
      heightInCm = (feet * 30.48) + (inches * 2.54);
    }
    
    const dataToSave = {
      ...formData,
      height: heightInCm,
      lastUpdated: new Date().toISOString()
    };
    
    localStorage.setItem('userProfile', JSON.stringify(dataToSave));
    setShowSuccess(true);
    
    // Hide success message after 3 seconds
    setTimeout(() => {
      setShowSuccess(false);
    }, 3000);
  };

  // Options are now handled inline in the form

  return (
    <Layout>
      <PageHeader 
        title="Profile"
        subtitle="Manage your health information"
        showBackButton
        onBack={() => navigate('/settings')}
      />
      <PageContent>
        {showSuccess && (
          <SuccessMessage>
            Profile saved successfully!
          </SuccessMessage>
        )}
        
        <Form onSubmit={handleSave}>
          <Section>
            <SectionTitle>Basic Information</SectionTitle>
            
            <GenderSelector
              value={formData.gender}
              onChange={(value) => handleInputChange('gender', value)}
              label="Biological Sex"
            />
            
            <DateSelector
              value={formData.dateOfBirth}
              onChange={(date) => handleInputChange('dateOfBirth', date)}
              label="Date of Birth"
              required
            />
            
            <HeightInputWrapper>
              <Label>Height</Label>
              {formData.heightUnit === 'cm' ? (
                <Input
                  type="number"
                  value={formData.height}
                  onChange={(e) => handleInputChange('height', parseFloat(e.target.value) || 0)}
                  placeholder="Height in centimeters"
                  step={1}
                  min={100}
                  max={250}
                  required
                />
              ) : (
                <HeightInputGroup>
                  <Input
                    type="number"
                    min="0"
                    max="8"
                    placeholder="Feet"
                    value={formData.heightFeet || ''}
                    onChange={(e) => handleInputChange('heightFeet', e.target.value)}
                    required
                    style={{ flex: 1 }}
                  />
                  <Input
                    type="number"
                    min="0"
                    max="11"
                    placeholder="Inches"
                    value={formData.heightInches || ''}
                    onChange={(e) => handleInputChange('heightInches', e.target.value)}
                    required
                    style={{ flex: 1 }}
                  />
                </HeightInputGroup>
              )}
              <UnitToggle>
                <label>
                  <input
                    type="radio"
                    name="heightUnit"
                    checked={formData.heightUnit === 'cm'}
                    onChange={() => handleHeightUnitChange('cm')}
                  />
                  cm
                </label>
                <label>
                  <input
                    type="radio"
                    name="heightUnit"
                    checked={formData.heightUnit === 'feet'}
                    onChange={() => handleHeightUnitChange('feet')}
                  />
                  ft/in
                </label>
              </UnitToggle>
            </HeightInputWrapper>
          </Section>

          <Section>
            <SectionTitle>Health History</SectionTitle>
            
            <FormRow>
              <div>
                <Label>Do you smoke?</Label>
                <StyledSelect
                  value={formData.smokingStatus}
                  onChange={(e) => handleInputChange('smokingStatus', e.target.value)}
                  required
                >
                  <option value="">Select...</option>
                  <option value="never">Never smoked</option>
                  <option value="past">In the past</option>
                  <option value="current">Yes, currently</option>
                </StyledSelect>
              </div>
              
              <div>
                <Label>Do you have diabetes?</Label>
                <StyledSelect
                  value={formData.diabetesStatus}
                  onChange={(e) => handleInputChange('diabetesStatus', e.target.value)}
                  required
                >
                  <option value="">Select...</option>
                  <option value="no">No</option>
                  <option value="type1">Type 1</option>
                  <option value="type2">Type 2</option>
                </StyledSelect>
              </div>
            </FormRow>
            
            <FormRow>
              <div>
                <Label>Do you have hypertension?</Label>
                <StyledSelect
                  value={formData.hasHypertension}
                  onChange={(e) => handleInputChange('hasHypertension', e.target.value)}
                  required
                >
                  <option value="">Select...</option>
                  <option value="no">No</option>
                  <option value="yes">Yes</option>
                </StyledSelect>
              </div>
              
              <div>
                <Label>Are you taking blood pressure medication?</Label>
                <StyledSelect
                  value={formData.takingBPMedication}
                  onChange={(e) => handleInputChange('takingBPMedication', e.target.value)}
                  required
                >
                  <option value="">Select...</option>
                  <option value="no">No</option>
                  <option value="yes">Yes</option>
                </StyledSelect>
              </div>
            </FormRow>
          </Section>

          <SaveButtonWrapper>
            <PrimaryGradientButton type="submit" size="large">
              Save Profile
            </PrimaryGradientButton>
          </SaveButtonWrapper>
        </Form>
      </PageContent>
    </Layout>
  );
};