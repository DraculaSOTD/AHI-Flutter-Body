# Deploy Full Lambda Metrics Update

## What's New
All Lambda health metrics are now displayed on the Face Scan Results screen:

### New Metrics Added:
- **BMI Value**: Actual value displayed (e.g., 22.9 kg/m²)
- **Cholesterol Panel**: Total, LDL, HDL, Triglycerides with values
- **Fitness Score**: VO2 Max estimate  
- **Cardiovascular Metrics**: 
  - Arterial Stiffness (baPWV)
  - 10-year CVD Risk percentage
  - Framingham Score

## Deploy Commands

Run these commands to deploy:

```bash
# 1. Deploy the build
sudo ./deploy-to-web.sh

# 2. Ensure nginx is reloaded (if not already done)
sudo systemctl reload nginx
```

## What Users Will See

The Face Scan Results screen now has three sections:

1. **Main Vitals Grid**: Heart rate, HRV, BP, RR, SpO2, Stress, Heart Age, Biological Age, BMI
2. **Metabolic Panel**: All cholesterol values with risk indicators
3. **Advanced Metrics**: Fitness score, arterial stiffness, CVD risk scores

All values are color-coded based on risk levels from Lambda.