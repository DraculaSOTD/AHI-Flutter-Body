from controllers.sessionController import get_session
from hypymedia import (
    a_,
    div_,
    h1_,
    h2_,
    h3_,
    header_,
    main_,
    p_,    
    section_,
    span_,
    svg_,
    title_,
    button_,
    path_,
    link_,
    style_
)
from hypymedia.main import element
from views.template import template
from datetime import datetime


def content_model(session_id: str):
    session = get_session(session_id)

    return content(session)


def get_risk(risk_in: str):
    risks = {"low": "bg-green-500", "medium": "bg-orange-400", "high": "bg-red-500"}

    risk_class = risks.get(risk_in.lower(), "")

    risk = "moderate" if risk_in == "medium" else risk_in

    return div_(
        {"class": "flex flex-row gap-2 items-center"},
        div_({"class": f"{risk_class} w-2.5 h-2.5 rounded-full"}),
        p_(f"{risk.capitalize()} risk status"),
    )
    
def get_percentage_risk(risk_in: str):   
    try:
        # Convert string input to float
        risk_value = float(risk_in)
        
        # Validate input range
        if not (0 <= risk_value <= 100):
            raise ValueError("Risk value must be between 0 and 100")
            
        # Determine color based on risk value
        if risk_value < 50:
            risk_class = "bg-red-500"
        elif risk_value < 70:
            risk_class = "bg-orange-400"
        else:
            risk_class = "bg-green-500"
            
        return div_(
            {"class": "flex flex-row gap-2 items-center"},
            div_({"class": f"{risk_class} w-2.5 h-2.5 rounded-full"}),
        )
        
    except ValueError as e:
        # Handle invalid input
        print(f"Error: {str(e)}")
        return div_(
            {"class": "flex flex-row gap-2 items-center"},
            div_({"class": "bg-gray-300 w-2.5 h-2.5 rounded-full"}),  # Default to gray for invalid input
        )

def get_result_div(result):
    modal_name = f"{result.get('title').replace(' ', '-')}-modal"
    if "info" not in result:
        return h3_({"class": "text-sm text-slate-700"}, result.get("title"))
    return div_(
        {"class": "flex flex-row"},
        h3_({"class": "text-base text-slate-700 flex-none font-bold"}, result.get("title")),
    )
    
    
    
def generate_health_score_html(metric_name, metric_value, unit, risk_status='', description='', icon_name='chart-line', marker_position='50',info=''):
    """
    Generate a generic HTML template for health metrics.

    :param metric_name: Name of the metric (e.g., 'Resting Heart Rate')
    :param metric_value: Numeric value of the metric
    :param unit: Unit of measurement (e.g., 'bpm', 'breaths/min')
    :param risk_status: Numeric value or string to determine risk status
    :param description: Detailed description of the metric
    :param icon_name: Font Awesome icon name to use (defaults to 'chart-line')
    :param marker_position: The value of the marker
    :param info: The text for the info pop up
    :return: A string of HTML for the metric section
    """
    # Determine risk status based on numeric value
    def determine_risk_status(status):
        # Convert to float if possible, otherwise default to 50
        try:
            numeric_status = float(status)
        except (ValueError, TypeError):
            return 'unknown-risk'
        
        # Determine risk based on numeric value
        if 0 <= numeric_status < 50:
            return 'high-risk'
        elif 50 <= numeric_status < 70:
            return 'moderate-risk'
        elif 70 <= numeric_status <= 100:
            return 'low-risk'
        else:
            return 'unknown-risk'

    # Determine risk status
    risk_status = determine_risk_status(risk_status)
    
    modal_name = f"{metric_name.replace(' ', '-')}-modal"
    
    # Risk class definitions
    risk_classes = {
        "low-risk": ("Low Risk", "low-risk"),
        "moderate-risk": ("Moderate Risk", "moderate-risk"),
        "high-risk": ("High Risk", "high-risk"),
        "unknown-risk": ("Unknown Risk", "unknown-risk"),
    }
    
    # Get risk text and class
    risk_text, risk_class = risk_classes.get(risk_status, ("Unknown Risk", "unknown-risk"))

    # Ensure marker_position is a string and within 0-100 range
    try:
        marker_position = str(max(0, min(100, float(marker_position))))
    except (ValueError, TypeError):
        marker_position = '50'

    # Generate HTML
    return f'''
        <div class="section-container">            
            <div class="cardio-metabolic-container">                    
                    <div class="metric-icon {risk_class}">
                        <span class="fa-solid {icon_name}"></span>                    
                    </div>
                    <div class="metric-header">
                        <div>{metric_name}</div>
                        <div class="risk-indicator {risk_class}">{risk_text}
                        <button class="cursor-pointer hover:text-gray-600" 
                                data-modal-target="{modal_name}" 
                                data-modal-toggle="{modal_name}" 
                                type="button">&#x24d8</button>        
                        </div>
                        <div 
                            id="{modal_name}" 
                            tabindex="-1" 
                            aria-hidden="true" 
                            class="hidden overflow-y-auto overflow-x-hidden fixed top-0 right-0 left-0 z-50 justify-center items-center w-full md:inset-0 h-[calc(100%-1rem)] max-h-full">
                            
                            <div class="relative p-4 w-full max-w-2xl max-h-full">
                                <div class="relative bg-white rounded-lg shadow">
                                    <div class="flex items-center justify-between p-4 md:p-5 border-b rounded-t">
                                        <h3 class="text-xl font-semibold text-gray-900">{metric_name}</h3>
                                        
                                        <button 
                                            type="button" 
                                            class="text-gray-400 bg-transparent hover:bg-gray-200 hover:text-gray-900 rounded-lg text-sm w-8 h-8 ms-auto inline-flex justify-center items-center"
                                            data-modal-hide="{modal_name}">
                                            
                                            <svg 
                                                class="w-3 h-3" 
                                                aria-hidden="true" 
                                                xmlns="http://www.w3.org/2000/svg" 
                                                fill="none" 
                                                viewBox="0 0 14 14">
                                                <path 
                                                    stroke="currentColor" 
                                                    stroke-linecap="round" 
                                                    stroke-linejoin="round" 
                                                    stroke-width="2" 
                                                    d="m1 1 6 6m0 0 6 6M7 7l6-6M7 7l-6 6">
                                                </path>
                                            </svg>
                                            
                                            <span class="sr-only">Close modal</span>
                                        </button>
                                    </div>
                                    
                                    <div class="p-4 md:p-5 space-y-4">
                                        <p class="text-base leading-relaxed text-gray-500">{info}</p>
                                    </div>
                                </div>
                            </div>
                        </div>                    
                </div>                
                                                    
                
                <div class="score-main">
                    <div class="score-value-container">
                        <div class="metric-value">{metric_value}{unit}</div>
                    </div>
                    <div class="score-description">
                        <div class="metric-description">{description}</div> 
                    </div>
                </div>
                <div class="score-progress">
                    <div class="score-marker" style="left: {marker_position}%;"></div>
                </div>                                
            </div>          
    '''


  
    
def generate_metric_blood_html(metric_name, metric_value, unit, risk_status='', description='', icon_name='chart-line',info=''):
    """
    Generate a generic HTML template for health metrics.

    :param metric_name: Name of the metric (e.g., 'Resting Heart Rate')
    :param metric_value: Numeric value of the metric
    :param unit: Unit of measurement (e.g., 'bpm', 'breaths/min')
    :param risk_status: Risk status ('low', 'medium', 'high', or 'unknown')
    :param description: Detailed description of the metric
    :param icon_name: Font Awesome icon name to use (defaults to 'chart-line')
    :param info: The text for the info pop up
    :return: A string of HTML for the metric section
    
    """
    
    modal_name = f"{metric_name.replace(' ', '-')}-modal"
    
    # Determine risk text and class
    risk_classes = {
        "low": ("Low Risk", "low-risk"),
        "medium": ("Moderate Risk", "moderate-risk"),
        "high": ("High Risk", "high-risk"),
        "unknown": ("Unknown Risk", "unknown-risk"),
    }
    
    # Get risk text and class
    risk_text, risk_class = risk_classes.get(risk_status.lower(), ("Unknown Risk", "unknown-risk"))
    
      # Determine the info button styling based on risk status
    if risk_status.lower() == "unknown":
        # For Unknown Risk: gray button without background
        info_button_style = "inline-flex items-center justify-center ml-1 text-gray-500 hover:text-gray-700 transition-colors duration-200 focus:outline-none focus:text-gray-700"
    else:
        # For other risk statuses: button with background
        info_button_style = "cursor-pointer hover:text-gray-600"

    # Generate HTML
    return f'''
    <div class="section-container">
    <div class="metric-container">    
        <div class="">
            <div class="metric-icon {risk_class}">
                <span class="fa-solid {icon_name}"></span>                    
            </div>
            <div class="metric-header">
                <div>{metric_name}</div>
                <div class="risk-indicator {risk_class}">{risk_text}
                   <button class="{info_button_style}" 
                                data-modal-target="{modal_name}" 
                                data-modal-toggle="{modal_name}" 
                                type="button">&#x24d8</button></div>        
                        <div 
                            id="{modal_name}" 
                            tabindex="-1" 
                            aria-hidden="true" 
                            class="hidden overflow-y-auto overflow-x-hidden fixed top-0 right-0 left-0 z-50 justify-center items-center w-full md:inset-0 h-[calc(100%-1rem)] max-h-full">
                            
                            <div class="relative p-4 w-full max-w-2xl max-h-full">
                                <div class="relative bg-white rounded-lg shadow">
                                    <div class="flex items-center justify-between p-4 md:p-5 border-b rounded-t">
                                        <h3 class="text-xl font-semibold text-gray-900">{metric_name}</h3>
                                        
                                        <button 
                                            type="button" 
                                            class="text-gray-400 bg-transparent hover:bg-gray-200 hover:text-gray-900 rounded-lg text-sm w-8 h-8 ms-auto inline-flex justify-center items-center"
                                            data-modal-hide="{modal_name}">
                                            
                                            <svg 
                                                class="w-3 h-3" 
                                                aria-hidden="true" 
                                                xmlns="http://www.w3.org/2000/svg" 
                                                fill="none" 
                                                viewBox="0 0 14 14">
                                                <path 
                                                    stroke="currentColor" 
                                                    stroke-linecap="round" 
                                                    stroke-linejoin="round" 
                                                    stroke-width="2" 
                                                    d="m1 1 6 6m0 0 6 6M7 7l6-6M7 7l-6 6">
                                                </path>
                                            </svg>
                                            
                                            <span class="sr-only">Close modal</span>
                                        </button>
                                    </div>
                                    
                                    <div class="p-4 md:p-5 space-y-4">
                                        <p class="text-base leading-relaxed text-gray-500">{info}</p>
                                    </div>
                                </div>
                            </div>
                        </div>
                    </div>                              
            
            <div class="score-main">
                <div class="score-value-container">
                    <div class="metric-value">{metric_value}{unit}</div>
                </div>
                <div class="score-description">
                    <div class="metric-description">{description}</div> 
                </div>
            </div>                                           
        </div>  
    </div>
'''

def generate_metric_html(metric_name, metric_value, unit, risk_status='', description='', icon_name='chart-line',info=''):
    """
    Generate a generic HTML template for health metrics.

    :param metric_name: Name of the metric (e.g., 'Resting Heart Rate')
    :param metric_value: Numeric value of the metric
    :param unit: Unit of measurement (e.g., 'bpm', 'breaths/min')
    :param risk_status: Risk status ('low', 'medium', 'high', or 'unknown')
    :param description: Detailed description of the metric
    :param icon_name: Font Awesome icon name to use (defaults to 'chart-line')
    :param info: The text for the info pop up
    :return: A string of HTML for the metric section
    
    """
    
    modal_name = f"{metric_name.replace(' ', '-')}-modal"
    
    # Determine risk text and class
    risk_classes = {
        "low": ("Low Risk", "low-risk"),
        "medium": ("Moderate Risk", "moderate-risk"),
        "high": ("High Risk", "high-risk"),
        "unknown": ("Unknown Risk", "unknown-risk"),
    }
    
    # Get risk text and class
    risk_text, risk_class = risk_classes.get(risk_status.lower(), ("Unknown Risk", "unknown-risk"))
    
      # Determine the info button styling based on risk status
    if risk_status.lower() == "unknown":
        # For Unknown Risk: gray button without background
        info_button_style = "inline-flex items-center justify-center ml-1 text-gray-500 hover:text-gray-700 transition-colors duration-200 focus:outline-none focus:text-gray-700"
    else:
        # For other risk statuses: button with background
        info_button_style = "cursor-pointer hover:text-gray-600"

    # Generate HTML
    return f'''
    <div class="metric-container">    
        <div class="">
            <div class="metric-icon {risk_class}">
                <span class="fa-solid {icon_name}"></span>                    
            </div>
            <div class="metric-header">
                <div>{metric_name}</div>
                <div class="risk-indicator {risk_class}">{risk_text}
                   <button class="{info_button_style}" 
                                data-modal-target="{modal_name}" 
                                data-modal-toggle="{modal_name}" 
                                type="button">&#x24d8</button></div>        
                        <div 
                            id="{modal_name}" 
                            tabindex="-1" 
                            aria-hidden="true" 
                            class="hidden overflow-y-auto overflow-x-hidden fixed top-0 right-0 left-0 z-50 justify-center items-center w-full md:inset-0 h-[calc(100%-1rem)] max-h-full">
                            
                            <div class="relative p-4 w-full max-w-2xl max-h-full">
                                <div class="relative bg-white rounded-lg shadow">
                                    <div class="flex items-center justify-between p-4 md:p-5 border-b rounded-t">
                                        <h3 class="text-xl font-semibold text-gray-900">{metric_name}</h3>
                                        
                                        <button 
                                            type="button" 
                                            class="text-gray-400 bg-transparent hover:bg-gray-200 hover:text-gray-900 rounded-lg text-sm w-8 h-8 ms-auto inline-flex justify-center items-center"
                                            data-modal-hide="{modal_name}">
                                            
                                            <svg 
                                                class="w-3 h-3" 
                                                aria-hidden="true" 
                                                xmlns="http://www.w3.org/2000/svg" 
                                                fill="none" 
                                                viewBox="0 0 14 14">
                                                <path 
                                                    stroke="currentColor" 
                                                    stroke-linecap="round" 
                                                    stroke-linejoin="round" 
                                                    stroke-width="2" 
                                                    d="m1 1 6 6m0 0 6 6M7 7l6-6M7 7l-6 6">
                                                </path>
                                            </svg>
                                            
                                            <span class="sr-only">Close modal</span>
                                        </button>
                                    </div>
                                    
                                    <div class="p-4 md:p-5 space-y-4">
                                        <p class="text-base leading-relaxed text-gray-500">{info}</p>
                                    </div>
                                </div>
                            </div>
                        </div>
                    </div>                              
            
            <div class="score-main">
                <div class="score-value-container">
                    <div class="metric-value">{metric_value}{unit}</div>
                </div>
                <div class="score-description">
                    <div class="metric-description">{description}</div> 
                </div>
            </div>                                           
        </div>  
    </div>
'''



def generate_score_only_html(metric_name, metric_value, unit, risk_status='', description='', icon_name='chart-line',info=''):
    """
    Generate a generic HTML template for health metrics.

    :param metric_name: Name of the metric (e.g., 'Resting Heart Rate')
    :param metric_value: Numeric value of the metric
    :param unit: Unit of measurement (e.g., 'bpm', 'breaths/min')
    :param risk_status: Risk status ('low', 'medium', 'high', or 'unknown')
    :param description: Detailed description of the metric
    :param icon_name: Font Awesome icon name to use (defaults to 'chart-line')
    :param info: The text for the info pop up
    :return: A string of HTML for the metric section
    
    """
    
    modal_name = f"{metric_name.replace(' ', '-')}-modal"
    
    # Determine risk text and class
    risk_classes = {
        "low": ("Low Risk", "low-risk"),
        "medium": ("Moderate Risk", "moderate-risk"),
        "high": ("High Risk", "high-risk"),
        "unknown": ("Unknown Risk", "unknown-risk"),
    }
    
    # Get risk text and class
    risk_text, risk_class = risk_classes.get(risk_status.lower(), ("Unknown Risk", "unknown-risk"))
    
      # Determine the info button styling based on risk status
    if risk_status.lower() == "unknown":
        # For Unknown Risk: gray button without background
        info_button_style = "inline-flex items-center justify-center ml-1 text-gray-500 hover:text-gray-700 transition-colors duration-200 focus:outline-none focus:text-gray-700"
    else:
        # For other risk statuses: button with background
        info_button_style = "cursor-pointer hover:text-gray-600"

    # Generate HTML
    html = f'''
    <div class="metric-container">    
        <div class="">
            <div class="metric-icon {risk_class}">
                <span class="fa-solid {icon_name}"></span>                    
            </div>
            <div class="metric-header">
                <div>{metric_name}</div>
                <div>
                   <button class="{info_button_style}" 
                                data-modal-target="{modal_name}" 
                                data-modal-toggle="{modal_name}" 
                                type="button">&#x24d8</button></div>        
                        <div 
                            id="{modal_name}" 
                            tabindex="-1" 
                            aria-hidden="true" 
                            class="hidden overflow-y-auto overflow-x-hidden fixed top-0 right-0 left-0 z-50 justify-center items-center w-full md:inset-0 h-[calc(100%-1rem)] max-h-full">
                            
                            <div class="relative p-4 w-full max-w-2xl max-h-full">
                                <div class="relative bg-white rounded-lg shadow">
                                    <div class="flex items-center justify-between p-4 md:p-5 border-b rounded-t">
                                        <h3 class="text-xl font-semibold text-gray-900">{metric_name}</h3>
                                        
                                        <button 
                                            type="button" 
                                            class="text-gray-400 bg-transparent hover:bg-gray-200 hover:text-gray-900 rounded-lg text-sm w-8 h-8 ms-auto inline-flex justify-center items-center"
                                            data-modal-hide="{modal_name}">
                                            
                                            <svg 
                                                class="w-3 h-3" 
                                                aria-hidden="true" 
                                                xmlns="http://www.w3.org/2000/svg" 
                                                fill="none" 
                                                viewBox="0 0 14 14">
                                                <path 
                                                    stroke="currentColor" 
                                                    stroke-linecap="round" 
                                                    stroke-linejoin="round" 
                                                    stroke-width="2" 
                                                    d="m1 1 6 6m0 0 6 6M7 7l6-6M7 7l-6 6">
                                                </path>
                                            </svg>
                                            
                                            <span class="sr-only">Close modal</span>
                                        </button>
                                    </div>
                                    
                                    <div class="p-4 md:p-5 space-y-4">
                                        <p class="text-base leading-relaxed text-gray-500">{info}</p>
                                    </div>
                                </div>
                            </div>
                        </div>
                    </div>                              
            
            <div class="score-main">
                <div class="score-value-container">
                    <div class="metric-value">{metric_value}{unit}</div>
                </div>
                <div class="score-description">
                    <div class="metric-description">{description}</div> 
                </div>
            </div>                                           
        </div>  
    </div>
    '''
    
     # Check if metric_name is Oxygen Saturation' and add extra div if it is
    if metric_name == 'Oxygen Saturation':
        html += '''
            </div>
        '''    
    
    return html


def generate_html_no_metric(metric_name, risk_status='', description='', icon_name='chart-line',info=''):
    """
    Generate a generic HTML template for health metrics.

    :param metric_name: Name of the metric (e.g., 'Resting Heart Rate')
    :param metric_value: Numeric value of the metric
    :param unit: Unit of measurement (e.g., 'bpm', 'breaths/min')
    :param risk_status: Risk status ('low', 'medium', 'high', or 'unknown')
    :param description: Detailed description of the metric
    :param icon_name: Font Awesome icon name to use (defaults to 'chart-line')
    :param info: The text for the info pop up
    :return: A string of HTML for the metric section
    
    """
    
    modal_name = f"{metric_name.replace(' ', '-')}-modal"
    
    # Determine risk text and class
    risk_classes = {
        "low": ("Low Risk", "low-risk"),
        "medium": ("Moderate Risk", "moderate-risk"),
        "high": ("High Risk", "high-risk"),
        "unknown": ("Unknown Risk", "unknown-risk"),
    }
    
    # Get risk text and class
    risk_text, risk_class = risk_classes.get(risk_status.lower(), ("Unknown Risk", "unknown-risk"))
    
     # Determine the info button styling based on risk status
    if risk_status.lower() == "unknown risk":
        # For Unknown Risk: gray button without background
        info_button_style = "inline-flex items-center justify-center ml-1 text-gray-500 hover:text-gray-700 transition-colors duration-200 focus:outline-none focus:text-gray-700"
    else:
        # For other risk statuses: button with background
        info_button_style = "cursor-pointer hover:text-gray-600"
    
    html = f''''''            

    # Generate HTML
    if metric_name == 'LDL Cholesterol':
        html += f'''
            <div class="section-container">
        '''    
    
    html += f'''        
        <div class="metric-container">            
            <div class="metric-icon {risk_class}">
                <span class="fa-solid {icon_name}"></span>                    
            </div>
            <div class="metric-header">
                <div>{metric_name}</div>
                <div class="risk-indicator {risk_class}">{risk_text} 
                   <button class="{info_button_style}" 
                                data-modal-target="{modal_name}" 
                                data-modal-toggle="{modal_name}" 
                                type="button">&#x24d8</button>   </div>     
                        <div 
                            id="{modal_name}" 
                            tabindex="-1" 
                            aria-hidden="true" 
                            class="hidden overflow-y-auto overflow-x-hidden fixed top-0 right-0 left-0 z-50 justify-center items-center w-full md:inset-0 h-[calc(100%-1rem)] max-h-full">
                            
                            <div class="relative p-4 w-full max-w-2xl max-h-full">
                                <div class="relative bg-white rounded-lg shadow">
                                    <div class="flex items-center justify-between p-4 md:p-5 border-b rounded-t">
                                        <h3 class="text-xl font-semibold text-gray-900">{metric_name}</h3>
                                        
                                        <button 
                                            type="button" 
                                            class="text-gray-400 bg-transparent hover:bg-gray-200 hover:text-gray-900 rounded-lg text-sm w-8 h-8 ms-auto inline-flex justify-center items-center"
                                            data-modal-hide="{modal_name}">
                                            
                                            <svg 
                                                class="w-3 h-3" 
                                                aria-hidden="true" 
                                                xmlns="http://www.w3.org/2000/svg" 
                                                fill="none" 
                                                viewBox="0 0 14 14">
                                                <path 
                                                    stroke="currentColor" 
                                                    stroke-linecap="round" 
                                                    stroke-linejoin="round" 
                                                    stroke-width="2" 
                                                    d="m1 1 6 6m0 0 6 6M7 7l6-6M7 7l-6 6">
                                                </path>
                                            </svg>
                                            
                                            <span class="sr-only">Close modal</span>
                                        </button>
                                    </div>
                                    
                                    <div class="p-4 md:p-5 space-y-4">
                                        <p class="text-base leading-relaxed text-gray-500">{info}</p>
                                    </div>
                                </div>
                            </div>
                        </div>
                    </div>                             
            
            <div class="score-main">               
                <div class="score-description">
                    <div class="metric-description">{description}</div> 
                </div>
            </div>                                                     
        </div>
    '''
    
    # Check if metric_name is 'Framingham Score Risk' and add extra div if it is
    if metric_name == 'Framingham Score Risk' or metric_name == 'Cardiorespiratory Fitness':
        html += '''
            </div>
        '''     
    
    return html


def generate_html_disclaimer(metric_name, description=''):
    """
    Generate a generic HTML template for health metrics.

    :param metric_name: Name of the metric (e.g., 'Resting Heart Rate')
    :param description: Detailed description of the metric    
    :return: A string of HTML for the metric section
    
    """
            
    
    # Get risk text and class    
    
    

    # Generate HTML
    return f'''
    <div class="metric-container">    
        <div class="">            
            <div class="metric-header">
                <div>{metric_name}</div> 
            </div>
            <div class="score-main">               
                <div class="score-description">                
                    <div class="metric-description">{description}</div> 
                </div>
            </div>                                           
        </div>  
    </div>
'''

def generate_heading_html(heading=''):
    """
    Generate a generic HTML template for heading.
    
    :param heading: Heading of the combined metric  
    :return: A string of HTML for the metric section
    
    """             

    # Generate HTML
    return f'''                   
    <h2 class="text-lg">{heading}<h2>    
'''


def get_results(session):
    health_score = session.get('health_score', 0)
    
    results_to_display = [            
        {
            "title": "Health Scores",
            "value": generate_heading_html(heading="Health Scores")           
        },
        {
            "title": "Cardio Metabolic Score",
            "value": generate_health_score_html(
                metric_name="Cardio Metabolic Score",
                metric_value=f"{health_score}",
                unit="%",
                icon_name="fa-heart-circle-check",
                risk_status=f"{health_score}",
                description="""An aggregate measure of your overall health derived from fitness and body composition.""",
                marker_position=f"{health_score}",
                info="""
Your Cardio Metabolic Score is calculated using established scientific literature 
to provide a score which is balanced, taking into account your cardiorespiratory 
fitness and body composition estimates. To ensure a fair result, the estimates are 
normalized for your age and biological sex.<br/><br/>

The score is a percentage, out of 100%, where a higher score would represent a 
healthier balance between fitness and body fat. If your estimated risk is high, 
or you are concerned, it’s important to discuss with your healthcare professional 
to understand your risk and explore potential changes. This information is for 
general awareness and should not replace professional medical advice.

"""
            )
        },              
        {
            "title": "10-Year Cardiovascular Disease Risk",
             "value": generate_html_no_metric(
                metric_name="10-Year Cardiovascular Disease Risk",                
                icon_name="fa-person-arrow-up-from-line",
                risk_status=session.get('risk_adj_tenYrCvd', ''),
                description="""Your estimated risk of experiencing certain heart diseases, heart attack, or stroke in the next 10 years, calculated using several contributing factors.""",
                info="""
The 10-year CVD (Cardiovascular Disease) risk estimate predicts the likelihood of developing cardiovascular disease, heart attack, or stroke within the next 10 years. It is based on factors such as age, biological sex, resting heart rate, smoking status, BMI, and data from long-term studies.<br/><br/>

A higher risk score estimates an increased chance of cardiovascular events, while a lower score estimates a lower likelihood. However, this does not account for all possible risk factors.<br/><br/>

If you are concerned about your results or have additional risk factors, consult your healthcare professional for a comprehensive assessment and personalized guidance. This information is for general awareness and should not replace professional medical advice.
"""
            )           
        },
        {
            "title": "Framingham Score Risk",
             "value": generate_html_no_metric(
                metric_name="Framingham Score Risk",                
                icon_name="fa-shield-heart",
                risk_status=session.get('risk_adj_framinghamScore', ''),
                description="""An alternative estimate of your 10-year cardiovascular disease risk, based on data from the Framingham Study.""",
                info="""
The Framingham Risk Score estimates the 10-year risk of developing cardiovascular disease using data from the Framingham Heart Study, one of the longest-running studies on heart health.<br/><br/>

This score helps estimate individuals at higher risk, guiding prevention and lifestyle changes. However, it is a general estimate and may not reflect all individual health factors.<br/><br/>

If you have concerns about your risk level, consult your healthcare professional to interpret your results in the context of your overall health. This information is for general awareness and should not replace professional medical advice.
"""
            )           
        },
        {
            "title": "Vitals",
            "value": generate_heading_html(heading="Vitals")           
        },
        {
            "title": "Blood Pressure",
            "value": generate_metric_blood_html(
                metric_name="Blood Pressure",
                metric_value=f"{session.get('mmHg_ent_systolicBP')} / {session.get('mmHg_ent_diastolicBP')}",
                unit="mmhg",
                icon_name="fa-heart-pulse",
                risk_status=session.get('risk_adj_bloodPressure', ''),
                description="""The estimated force of blood in your arteries due to contraction and relaxation of the heart, and associated risk.""",
                info="""Blood pressure is the force of blood pushing against the walls of the arteries. It is highest when the heart beats and pumps blood into the arteries (systolic pressure) and lowest between beats when the heart relaxes (diastolic pressure).<br/><br/>

Consistently high blood pressure (hypertension) means the heart works harder and has knock-on effects in the arteries, kidneys and other organs. It increases the risk of serious conditions such as heart attack, stroke, heart failure, aneurysm, kidney damage, vision loss, metabolic syndrome, and dementia. Persistently low blood pressure (hypotension) may reduce blood flow to the heart and brain, increasing the risk of damage.<br/><br/>

For the most accurate estimate, measure when you are not experiencing illness-related symptoms, are relaxed (e.g., have sat down for five minutes), are comfortable, and have avoided caffeine or exercise in the past hour. Individual estimates may vary due to various factors. If concerned about your result, take multiple measurements to check for consistency. For further evaluation, consult your healthcare professional. This information is for general awareness and does not replace professional medical advice."""
            )
        },
        {
            "title": "Resting Heart Rate",
            "value": generate_metric_html(
                metric_name="Resting Heart Rate",
                metric_value=session.get('bpm_ent_restingHeartRate'),
                unit="bpm",
                icon_name="fa-chart-line",
                risk_status=session.get('risk_adj_restingHeartRate', ''),
                description="""The estimated number of times your heart beats per minute while at rest, and associated risk.""",
                info="""Resting heart rate is the number of beats per minute when at rest - sitting or lying down and relaxed (but not sleeping). Note that resting heart rate is not related to blood pressure.<br/><br/> 

Factors which can influence resting heart rate include fitness and activity levels, smoking, body size, air temperature, stress or anxiety and medication. Persistently high resting heart rate increases the risk of diabetes, hypertension and cardiovascular disease. <br/><br/>

For the most accurate estimate, measure when you are not experiencing illness-related symptoms, relaxed (e.g., have sat down for five minutes), are comfortable, and have avoided caffeine or exercise in the past hour. Individual estimates may vary due to various factors. If concerned about your result, take multiple estimates to check for consistency. For further evaluation, consult your healthcare professional. This information is for general awareness and does not replace professional medical advice."""
            )
        },        
        {
            "title": "Respiration Rate",
             "value": generate_score_only_html(
                metric_name="Respiration Rate",
                metric_value=session.get('int_raw_rr'),
                unit=" breaths per min",
                icon_name="fa-wind",
                risk_status="unknown",
                description="""The estimated number of breaths you take per minute while at rest.""",
                info="""Respiration rate is the number of breaths taken per minute and is a key indicator of lung health. For adults over 18, a normal resting respiratory rate ranges from 12 to 20 breaths per minute.<br/><br/>

A consistently high respiratory rate, especially when accompanied by other indicators like high pulse rate, fever, shortness of breath, cough, or chest pain, may indicate an underlying health issue. If you experience these symptoms, consult your healthcare professional for further evaluation. This information is for general awareness and should not replace professional medical advice."""                
            )           
        },
        {
            "title": "Oxygen Saturation",
             "value": generate_score_only_html(
                metric_name="Oxygen Saturation",
                metric_value=session.get('int_raw_oxygen'),
                unit="%",
                icon_name="fa-lungs",
                risk_status="unknown",
                description="""The estimated quantity of oxygen carried in your blood.""",
                info= """
Oxygen saturation (SpO2) measures how much oxygen your red blood cells are carrying. Normal levels range from 95% to 100% for adults and children, though levels around 95% may be typical for adults over 70 years of age.<br/><br/>

Oxygen levels below 95% may indicate an underlying issue with lung or circulatory function. If your estimates are consistently below this level or if you have concerns, consult your healthcare professional for further evaluation. This information is for general awareness and should not replace professional medical advice.
"""
            )           
        },       
        {
            "title": "Digital Biomarkers",
            "value": generate_heading_html(heading="Digital Biomarkers")           
        },
        {
            "title": "LDL Cholesterol",
             "value": generate_html_no_metric(
                metric_name="LDL Cholesterol",                
                icon_name="fa-arrow-down-up-across-line",
                risk_status=session.get('risk_adj_ldlC', ''),
                description="""Risk associated with the type of cholesterol that can cause plaque buildups that clog arteries and contribute to heart disease.""",
                info="""
LDL cholesterol (low-density lipoprotein) is often called the "bad" cholesterol because it can contribute to plaque buildup in the arteries, increasing the risk of heart disease and stroke.<br/><br/>

Lower LDL levels are generally better for heart health. If your estimated LDL cholesterol risk is high, consult your healthcare professional for further personalised analysis and to potentially discuss lifestyle changes or options to reduce your risk. This information is for general awareness and should not replace professional medical advice.
"""
            )           
        },        
        {
            "title": "HDL Cholesterol",
             "value": generate_html_no_metric(
                metric_name="HDL Cholesterol",                
                icon_name="fa-arrow-up-right-dots",
                risk_status=session.get('risk_adj_hdlC', ''),
                description="""Risk associated with the type of cholesterol that helps to remove excess cholesterol, protecting against heart disease.""",
                info="""
HDL cholesterol (high-density lipoprotein), known as the "good" cholesterol, helps remove excess cholesterol from the bloodstream. Higher HDL levels are associated with a lower risk of heart disease and help prevent LDL cholesterol buildup in the arteries.<br/><br/>

Maintaining a healthy HDL level is beneficial for heart health. If your estimated HDL cholesterol risk is high, consult your healthcare professional for further personalised analysis and to potentially discuss lifestyle changes to improve your cardiovascular protection. This information is for general awareness and should not replace professional medical advice.
"""
            )           
        },
        {
            "title": "Triglycerides",
             "value": generate_html_no_metric(
                metric_name="Triglycerides",                
                icon_name="fa-capsules",
                risk_status=session.get('risk_adj_triglycerides', ''),
                description="""Risk associated with triglycerides: a type of fat (lipid) in the blood that can harden arteries and increase cardiovascular risk.""",
                info="""
Triglycerides are a type of fat (lipid) in the blood that the body uses for energy. Elevated levels can contribute to artery hardening (arteriosclerosis), increasing the risk of heart disease, heart attack, and stroke.<br/><br/>

Keeping triglyceride levels within a normal range supports heart health. If your estimated Triglycerides risk is high, consult your healthcare professional for further personalised analysis and to potentially discuss lifestyle changes or options to reduce your risk. This information is for general awareness and should not replace professional medical advice.
"""
            )           
        },
        {
            "title": "Total Cholesterol",
             "value": generate_html_no_metric(
                metric_name="Total Cholesterol",                
                icon_name="fa-chart-bar",
                risk_status=session.get('risk_adj_totalCholesterol', ''),
                description="""Risk associated with your overall cholesterol level and cardiovascular health based on HDL-cholesterol, LDL-cholesterol, and triglycerides.""",
                info="""
Total cholesterol measures the overall amount of cholesterol in your blood, including both the "good" (HDL) and the "bad" (LDL) cholesterol providing an overall picture of cholesterol levels and cardiovascular health.<br/><br/>

An estimated high Total cholesterol risk may indicate an increased risk of heart disease and stroke, particularly if LDL or Triglycerides are elevated. However, the ratio of HDL to LDL also matters. Maintaining a healthy balance through diet, exercise, and lifestyle choices can help support heart health.<br/><br/>

If your estimated Total cholesterol risk is high, consult your healthcare professional to assess your full lipid profile. This information is for general awareness and should not replace professional medical advice.
"""
            )           
        },                                                                                            
        {
            "title": "Body Mass Index",
             "value": generate_html_no_metric(
                metric_name="Body Mass Index",                
                icon_name="fa-weight-scale",
                risk_status=session.get('risk_adj_bmi', ''),
                description="""Risk associated with your BMI, a measure of overweight or obesity.""",
                info="""
Body Mass Index (BMI) is a simple estimate that compares your weight to your height to estimate your body fat and potential risk for obesity-related conditions. It is commonly used as an indicator for risk of cardiovascular disease, high blood pressure, diabetes, osteoarthritis, and certain cancers.<br/><br/>

While BMI is a helpful tool for assessing overall health risk, it may not be as accurate for older adults, people with extreme heights, or those with more muscle mass, such as athletes or bodybuilders, who may have higher BMI due to muscle rather than fat. It is important to consider BMI alongside other body composition measurements for a more complete health assessment.<br/><br/>

If you are concerned about your BMI or its interpretation, consult your healthcare professional for a personalized evaluation of your health risks. This information is for general awareness and should not replace professional medical advice.
"""
            )           
        },
        {
            "title": "Arterial Stiffness",
             "value": generate_html_no_metric(
                metric_name="Arterial Stiffness",                
                icon_name="fa-brain",
                risk_status=session.get('risk_adj_bapwv', ''),
                description="""Risk associated with how stiff your aorta, other large arteries, and peripheral artery walls are: greater stiffness is associated with a higher risk of cardiovascular events.""",
                info="""
Arterial stiffness refers to how flexible or rigid the walls of your arteries are. It is an important indicator of heart health and is associated with the workload the heart faces to circulate blood throughout the body.
As you age, arterial stiffness naturally increases, which can raise blood pressure and contribute to conditions like insulin resistance. Higher arterial stiffness is linked to an increased risk of hypertension, diabetes, heart attack, stroke, heart failure, aneurysm, and kidney damage.<br/><br/>

If your estimated risk for arterial stiffness is high, it’s important to discuss with your healthcare professional to understand your risk and explore potential lifestyle changes or treatments. This information is for general awareness and should not replace professional medical advice.
"""
            )           
        }, 
        {
            "title": "Cardiorespiratory Fitness",
             "value": generate_html_no_metric(
                metric_name="Cardiorespiratory Fitness",                
                icon_name="fa-person-running",
                risk_status=session.get('risk_adj_restingFitness', ''),
                description="""Risk associated with heart and lung health based on VO2 max: the maximum amount of oxygen your body can consume during exercise.""",
                info="""
Cardiorespiratory fitness refers to the ability of your heart and lungs to supply oxygen to your muscles during exercise. It is commonly measured by VO2 max, which indicates the maximum rate of oxygen your body can utilize during physical activity.<br/><br/>

Lower levels of cardiorespiratory fitness are associated with an increased risk of heart disease, stroke, heart failure, and certain cancers. Improving your fitness through regular physical activity can help lower these risks and improve overall health.<br/><br/>

If your estimated risk for cardiorespiratory fitness is high, consider consulting your healthcare professional for advice on how to safely improve your fitness and reduce related health risks. This information is for general awareness and should not replace professional medical advice.
"""
            )           
        },
        {
            "title": "",
            "value": generate_html_disclaimer(
                metric_name="",                                
                description="""Seek medical advice if your health vital estimates are consistently outside of a healthy range and / or you experience symptoms.
"""
            )           
        },
        {
            "title": "Disclaimer",
            "value": generate_html_disclaimer(
                metric_name="Disclaimer",                                
                description="""This product is not designed for medical diagnosis or to evaluate medical conditions 
and is not a substitute for the clinical judgment of a health care professional. This 
product does not diagnose, treat, mitigate, monitor or prevent any disease, symptom, 
disorder or abnormal physical state. You should always consult a trusted health 
professional before making decisions about your health care.
"""
            )           
        },                
    ]
    resp = []
    for result in results_to_display:
        resp.append(
            span_(
                #{"class": "shadow-xl p-4 rounded-xl border border-gray-200 bg-white"},
                #get_result_div(result),
                #div_({"style": "height: 12px;"}),
                p_(result.get("value")) if result.get("value") else "",
                #p_(result.get("risk")) if result.get("risk") else "",
                #div_({"style": "height: 12px;"}),
                #div_({"class": ""},
                    #p_({"class": "text-sm leading-relaxed"}, result.get("info")),
                #),
            ),
        )
    return "".join(resp)

def get_details(session):
    resp = []
    resp.append(
            div_({"class": "shadow-xl p-4 rounded-xl border border-gray-200 bg-white flex justify-between"},
                # div_(),
                # div_({"class": "text-center"},                    
                #     span_({"class": "fa-solid fa-user w-6 h-6 text-blue-500"}),
                #     p_({"class": "text-sm font-bold"}, "Age"),
                #     p_({"class": "text-sm"}, session.get('yr_raw_age','')),
                # ),
                div_(),
                div_({"class": "text-center"},
                    span_({"class": "fa-solid fa-ruler-vertical w-6 h-6 text-blue-500"}),
                    p_({"class": "text-sm font-bold"}, "Height"),
                    p_({"class": "text-sm"}, session.get('cm_ent_height',''),
                        span_({"class": "text-sm"}, " cm")
                    ),
                ),
                div_(),
                div_({"class": "text-center"},
                    span_({"class": "fa-solid fa-weight-scale w-6 h-6 text-blue-500"}), 
                    p_({"class": "text-sm font-bold"}, "Weight"),
                    p_({"class": "text-sm"}, session.get('kg_ent_weight',''),
                        span_({"class": "text-sm"}, " kg")
                    ),
                ),
                div_(),
                div_({"class": "text-center"},
                    span_({"class": "fa-solid fa-venus-mars w-6 h-6 text-blue-500"}), 
                    p_({"class": "text-sm font-bold"}, "Biological sex"),
                    p_({"class": "text-sm"}, session.get('enum_ent_sex','')),
                ),
                div_(),
            ),
    ),
    return "".join(resp)

def share_icon() -> str:
    path_ = element("path", end_tag=False)
    return svg_(
        {
            "xmlns": "http://www.w3.org/2000/svg",
            "fill": "none",
            "viewBox": "0 0 24 24",
            "stroke-width": "1.5",
            "stroke": "currentColor",
            "class": "size-6",
        },
        path_(
            {
                "stroke-linecap": "round",
                "stroke-linejoin": "round",
                "d": "M7.217 10.907a2.25 2.25 0 1 0 0 2.186m0-2.186c.18.324.283.696.283 1.093s-.103.77-.283 1.093m0-2.186 9.566-5.314m-9.566 7.5 9.566 5.314m0 0a2.25 2.25 0 1 0 3.935 2.186 2.25 2.25 0 0 0-3.935-2.186Zm0-12.814a2.25 2.25 0 1 0 3.933-2.185 2.25 2.25 0 0 0-3.933 2.185Z",
            }
        ),
    )

def calendar_icon() -> str:
    path_ = element("path", end_tag=False)
    return svg_(
        {
            "xmlns": "http://www.w3.org/2000/svg",
            "viewBox": "0 0 20 20",
            "fill": "currentColor",
            "class": "w-4 h-4",
        },
        path_(
            {
                "fill-rule": "evenodd",
                "clip-rule": "evenodd",
                "d": "M6 2a1 1 0 00-1 1v1H4a2 2 0 00-2 2v11a2 2 0 002 2h12a2 2 0 002-2V6a2 2 0 00-2-2h-1V3a1 1 0 10-2 0v1H7V3a1 1 0 00-1-1zM4 8h12v9H4V8zm2-3h8V3H6v2z",
            }
        ),
    )
    

def get_return_button(session: dict):
    if "NextUrl" in session and session["NextUrl"] is not None:
        return div_(
            {"class": "pb-8 pt-2 flex justify-end"},
            button_(
                {
                    "class": "bg-blue-500 hover:bg-blue-700 text-white font-bold py-2 px-4 rounded",
                    "onclick": f"window.location.href = '{session['NextUrl']}';"
                },
                "Next"
            )
        )
    return div_(
            {"class": "pb-8 pt-2 flex justify-end"},
            button_(
                {
                    "class": "bg-blue-500 hover:bg-blue-700 text-white font-bold py-2 px-4 rounded",
                    "onclick": "print()"
                },
                "Print"
            )
        )

def content(session):
    current_date = datetime.now().strftime("%d %B %Y")
    return template(
        header_(
            {
                "class": "mx-auto container max-w-xl py-4 px-4",
            },
            div_(
                {"class": "flex justify-between items-center"},
                h1_({"class": "text-2xl font-bold"}, "Digital Health Check"),                
            ),
            div_(                
                span_({"class": "text-sm font-medium"}, """Thank you for trying the Digital Health Check demo!<br/><br/>"""),
                span_({"class": "text-sm"}, """This report is one example of how results could be presented to end-users. Other formats are possible, and different information can be provided, e.g., if intended for medical professionals.<br/><br/> 
                     
                     In addition to the information shown in this sample end-user report, licence-holders receive more detailed data that is captured in the scan and used in the calculations of risk estimates. All data is de-identified.<br/><br/>
                     
Contact us """ + a_({"href": "mailto:hello@ahi.tech"}, "hello@ahi.tech") + """ to find out more.                      
"""),
            ),
            div_({"style": "height: 10px;"}),
            div_(
                {"class": "flex items-center mt-2"},
                span_({"class": "text-sm font-medium"}, "Assessment conducted on"),
                div_(
                    {"class": "flex items-center ml-2"},
                    calendar_icon(),
                    span_({"class": "text-sm ml-1"}, current_date),
                ),
            ),            
            div_({"style": "height: 10px;"}),
        ),
        main_(
            section_(
                {"class": "container max-w-xl mx-auto px-4 flex flex-col gap-5"},
                # div_(
                #     {"class": "flex flex-row justify-between"},
                #     h2_({"class": "text-lg font-bold"}, "Your health"),
                #     # a_({"href": "#"}, share_icon()),
                # ),
                get_details(session),                                                                        
                get_results(session),
                # a_({"href": f"https://{ROOT_DOMAIN}/"}, "Restart Assessment"),
                get_return_button(session),
            ),
        ),
        extra_headers=[
            title_(f"Results | Digital Health Check | {session['session_id']}"),
            link_({"rel": "stylesheet", "href": "https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.4.2/css/all.min.css" }),
            style_("""
                * {
                    -webkit-print-color-adjust: exact !important;   /* Chrome, Safari 6 – 15.3, Edge */
                    color-adjust: exact !important;                 /* Firefox 48 – 96 */
                    print-color-adjust: exact !important;           /* Firefox 97+, Safari 15.4+ */
                }
                
                .metric-container {
                    background-color: #ffffff;
                    border-radius: 16px;
                    box-shadow: 0 4px 6px rgba(0, 0, 0, 0.1);
                    padding: 24px;            
                    margin-bottom: 20px;
                    font-family: sans-serif;
                    color: #505050;
                    position: relative;
                    border: 1px solid #e5e7eb;
                }

                .cardio-metabolic-container {
                    background-color: #dde9f8;
                    border-radius: 16px;
                    box-shadow: 0 4px 6px rgba(0, 0, 0, 0.1);
                    padding: 24px;            
                    margin-bottom: 20px;
                    font-family: sans-serif;
                    color: #505050;
                    position: relative;
                }

                .score-header {
                    display: flex;
                    align-items: center;
                    justify-content: space-between;
                    margin-bottom: 20px;
                }

                .score-icon {
                    width: 50px;
                    height: 50px;
                    border-radius: 50%;
                    display: flex;
                    align-items: center;
                    justify-content: center;
                    position: absolute;
                    top: -15px;
                    left: -15px;
                    font-size: 30px;
                }

                .score-icon.high-risk {
                    color: #F44336;
                    border: 2px solid #F44336;
                    background-color: #fff;
                }

                .high-risk-container {
                    display: flex;
                    flex-direction: column;
                    align-items: flex-end;
                    background-color: rgba(240, 240, 240, 0.7);
                    padding: 4px 8px;
                    border-radius: 15px;
                }

                .high-risk {
                    display: flex;
                    align-items: center;
                    font-size: 12px;
                    font-weight: 500;
                    margin-bottom: 5px;
                }

                .high-risk-text {
                    margin-right: 5px;
                    color: #777;
                }

                .info-icon {
                    margin-left: 5px;
                    color: #888;
                    cursor: pointer;
                }

                .high-risk-indicators {
                    display: flex;
                    align-items: center;
                    margin-left: 8px;
                }

                .high-risk-indicator {
                    width: 15px;
                    height: 4px;
                    border-radius: 2px;
                    margin-left: 2px;
                    background-color: #ddd;
                }

                .high-risk-indicator.active {
                    background-color: #F44336;
                }

                .score-main {
                    display: flex;
                    justify-content: center;
                    align-items: center;
                }

                .score-value-container {
                    display: flex;
                    flex-direction: column;
                    align-items: center;
                    margin-right: 10px;
                }

                .score-value {
                    font-size: 48px;
                    font-weight: bold;
                    color: #6b7280;
                }

                .score-percentage {
                    font-size: 24px;
                    font-weight: bold;
                    color: #6b7280;
                    margin-left: 4px;
                }

                .score-description {
                    color: #6b7280;
                    font-size: 16px;
                    line-height: 1.4;
                }

                .score-progress {
                    width: 100%;
                    height: 8px;
                    background: linear-gradient(to right, #F44336, #FFEB3B, #4CAF50);
                    border-radius: 4px;
                    margin-top: 16px;
                    position: relative;
                }

                .score-marker {
                    position: absolute;
                    width: 4px;
                    height: 18px;
                    background-color: #134a8d;
                    top: 50%;
                    transform: translate(-50%, -50%);
                    left: 40%;
                }

                .metric-header {
                    display: flex;
                    align-items: center;
                    justify-content: space-between;
                    margin-bottom: 10px;
                }

                .metric-icon {
                    width: 40px;
                    height: 40px;
                    border-radius: 50%;
                    display: flex;
                    align-items: center;
                    justify-content: center;
                    position: absolute;
                    top: -10px;
                    left: -10px;
                    font-size: 20px;
                }

                .metric-icon.high-risk {
                    color: #F44336;
                    border: 2px solid #F44336;
                    background-color: #fff;
                }

                .metric-icon.moderate-risk {
                    color: #FF9800;
                    border: 2px solid #FF9800;
                    background-color: #fff;
                }

                .metric-icon.low-risk {
                    color: #4CAF50;
                    border: 2px solid #4CAF50;
                    background-color: #fff;
                }
                
                .metric-icon.unknown-risk {
                    color: #000000;
                    border: 2px solid #000000;
                    background-color: #fff;
                }

                .metric-value {
                    font-size: 24px;
                    font-weight: bold;
                    width: 120px;
                    text-align: center;
                }

                .metric-description {
                    font-size: 14px;
                    color: #777;
                }

                .risk-indicator {
                    display: inline-block;
                    width: 120px; /* Fixed width of 120 pixels */
                    text-align: center; /* Center the text within the fixed width */
                    padding: 4px 8px;
                    border-radius: 10px;
                    font-size: 12px;
                    font-weight: bold;                    
                    box-sizing: border-box; /* Ensures padding doesn't increase the total width */
                }

                .high-risk {
                    background-color: #FBC5C5;
                    color: #B36B6A;
                }

                .moderate-risk {
                    background-color: #FFE0B2; /* Light orange background */
                    color: #ff8c00; /* Darker text for better readability */
                }

                .low-risk {
                    background-color: #C4F0CE;
                    color: #5A9268;
                }
                
                .unknown-risk{
                    background-color: #000000;
                }
                

                i.fa-solid, i.fas {
                    font-family: 'Font Awesome 6 Free';
                }
                
                .section-container {
					background-color: #ffffff;
					border-radius: 16px;
					box-shadow: 0 4px 6px rgba(0, 0, 0, 0.1);
					padding: 24px;
					margin-bottom: 20px;
					font-family: sans-serif;
					color: #505050;
					position: relative;
					border: 1px solid #e5e7eb;
				}                
                
                   """
            ),
        ],
        body_style="bg-[#F9F9FC] text-[#4F6172]",
    )
