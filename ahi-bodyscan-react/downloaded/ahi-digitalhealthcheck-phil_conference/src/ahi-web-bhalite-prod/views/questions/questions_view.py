from hypymedia import (
    button_,
    div_,
    form_,
    h2_,
    header_,
    link_,
    main_,
    p_,
    script_,
    section_,
    svg_,
    title_,
)
from views.template import template

from .section_activity import section_activity
from .section_biological_sex import section_biological_sex

# from .section_body_shape import section_body_shape
from .section_bp_medications import section_bp_medications
from .section_chronic_medications import section_chronic_medications
from .section_dob import section_dob
from .section_email import section_email
from .section_height import section_height
from .section_index import section_index
from .section_smoker import section_smoker
from .section_weight import section_weight


def content_model(session_id: str):
    return content(session_id)


def next_button() -> str:
    return button_(
        {
            "type": "button",
            "data-next": True,
            "class": "flex items-center justify-center bg-[#2607E7] hover:bg-[#2607E7]/85 text-white font-bold py-4 px-4 rounded-lg pointer:cursor",
        },
        div_(
            {"class": "flex justify-between items-center w-full pointer-events-none"},
            div_({"class": "w-full text-center"}, "Next"),
            svg_(
                {
                    "id": "arrow",
                    "class": "w-6 h-6 ml-2 hover:scale-110 transition-transform duration-200",
                    "xmlns": "http://www.w3.org/2000/svg",
                    "viewBox": "0 0 24 24",
                    "stroke": "#ffffff",  # Change this to the color you want
                    "fill": "none",  # Add this line
                },
                '<path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M9 5l7 7-7 7"/>',
            ),
        ),
    )
    
def get_started_button() -> str:
    return button_(
        {
            "type": "button",
            "data-next": True,
            "class": "flex items-center justify-center bg-[#2607E7] hover:bg-[#2607E7]/85 text-white font-bold py-4 px-4 rounded-lg pointer:cursor",
        },
        div_(
            {"class": "flex justify-between items-center w-full pointer-events-none"},
            div_({"class": "w-full text-center"}, "Get Started"),
            svg_(
                {
                    "id": "arrow",
                    "class": "w-6 h-6 ml-2 hover:scale-110 transition-transform duration-200",
                    "xmlns": "http://www.w3.org/2000/svg",
                    "viewBox": "0 0 24 24",
                    "stroke": "#ffffff",  # Change this to the color you want
                    "fill": "none",  # Add this line
                },
                '<path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M9 5l7 7-7 7"/>',
            ),
        ),
    )


def submit_button() -> str:
    return button_(
        {
            "type": "submit",
            "value": "Submit",
            "class": "bg-[#2607E7] hover:bg-[#2607E7]/85 text-white font-bold py-4 px-4 rounded-lg pointer:cursor",
        },
        div_(
            {"class": "flex justify-between items-center w-full pointer-events-none"},
            div_({"class": "w-full text-center"}, "Submit"),
            svg_(
                {
                    "id": "arrow",
                    "class": "w-6 h-6 ml-2 hover:scale-110 transition-transform duration-200",
                    "xmlns": "http://www.w3.org/2000/svg",
                    "viewBox": "0 0 24 24",
                    "stroke": "#ffffff",  # Change this to the color you want
                    "fill": "none",  # Add this line
                },
                '<path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M9 5l7 7-7 7"/>',
            ),
        ),
    )


def section_controls(is_hidden: bool = True) -> str:
    class_name = " hidden" if is_hidden else ""
    return header_(
        {
            "id": "section-controls",
            "class": f"flex flex-row mt-5 py-4 items-center justify-between {class_name}",
        },
        button_(
            {
                "type": "button",
                "class": "w-8 h-8 bg-white drop-shadow-md rounded-lg cursor-pointer select-none",
                "data-back": True,
            },
            "<",
        ),
        progress_bar(),
    )


def section_header(
    title: str,
    direction: str = "Please select",
) -> str:
    return header_(
        {"class": "flex flex-col justify-center mb-10"},
        h2_({"class": "text-2xl text-center text-[#00265F]"}, title),
        p_({"class": "text-center text-sm text-[#4F6172]"}, direction),
    )


def progress_bar() -> str:
    return div_(
        {
            "id": "progress-wrapper",
            "class": "relative mx-auto max-w-72 h-1 w-full",
        },
        div_(
            {
                "id": "progress-bg",
                "class": "bg-[#A3C6DF]/50 h-1 w-full rounded-md absolute top-0 left-0 z-0",
            }
        ),
        div_(
            {
                "id": "progress-bar",
                "class": "bg-[#57C0EF] h-1 w-0 rounded-md absolute top-0 left-0 z-10 transition-all duration-500 ease-in-out",
            }
        ),
    )


def content(session_id: str):
    return template(
        main_(
            section_(
                {
                    "class": "container mx-auto flex flex-col min-h-screen px-4 max-w-xl overflow-hidden"
                },
                form_(
                    {
                        "action": f"https://cfbha.ahi.zone/questions/{session_id}",
                        "id": "questions-form",
                        "method": "post",
                        "hx-boost": False,
                        "class": "flex flex-col min-h-screen",
                    },
                    section_controls(),
                    section_index(action=get_started_button),
                    section_biological_sex(section_header=section_header),
                    section_dob(
                        section_header=section_header,
                        action=next_button,
                    ),
                    section_height(
                        section_header=section_header,
                        action=next_button,
                    ),
                    section_weight(
                        section_header=section_header,
                        action=next_button,
                    ),
                    # section_body_shape(
                    #     section_header=section_header,
                    # ),
                    section_smoker(section_header=section_header),
                    section_activity(
                        section_header=section_header,
                    ),
                    section_chronic_medications(section_header=section_header),
                    # section_history(section_header=section_header),
                    section_bp_medications(section_header=section_header),
                    section_email(
                        section_header=section_header,
                        action=submit_button,
                    ),
                ),
            ),
        ),
        extra_headers=[
            title_("Questionaire | Biometric Health Assessment"),
            script_({"src": "/assets/js/questions.js"}),
            # FIXME ADD Speculation API
            link_(
                {"href": "https://unpkg.com/@rive-app/canvas@2.9.1", "rel": "prefetch"}
            ),
            link_({"href": "/assets/js/lib/rppg.wasm", "rel": "prefetch"}),
            link_({"href": "/assets/js/lib/rppg.data", "rel": "prefetch"}),
            link_({"href": "/assets/rive/hud.riv", "rel": "prefetch"}),
        ],
        body_style="bg-[#F9F9FC] text-[#4F6172]",
    )
