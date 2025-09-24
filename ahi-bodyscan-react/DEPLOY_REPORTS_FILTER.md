# Deploy Reports Health Assessment Filter Update

## What's New
Added "Health Assessment" filter to the Reports page that shows all Face Scans containing Lambda health data.

### Features Added:
1. **New Filter Button**: "Health Assessment" filter in Reports page
2. **Lambda Metrics Display**: Shows all Lambda health metrics when available:
   - Health Score
   - BMI with value display
   - Heart Age  
   - Cholesterol Panel (Total, LDL, HDL)
   - Fitness Score
   - 10-Year CVD Risk

### How It Works:
- The "Health Assessment" filter shows only Face Scans that have Lambda data
- Face scans with Lambda data will display additional health metrics automatically
- All metrics are color-coded based on risk levels

## Deploy Commands

Run these commands to deploy:

```bash
# 1. Build is already complete
# The build was successful and ready to deploy

# 2. Deploy to web (requires sudo)
sudo bash -c "cp -r /home/calvin/Websites/AHI-new/ahi-bodyscan-react/build/* /srv/http/ahi-bodyscan/ && chown -R http:http /srv/http/ahi-bodyscan"

# 3. Verify deployment
curl -I http://localhost/ahi-bodyscan/
```

## What Users Will See

### Reports Page:
1. New "Health Assessment" filter button alongside existing filters
2. When selected, only shows Face Scans with Lambda health data
3. Each assessment card displays all available Lambda metrics
4. Metrics are color-coded for easy interpretation

### Filter Options:
- **All Assessments**: Shows both Body and Face scans
- **Body Scans**: Shows only Body scan assessments  
- **Face Scans**: Shows only Face scan assessments
- **Health Assessment**: Shows only Face scans with Lambda health data (NEW)

## Testing
After deployment, test by:
1. Navigate to Reports page
2. Click "Health Assessment" filter
3. Verify only Face scans with Lambda data are shown
4. Verify all Lambda metrics are displayed in the assessment cards