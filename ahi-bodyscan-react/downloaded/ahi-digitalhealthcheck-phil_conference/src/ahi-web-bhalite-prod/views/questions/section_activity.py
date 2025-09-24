from hypymedia import div_, fieldset_, h3_, input_, label_, legend_, p_, section_

SECTION_OPTIONS = [
    {
        "id": "Sedentary",
        "value": "inactive",
        "description": "Little to no physical activity beyond daily tasks",
    },
    {
        "id": "Lightly active",
        "value": "exercise10Mins",
        "description": "Engage in light physical activities like walking or gentle exercises for at least ten minutes at a time, at least five days a week",
    },
    {
        "id": "Moderately active",
        "value": "exercise20to60Mins",
        "description": "Engage in aerobic exercises for twenty to sixty minutes a week",
    },
    {
        "id": "Fairly active",
        "value": "exercise1to3Hours",
        "description": "Engage in aerobic exercises for one to three hours a week",
    },
    {
        "id": "Highly active",
        "value": "exerciseOver3Hours",
        "description": "Engage in aerobic exercises for over three hours a week",
    },
]


# FIXME: This is a temporary solution to the issue of the id and description keys
def option(id_: str, value_: str, description: str) -> str:
    return label_(
        {
            "for": id_,
            "class": "flex flex-row border rounded-xl justify-between p-4 hover:bg-[#57C0EF]/20",
        },
        div_(
            {"class": "flex flex-col"},
            h3_(
                id_.replace("_", " ").capitalize(),
            ),
            p_(
                {"class": "text-sm pr-6"},
                description,
            ),
        ),
        input_(
            {
                "type": "radio",
                "id": id_,
                "name": "activity",
                "value": value_,
                "required": True,
            }
        ),
    )


def get_options() -> str:
    return "".join(
        option(
            option_data.get("id"),
            option_data.get("value"),
            option_data.get("description"),
        )
        for option_data in SECTION_OPTIONS
    )


def section_activity(section_header: callable) -> str:
    return section_(
        {"data-index": True, "class": "hidden flex flex-col gap-4 fade-in"},
        section_header(
            title="Activity Level",
            direction="Choose one activity category that best describes your usual pattern of daily exercise",
        ),
        fieldset_(
            {"class": "flex flex-col gap-2"},
            legend_({"class": "sr-only"}, "Activity Level:"),
            get_options(),
        ),
    )
