from controllers.sessionController import get_session

def get_healthscore(bha_data: dict):    
    
    # Extract relevant values with safe defaults
    age = bha_data.get("outputs", {}).get("yr_raw_age", 0)
    resting_heart_rate = float(bha_data.get("inputs", {}).get("bpm_ent_restingHeartRate", 0))
    max_heart_rate = float(bha_data.get("outputs", {}).get("bpm_raw_maxHR", 0))	
    height = float(bha_data.get("inputs", {}).get("cm_ent_height", 0))
    weight = float(bha_data.get("inputs", {}).get("kg_ent_weight", 0))
    sex =  bha_data.get("inputs", {}).get("enum_ent_sex", "").strip()
	
    # Ensure valid data before calculation
    if resting_heart_rate > 0 and max_heart_rate > 0:
        cardiorespiratory_fitness = round(15 * (max_heart_rate / resting_heart_rate), 2)
        print(f"Cardiorespiratory Fitness: {cardiorespiratory_fitness}")
    else:
        print("Error: Invalid resting heart rate or max heart rate values.")
        cardiorespiratory_fitness = None  # Handle error case   

    # #MinM (Fitness Boundaries for Males)
    if age > 0:
        fitness_boundaries_male_min = round(34.685 - (0.204606 * age), 6)
        print(f"Fitness Boundaries Male Min: {fitness_boundaries_male_min}")
        
        # #MaxM (Fitness Boundaries for Males)
        fitness_boundaries_male_max = round(67.2876 - (0.419273 * age), 6)
        print(f"Fitness Boundaries Male Max: {fitness_boundaries_male_max}")

        # #MinM (Fitness Boundaries for Females)
        fitness_boundaries_female_min = round(36.0509 - (0.241818 * age), 6)
        print(f"Fitness Boundaries Female Min: {fitness_boundaries_female_min}")

        # #MaxM (Fitness Boundaries for Females)
        fitness_boundaries_female_max = round(69.3571 - (0.462182 * age) , 6)
        print(f"Fitness Boundaries Female Max: {fitness_boundaries_female_max}")
    else:
        print("Error: Age must be greater than 0.")  
        fitness_boundaries_male_min = None  
    

    #FitScale (FS) for Males
    #FS = (F - (34.685 - 0.204606 × A)) / ((67.2876 - 0.419273 × A) - (34.685 - 0.204606 × A)) for males         
    #If FS < 0, set FS = 0. If FS > 1, set FS = 1.
    # fitscale_males = (cardiorespiratory_fitness - fitness_boundaries_male_min) / (fitness_boundaries_male_max - fitness_boundaries_male_min)
    # if None not in (cardiorespiratory_fitness, fitness_boundaries_male_min, fitness_boundaries_male_max):
    fitscale_males = (
    (cardiorespiratory_fitness or 0) - (fitness_boundaries_male_min or 0)) / ((fitness_boundaries_male_max or 1) - (fitness_boundaries_male_min or 0))  # Avoid division by zero
    print(f"Fit Scale Males: {fitscale_males}")
    
    if fitscale_males < 0:
        fitscale_males = 0
    elif fitscale_males > 1:
        fitscale_males = 1  
        
    #FitScale (FS) for Males
    #FS = (F - (34.685 - 0.204606 × A)) / ((67.2876 - 0.419273 × A) - (34.685 - 0.204606 × A)) for males         
    #If FS < 0, set FS = 0. If FS > 1, set FS = 1.
    fitscale_females = (
    (cardiorespiratory_fitness or 0) - (fitness_boundaries_female_min or 0)) / ((fitness_boundaries_female_max or 1) - (fitness_boundaries_female_min or 0))  # Avoid division by zero

    print(f"Fit Scale Females: {fitscale_females}")
    
    if fitscale_females < 0:
            fitscale_females = 0
    elif fitscale_females > 1:
        fitscale_females = 1             
        
    bmi = 0 if not weight or not height or height <= 0 else weight / ((height / 100) ** 2)
    
    # #bmi75 = 23.9434 + 0.285262 * data.age - 0.00250701 * data.age ** 2
    bmi75 = (
        0 if not age else 23.9434 + 0.285262 * age - 0.00250701 * age ** 2
    )
    
    bmi50 = (
        0 if not age else 20.5918 + 0.260177 * age - 0.00217537 * age ** 2
    )
        
    bmi25 = (
        0 if not age else 18.1103 + 0.221603 * age -  0.00176849 * age ** 2
    )

    bmisd = (bmi75 - bmi25) / 1.349 if bmi75 is not None and bmi25 is not None else 0
    
    bmihigh = bmi50 + 1.96 * bmisd if bmisd is not None else 0

    bmilow = 18.5        
    
    #To cater for the edge BMI's - high and low
    #bmiscale = 0 if (1 - ((bmi - bmilow) / (bmihigh - bmilow))) < 0 else (1 if (1 - ((bmi - bmilow) / (bmihigh - bmilow))) > 1 else max(0, min(1, 1 - ((bmi - bmilow) / (bmihigh - bmilow)))))
    if bmi < bmi50:
            if bmi < 18:
                if bmi < 16:
                    bmiscale = 0
                else:
                    bmiscale =  0.5 * bmi - 8
            else:
                bmiscale = 1
    else:
            value = 1 - ((bmi - bmi50) / (bmihigh - bmi50))
            if value < 0:
                bmiscale = 0 
            else:
                bmiscale = value
    
    risk_function = 0
    if sex == 'male':
        risk_function = (2.53867 - 1.26934 * bmiscale) * (1.57563 - 0.787813 * fitscale_males) if None not in (bmiscale, fitscale_males) else 0        
    elif sex == 'female':
        risk_function = (2.53867 - 1.26934 * bmiscale) * (1.57563 - 0.787813 * fitscale_females) if None not in (bmiscale, fitscale_females) else 0
                    

    # Calculate Health Score (example formula)
    #sex_factor = 5 if data.biologicalSex.lower() == "male" else -161
    health_score = 100 * ((4 - risk_function) / 3) if risk_function is not None else 0
    
    return round(health_score)        
