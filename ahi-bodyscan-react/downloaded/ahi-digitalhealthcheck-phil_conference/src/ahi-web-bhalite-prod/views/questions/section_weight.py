from hypymedia import input_, label_, section_


def section_weight(
    section_header: callable,
    action: callable,
) -> str:
    return section_(
        {"data-index": True, "class": "hidden flex flex-col gap-4 fade-in"},
        section_header(
            title="Weight",
            direction="Please enter your weight in kilograms.",
        ),
        label_({"for": "weight", "class": "sr-only"}, "Weight (in KG):"),
        input_(
            {
                "type": "number",
                "id": "weight",
                "name": "weight",
                "required": True,
                "class": "w-full p-2 border border-gray-300 rounded p-4",
                "min": "30",
                "max": "250",
            }
        ),
        action(),
    )
