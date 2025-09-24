from hypymedia import input_, label_, section_, div_, p_, h3_, button_, svg_, span_, path_, br_


def get_result_div(result):
    # Split the text into lines and create br elements
    modal_name = f"{result.get('title', '').replace(' ', '-').lower()}-modal"
    info_lines = result.get("info", "").split("\n")
    helptext_lines = result.get("helptext", "").split("\n")    
    info_content = []
    for i, line in enumerate(info_lines):
        info_content.append(line)
        if i < len(info_lines) - 1:
            info_content.append(br_())
            
    helptext_content = []
    for i, line in enumerate(helptext_lines):
        helptext_content.append(line)
        if i < len(helptext_lines) - 1:
            helptext_content.append(br_())
    
    
    return div_(
        {"class": "flex flex-row items-center justify-center w-full"},
        h3_(
            {"class": "text-center font-bold"},
            result.get("title", "")
        ),        
        div_({"class": ""}),           
        button_({
            "class": "block cursor-pointer hover:text-gray-600",
            "data-modal-target": modal_name,
            "data-modal-toggle": modal_name,
            "type": "button"
        }, "&#x24d8;"),
       div_({
            "id": modal_name,
            "tabindex": "-1",
            "aria-hidden": "true",
            "class": "hidden overflow-y-auto overflow-x-hidden fixed top-0 right-0 left-0 z-50 justify-center items-center w-full md:inset-0 h-[calc(100%-1rem)] max-h-full"
        },
         div_({"class": "relative p-4 w-full max-w-2xl max-h-full"},
        
        # Modal Container
        div_({"class": "relative bg-white rounded-lg shadow"},
        button_({
            "type": "button",
            "class": "absolute top-4 right-4 text-gray-400 bg-transparent hover:bg-gray-200 hover:text-gray-900 rounded-lg text-sm w-8 h-8 inline-flex justify-center items-center",
            "data-modal-hide": modal_name,                        
             },
            svg_({
                    "class": "w-3 h-3",
                    "aria-hidden": "true",
                    "xmlns":"http://www.w3.org/2000/svg",
                    "fill": "none",
                    "viewBox": "0 0 14 14",
                },
                path_({
                    "stroke": "currentColor",
                    "stroke-linecap": "round",
                    "stroke-linejoin": "round",
                    "stroke-width": "2",
                    "d": "m1 1 6 6m0 0 6 6M7 7l6-6M7 7l-6 6",
                }),
            ),
            span_({"class": "sr-only"},
                    "Close modal"
            ),
        ),           
                # Modal Body with Explicit Line Breaks
                div_({"class": "p-4 md:p-5 space-y-4"},
                    p_({
                        "class": "text-left"
                    }, *info_content),
                    p_({
                        "class": "text-sm block mt-2 text-left"
                    }, *helptext_content)
                )
            )
        )           
    )
    )



def section_email(section_header: callable, action: callable) -> str:
    return section_(
        {"data-index": True, "class": "hidden flex flex-col gap-4 fade-in"},
        section_header(
            title="Email",
            direction="Please enter a valid email address. <br/> Without a valid email, the scan will not activate.",
        ),
        label_({"for": "email", "class": "sr-only"}, "Email:"),
        input_(
            {
                "type": "email",
                "id": "email",
                "name": "email",
                "required": True,
                "class": "w-full p-4 border border-gray-300 rounded bg-white",
                "placeholder": "email address",
            }
        ),
        action(),
        div_(
            {"class": "mt-6"},                                  
             h3_(
                {"class": "text-center"},
                "After selecting <b>Submit</b>, you will be directed to complete a quick scan of your face.",
            ), 
        ),
        div_(
            {"class": "text-center"},                                   
            # Add the result div here
            get_result_div({
                "title": "What does this entail?",
                "info": "<b>How do I complete the scan?</b> \n\n <b>1. Enable Camera Access:</b> Allow usage of your front camera. No images or videos are captured or stored.\n\n <b>2. Get in Position:</b> Hold your phone at face level with a relaxed elbow bend or move closer to your laptop screen.\n\n <b>3. Align Your Face:</b> Centre your face within the on-screen circle. \n\n <b>4. Scan Starting Shortly:</b> The scan will automatically begin. \n\n Need Help?",
                "helptext": "No scan after 10 seconds? Adjust your distance and ensure your face stays centred. \n Lighting Check: If scanning still doesn't start, move to an area with even lighting for better results. \n\n"
            })
        ),
        
    )
