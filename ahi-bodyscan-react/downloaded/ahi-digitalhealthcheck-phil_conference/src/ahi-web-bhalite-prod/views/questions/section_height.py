from hypymedia import (
    input_,
    label_,
    section_,
)


def section_height(section_header: callable, action: callable) -> str:
    return section_(
        {
            "data-index": True,
            "class": "hidden flex flex-col gap-4 fade-in",
        },
        section_header(
            title="Height",
            direction="Please enter your height in centimeters.",
        ),
        label_({"for": "height", "class": "sr-only"}, "Height (in CM):"),
        input_(
            {
                "type": "number",
                "id": "height",
                "name": "height",
                "required": True,
                "min": 100,
                "max": 250,
                "class": "w-full p-2 border border-gray-300 rounded p-4",
            }
        ),
        action(),
    )
