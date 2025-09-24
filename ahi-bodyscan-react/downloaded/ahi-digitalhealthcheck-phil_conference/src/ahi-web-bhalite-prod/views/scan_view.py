from controllers.sessionController import get_session
from hypymedia import (
    body_,
    canvas_,
    div_,
    form_,
    head_,
    html_,
    link_,
    meta_,
    script_,
    section_,
    title_,
    video_,
)

# from views.template import template
# Tailwind is currently causing this to die a horrid death.
# We need to figure out how to get it to work with Tailwind.
# In the meantime I'll return a custom template


def content_model(session_id: str):
    # Need to get biological sex, age, height, and weight from the user
    session_data = get_session(session_id)

    return content(session_id, session_data)


def content(session_id: str, session_data):
    return "<!DOCTYPE html>" + html_(
        {"lang": "en"},
        head_(
            meta_({"charset": "UTF-8"}),
            meta_(
                {"name": "viewport", "content": "width=device-width, initial-scale=1.0"}
            ),
            script_(
                {
                    "src": "https://unpkg.com/htmx.org@1.9.12",
                    "integrity": "sha384-ujb1lZYygJmzgSwoxRggbCHcjc0rB2XoQrxeTUQyRjrOnlCoYta87iKBWq3EsdM2",
                    "crossorigin": "anonymous",
                    "defer": True,
                }
            ),
            script_(
                {
                    "src": "https://unpkg.com/htmx.org/dist/ext/response-targets.js",
                    "defer": True,
                }
            ),
            script_(
                {
                    "src": "https://unpkg.com/@rive-app/canvas@2.9.1",
                }
            ),
            link_(
                {
                    "rel": "stylesheet",
                    "type": "text/css",
                    "href": "/assets/css/ratio.css",
                }
            ),
            title_("Biometric Health Assessment Scan"),
        ),
        body_(
            section_(
                {"id": "container"},
                video_(
                    {
                        "id": "video",
                        "playsinline": True,
                    }
                ),
                canvas_({"id": "canvas"}),
                div_(
                    {"id": "chart-overlay"},
                    canvas_({"id": "chart"}),
                ),
            ),
            form_(
                {
                    "action": f"https://cfbha.ahi.zone/scan/{session_id}",
                    "method": "post",
                    "id": "post-scan",
                    "class": "hidden",
                    "data-sex": str(session_data.get("enum_ent_sex")),
                    "data-age": str(session_data.get("yr_raw_age")),
                    "data-height": str(session_data.get("cm_ent_height")),
                    "data-weight": str(session_data.get("kg_ent_weight")),
                },
            ),
            script_({"src": "/assets/js/lib/rppg.min.js"}),
            script_({"src": "/assets/js/lib/index.js"}),
        ),
    )
