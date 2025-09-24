# Unified Profile Modal and Lambda Integration Update

## Changes Made

### 1. Created Unified Profile Modal
- **New Component**: `/src/components/profile/UnifiedProfileModal.tsx`
- Single consistent UI for all profile creation/editing scenarios
- All profile popups now have identical fields and layout

### 2. Enhanced Profile Fields
**New Required Fields Added:**
- **Exercise Level**: 
  - "I don't exercise" = `none`
  - "10 minutes" = `exercise10Mins`
  - "20-60 minutes" = `exercise20to60Mins`
  - "1-3 hours" = `exercise1to3Hours`
  - "Over 3 hours" = `exerciseOver3Hours`

- **Chronic Medication**:
  - "No" = `none`
  - "Yes, for 1-2 chronic conditions" = `oneOrTwoDiseases`
  - "Yes, for 3+ chronic conditions" = `threeOrMoreDiseases`

### 3. Unit Conversions for Lambda
The Lambda function now properly receives:
- **Height**: Always in centimeters (cm)
- **Weight**: Always in kilograms (kg)

Conversions handled automatically:
- Feet/inches → cm: `(feet × 30.48) + (inches × 2.54)`
- Pounds → kg: `pounds × 0.453592`

### 4. Updated Components
All profile modals now use the unified structure:
- `ProfileRequiredModal` - For first-time scan users
- `ProfileCreationModal` - For new users
- `HealthHistoryConfirmationModal` - For profile updates
- `ProfileScreen` - For editing profiles
- `UserProfileScreen` - For managing profiles

### 5. Lambda Integration Updates
- `BHALambdaService` now handles unit conversions properly
- Face scan passes exercise level and chronic medication to Lambda
- All profile data is normalized before Lambda submission

## Deploy Commands

```bash
# Build is complete and ready
# Deploy to production:
sudo bash -c "cp -r /home/calvin/Websites/AHI-new/ahi-bodyscan-react/build/* /srv/http/ahi-bodyscan/ && chown -R http:http /srv/http/ahi-bodyscan"

# Verify deployment:
curl -I http://localhost/ahi-bodyscan/
```

## What Users Will See

### Consistent Profile Form
All profile popups now display:
1. Biological Sex (Male/Female)
2. Date of Birth
3. Height (with cm/ft toggle)
4. Weight (with kg/lbs toggle)
5. Exercise Level (dropdown)
6. Chronic Medication (dropdown)

### Key Benefits
- **Consistency**: Same UI everywhere profiles are edited
- **Lambda Accuracy**: Proper units ensure accurate health assessments
- **Complete Data**: All required health data collected upfront
- **User-Friendly**: Toggle between metric/imperial units

## Testing Checklist
- [ ] Create new profile - all fields present
- [ ] Edit existing profile - all fields editable
- [ ] Face scan - Lambda receives correct units
- [ ] Body scan - profile modal shows all fields
- [ ] Unit conversions work both ways (metric ↔ imperial)