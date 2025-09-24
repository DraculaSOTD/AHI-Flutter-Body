from hypymedia import (
    a_,
    canvas_,
    h1_,
    h2_,
    header_,
    main_,
    p_,
    script_,
    section_,
    span_,
    style_,
    svg_,
    title_,
)
from views.template import template


def content_model():
    return content()


def particle_viewer():
    return canvas_(
        {
            "id": "canvas",
            "width": "500",
            "height": "500",
            "data-rgb": "255, 255, 255",
            "data-bg": "#2607e7",
            "class": "mx-auto w-80 h-80 md:w-auto md:h-auto",
        }
    )


def content():
    return template(
        header_(
            {
                "class": "mx-auto container max-w-xl py-4 px-4",
            },
            a_(
                {"href": "https://cfbha.ahi.zone/"},
                h1_(
                    "Biometric Health Assessment",
                ),
            ),
        ),
        main_(
            section_(
                {
                    "class": "mx-auto container max-w-xl flex flex-col gap-y-3 px-2 my-8",
                },
                particle_viewer(),
                h2_(
                    {"class": "font-semi-bold"},
                    f"Discover your{span_({'class':'font-bold text-2xl block'},'Biometric Health in 60 seconds')}",
                ),
                p_(
                    {"class": "font-light text-sm", "style": "color: #f9f9fc;"},
                    "Answer a few quick questions, and let our AI-powered facial scan provide you with a&nbsp;personalized health assessment.",
                ),
                a_(
                    {
                        "href": "https://cfbha.ahi.zone/questions",
                        "hx-boost": False,
                        "class": "flex items-center justify-center py-6 px-4 gap-1.5 rounded-lg w-full max-w-96 mx-auto mt-4 hover:bg-white-700 hover:cursor-pointer",
                        "style": "background-color: rgba(87,192,239,0.4);",
                    },
                    f"Start Your {span_({'class': 'font-bold'}, 'Free')} Health Assessment",
                    svg_(
                        {
                            "id": "arrow",
                            "class": "w-6 h-6 ml-2",
                            "xmlns": "http://www.w3.org/2000/svg",
                            "viewBox": "0 0 24 24",
                            "stroke": "#ffffff",  # Change this to the color you want
                            "fill": "none",
                        },
                        '<path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M9 5l7 7-7 7"/>',
                    ),
                ),
                style_(
                    """canvas {
width:100%;
height:100%;

max-width:500px;
max-height:500px;
}

@keyframes scaleArrow {
    0% { transform: scale(1); }
    50% { transform: scale(1.18); }
    100% { transform: scale(1); }
}

"""
                ),
            ),
        ),
        script_(
            """
    window.addEventListener('DOMContentLoaded', (event) => {
        const svgElement = document.getElementById('arrow');
        if(svgElement) {
            svgElement.style.animation = 'scaleArrow 2s infinite';
        }
    });
    """
        ),
        script_(
            {
                "src": "/assets/js/particle-system.js",
                "defer": True,
            }
        ),
        extra_headers=[
            title_("Biometric Health Assessment"),
        ],
        body_style="bg-[#2607e7] text-white",
    )
