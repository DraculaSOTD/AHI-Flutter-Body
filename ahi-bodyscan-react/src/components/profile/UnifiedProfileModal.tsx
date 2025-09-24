import React, { useState } from 'react';
import styled from 'styled-components';
import { theme } from '../../styles/theme';
import { Modal, ModalBody, ModalFooter } from '../common/Modal';
import { Input } from '../forms/Input';
import { PrimaryGradientButton } from '../common/Button';
import { GenderSelector } from '../forms/SegmentedControl';
import { DateSelector } from '../forms/DateSelector';

export interface UnifiedProfileData {
  name?: string;
  gender: string;
  dateOfBirth: string;
  height: number;
  weight: number;
  heightUnit: 'cm' | 'ft';
  weightUnit: 'kg' | 'lbs';
  exerciseLevel: 'inactive' | 'exercise10Mins' | 'exercise20to60Mins' | 'exercise1to3Hours' | 'exerciseOver3Hours';
  chronicMedication: 'none' | 'oneOrTwoDiseases' | 'threeOrMoreDiseases';
  smokingStatus?: string;
  diabetesStatus?: string;
  hasHypertension?: string;
  takingBPMedication?: string;
}

interface UnifiedProfileModalProps {
  isOpen: boolean;
  onClose: () => void;
  onComplete: (profileData: UnifiedProfileData) => void;
  title?: string;
  subtitle?: string;
  submitButtonText?: string;
  initialData?: Partial<UnifiedProfileData>;
  scanType?: 'body' | 'face';
}

const ContentWrapper = styled.div`
  padding: ${theme.dimensions.paddingLarge} 0;
`;

const Message = styled.p`
  font-size: ${theme.typography.bodyLarge.fontSize};
  color: ${theme.colors.textSecondary};
  text-align: center;
  margin-bottom: ${theme.dimensions.paddingXLarge};
  line-height: 1.6;
`;

const Form = styled.form`
  display: flex;
  flex-direction: column;
  gap: ${theme.dimensions.paddingLarge};
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

const Label = styled.label`
  font-size: ${theme.typography.bodyMedium.fontSize};
  font-weight: 500;
  color: ${theme.colors.textPrimary};
  margin-bottom: ${theme.dimensions.paddingSmall};
  display: block;
`;

const UnitToggle = styled.button<{ active: boolean }>`
  padding: ${theme.dimensions.paddingSmall} ${theme.dimensions.paddingMedium};
  border: 1px solid ${theme.colors.borderLight};
  background: ${props => props.active ? theme.colors.primaryPurple : theme.colors.backgroundWhite};
  color: ${props => props.active ? theme.colors.textWhite : theme.colors.textSecondary};
  border-radius: ${theme.dimensions.radiusSmall};
  font-size: ${theme.typography.bodySmall.fontSize};
  cursor: pointer;
  transition: all ${theme.transitions.fast};
  
  &:hover {
    background: ${props => props.active ? theme.colors.primaryPurple : theme.colors.backgroundLight};
  }
`;

const UnitToggleContainer = styled.div`
  display: inline-flex;
  gap: ${theme.dimensions.paddingSmall};
  margin-bottom: ${theme.dimensions.paddingSmall};
`;

const HeightInputGroup = styled.div`
  display: flex;
  gap: ${theme.dimensions.paddingSmall};
  align-items: flex-end;
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
    border-color: ${theme.colors.primaryPurple};
  }
  
  &:hover {
    border-color: ${theme.colors.borderAccent};
  }
`;

export const UnifiedProfileModal: React.FC<UnifiedProfileModalProps> = ({
  isOpen,
  onClose,
  onComplete,
  title = "Complete Your Profile",
  subtitle,
  submitButtonText = "Save Profile & Continue",
  initialData = {},
  scanType
}) => {
  const [loading, setLoading] = useState(false);
  const [formData, setFormData] = useState<UnifiedProfileData>({
    name: initialData.name || '',
    gender: initialData.gender || '',
    dateOfBirth: initialData.dateOfBirth || '',
    height: initialData.height || 175,
    weight: initialData.weight || 70,
    heightUnit: initialData.heightUnit || 'cm',
    weightUnit: initialData.weightUnit || 'kg',
    exerciseLevel: initialData.exerciseLevel || 'inactive',
    chronicMedication: initialData.chronicMedication || 'none',
    smokingStatus: initialData.smokingStatus || 'never',
    diabetesStatus: initialData.diabetesStatus || 'no',
    hasHypertension: initialData.hasHypertension || 'no',
    takingBPMedication: initialData.takingBPMedication || 'no'
  });
  const [heightFeet, setHeightFeet] = useState('');
  const [heightInches, setHeightInches] = useState('');

  // Update formData when initialData changes meaningfully (when editing different profiles)
  React.useEffect(() => {
    // Only update if initialData has actual content (not just an empty object)
    if (initialData.name || initialData.gender || initialData.dateOfBirth) {
      setFormData({
        name: initialData.name || '',
        gender: initialData.gender || '',
        dateOfBirth: initialData.dateOfBirth || '',
        height: initialData.height || 175,
        weight: initialData.weight || 70,
        heightUnit: initialData.heightUnit || 'cm',
        weightUnit: initialData.weightUnit || 'kg',
        exerciseLevel: initialData.exerciseLevel || 'inactive',
        chronicMedication: initialData.chronicMedication || 'none',
        smokingStatus: initialData.smokingStatus || 'never',
        diabetesStatus: initialData.diabetesStatus || 'no',
        hasHypertension: initialData.hasHypertension || 'no',
        takingBPMedication: initialData.takingBPMedication || 'no'
      });
      
      // Also update feet/inches if needed
      if (initialData.heightUnit === 'ft' && initialData.height) {
        const totalInches = initialData.height / 2.54;
        const feet = Math.floor(totalInches / 12);
        const inches = Math.round(totalInches % 12);
        setHeightFeet(feet.toString());
        setHeightInches(inches.toString());
      }
    }
  }, [initialData.name, initialData.gender, initialData.dateOfBirth, initialData.height, initialData.weight]);

  const calculateAge = (dob: string) => {
    const birthDate = new Date(dob);
    const today = new Date();
    let age = today.getFullYear() - birthDate.getFullYear();
    const monthDiff = today.getMonth() - birthDate.getMonth();
    if (monthDiff < 0 || (monthDiff === 0 && today.getDate() < birthDate.getDate())) {
      age--;
    }
    return age;
  };

  const handleSubmit = async (e: React.FormEvent) => {
    e.preventDefault();
    setLoading(true);

    try {
      // Calculate height in cm if using feet/inches
      let heightInCm = formData.height;
      if (formData.heightUnit === 'ft') {
        const feet = parseFloat(heightFeet) || 0;
        const inches = parseFloat(heightInches) || 0;
        heightInCm = (feet * 30.48) + (inches * 2.54);
      }

      // Calculate weight in kg if using lbs
      let weightInKg = formData.weight;
      if (formData.weightUnit === 'lbs') {
        weightInKg = formData.weight * 0.453592;
      }

      // Create profile data with normalized units
      const profileData = {
        ...formData,
        height: heightInCm,
        weight: weightInKg
      };

      // Save basic profile data for quick access
      const currentUserProfile = {
        userId: Date.now().toString(),
        name: formData.name || 'User',
        age: calculateAge(formData.dateOfBirth),
        gender: formData.gender,
        email: localStorage.getItem('userEmail') || 'user@example.com',
        exerciseLevel: formData.exerciseLevel,
        chronicMedication: formData.chronicMedication,
        smokingStatus: formData.smokingStatus,
        diabetesStatus: formData.diabetesStatus,
        hasHypertension: formData.hasHypertension,
        takingBPMedication: formData.takingBPMedication
      };

      localStorage.setItem('currentUserProfile', JSON.stringify(currentUserProfile));
      localStorage.setItem('userHeight', String(heightInCm));
      localStorage.setItem('userWeight', String(weightInKg));
      localStorage.setItem('userExerciseLevel', formData.exerciseLevel);
      localStorage.setItem('userChronicMedication', formData.chronicMedication);

      // Save selected profile for quick access
      const selectedProfile = {
        id: currentUserProfile.userId,
        name: formData.name || 'User',
        age: calculateAge(formData.dateOfBirth),
        gender: formData.gender,
        height: heightInCm,
        weight: weightInKg,
        dateOfBirth: formData.dateOfBirth,
        exerciseLevel: formData.exerciseLevel,
        chronicMedication: formData.chronicMedication,
        smokingStatus: formData.smokingStatus,
        diabetesStatus: formData.diabetesStatus,
        hasHypertension: formData.hasHypertension,
        takingBPMedication: formData.takingBPMedication
      };
      localStorage.setItem('selectedProfile', JSON.stringify(selectedProfile));

      // Call onComplete with the profile data
      onComplete(profileData);
    } catch (error) {
      console.error('Error saving profile:', error);
    } finally {
      setLoading(false);
    }
  };

  const isFormValid = () => {
    if (!formData.name || !formData.gender || !formData.dateOfBirth) return false;
    
    if (formData.heightUnit === 'cm') {
      if (!formData.height || formData.height <= 0) return false;
    } else {
      if (!heightFeet || parseFloat(heightFeet) <= 0) return false;
    }
    
    if (!formData.weight || formData.weight <= 0) return false;
    if (!formData.exerciseLevel) return false;
    if (!formData.chronicMedication) return false;
    
    return true;
  };

  const defaultSubtitle = scanType 
    ? `To start a ${scanType} scan, we need some basic information about you. This helps us provide accurate measurements and health insights.`
    : 'Please provide your basic information to continue.';

  return (
    <Modal isOpen={isOpen} onClose={onClose} title={title} size="medium">
      <ModalBody>
        <ContentWrapper>
          {(subtitle || defaultSubtitle) && (
            <Message>{subtitle || defaultSubtitle}</Message>
          )}

          <Form onSubmit={handleSubmit}>
            <Input
              type="text"
              label="Profile Name"
              value={formData.name || ''}
              onChange={(e) => {
                setFormData({ ...formData, name: e.target.value });
              }}
              placeholder="Enter profile name (e.g., John, Mom, Dad)"
              required
            />

            <GenderSelector
              value={formData.gender}
              onChange={(value) => setFormData({ ...formData, gender: value })}
              label="Biological Sex"
            />

            <DateSelector
              value={formData.dateOfBirth}
              onChange={(date) => setFormData({ ...formData, dateOfBirth: date })}
              label="Date of Birth"
              required
            />

            <FormRow>
              <div>
                <Label>
                  Height
                  <UnitToggleContainer style={{ float: 'right' }}>
                    <UnitToggle 
                      type="button"
                      active={formData.heightUnit === 'cm'} 
                      onClick={(e) => {
                        e.preventDefault();
                        if (formData.heightUnit === 'ft') {
                          const feet = parseFloat(heightFeet) || 0;
                          const inches = parseFloat(heightInches) || 0;
                          const cm = Math.round((feet * 30.48) + (inches * 2.54));
                          setFormData({ ...formData, height: cm, heightUnit: 'cm' });
                        }
                      }}
                    >
                      cm
                    </UnitToggle>
                    <UnitToggle 
                      type="button"
                      active={formData.heightUnit === 'ft'} 
                      onClick={(e) => {
                        e.preventDefault();
                        if (formData.heightUnit === 'cm') {
                          const totalInches = formData.height / 2.54;
                          const feet = Math.floor(totalInches / 12);
                          const inches = Math.round(totalInches % 12);
                          setHeightFeet(feet.toString());
                          setHeightInches(inches.toString());
                          setFormData({ ...formData, heightUnit: 'ft' });
                        }
                      }}
                    >
                      ft/in
                    </UnitToggle>
                  </UnitToggleContainer>
                </Label>
                {formData.heightUnit === 'cm' ? (
                  <Input
                    type="number"
                    min="50"
                    max="250"
                    placeholder="Height in centimeters"
                    value={formData.height || ''}
                    onChange={(e) => setFormData({ ...formData, height: parseFloat(e.target.value) || 0 })}
                    required
                  />
                ) : (
                  <HeightInputGroup>
                    <div style={{ flex: 1 }}>
                      <Input
                        type="number"
                        min="1"
                        max="8"
                        placeholder="Feet"
                        value={heightFeet}
                        onChange={(e) => setHeightFeet(e.target.value)}
                        required
                      />
                    </div>
                    <div style={{ flex: 1 }}>
                      <Input
                        type="number"
                        min="0"
                        max="11"
                        placeholder="Inches"
                        value={heightInches}
                        onChange={(e) => setHeightInches(e.target.value)}
                      />
                    </div>
                  </HeightInputGroup>
                )}
              </div>

              <div>
                <Label>
                  Weight
                  <UnitToggleContainer style={{ float: 'right' }}>
                    <UnitToggle 
                      type="button"
                      active={formData.weightUnit === 'kg'} 
                      onClick={(e) => {
                        e.preventDefault();
                        if (formData.weightUnit === 'lbs') {
                          const kg = Math.round(formData.weight * 0.453592);
                          setFormData({ ...formData, weight: kg, weightUnit: 'kg' });
                        }
                      }}
                    >
                      kg
                    </UnitToggle>
                    <UnitToggle 
                      type="button"
                      active={formData.weightUnit === 'lbs'} 
                      onClick={(e) => {
                        e.preventDefault();
                        if (formData.weightUnit === 'kg') {
                          const lbs = Math.round(formData.weight * 2.20462);
                          setFormData({ ...formData, weight: lbs, weightUnit: 'lbs' });
                        }
                      }}
                    >
                      lbs
                    </UnitToggle>
                  </UnitToggleContainer>
                </Label>
                <Input
                  type="number"
                  min="20"
                  max={formData.weightUnit === 'kg' ? "300" : "660"}
                  step="0.1"
                  placeholder={`Weight in ${formData.weightUnit}`}
                  value={formData.weight || ''}
                  onChange={(e) => setFormData({ ...formData, weight: parseFloat(e.target.value) || 0 })}
                  required
                />
              </div>
            </FormRow>

            <FormRow>
              <div>
                <Label>How long do you exercise for on average?</Label>
                <StyledSelect
                  value={formData.exerciseLevel}
                  onChange={(e) => setFormData({ ...formData, exerciseLevel: e.target.value as any })}
                  required
                >
                  <option value="inactive">I don't exercise</option>
                  <option value="exercise10Mins">10 minutes</option>
                  <option value="exercise20to60Mins">20 - 60 minutes</option>
                  <option value="exercise1to3Hours">1 - 3 hours</option>
                  <option value="exerciseOver3Hours">Over 3 hours</option>
                </StyledSelect>
              </div>

              <div>
                <Label>Do you take chronic medication?</Label>
                <StyledSelect
                  value={formData.chronicMedication}
                  onChange={(e) => setFormData({ ...formData, chronicMedication: e.target.value as any })}
                  required
                >
                  <option value="none">No</option>
                  <option value="oneOrTwoDiseases">Yes, for 1-2 chronic conditions</option>
                  <option value="threeOrMoreDiseases">Yes, for 3+ chronic conditions</option>
                </StyledSelect>
              </div>
            </FormRow>

            <FormRow>
              <div>
                <Label>Do you smoke?</Label>
                <StyledSelect
                  value={formData.smokingStatus}
                  onChange={(e) => setFormData({ ...formData, smokingStatus: e.target.value })}
                  required
                >
                  <option value="never">Never smoked</option>
                  <option value="former">In the past</option>
                  <option value="current">Currently</option>
                </StyledSelect>
              </div>

              <div>
                <Label>Do you have diabetes?</Label>
                <StyledSelect
                  value={formData.diabetesStatus}
                  onChange={(e) => setFormData({ ...formData, diabetesStatus: e.target.value })}
                  required
                >
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
                  onChange={(e) => setFormData({ ...formData, hasHypertension: e.target.value })}
                  required
                >
                  <option value="no">No</option>
                  <option value="yes">Yes</option>
                </StyledSelect>
              </div>

              <div>
                <Label>Are you taking blood pressure medication?</Label>
                <StyledSelect
                  value={formData.takingBPMedication}
                  onChange={(e) => setFormData({ ...formData, takingBPMedication: e.target.value })}
                  required
                >
                  <option value="no">No</option>
                  <option value="yes">Yes</option>
                </StyledSelect>
              </div>
            </FormRow>
          </Form>
        </ContentWrapper>
      </ModalBody>

      <ModalFooter>
        <PrimaryGradientButton
          type="submit"
          onClick={handleSubmit}
          disabled={!isFormValid() || loading}
          loading={loading}
          fullWidth
        >
          {submitButtonText}
        </PrimaryGradientButton>
      </ModalFooter>
    </Modal>
  );
};