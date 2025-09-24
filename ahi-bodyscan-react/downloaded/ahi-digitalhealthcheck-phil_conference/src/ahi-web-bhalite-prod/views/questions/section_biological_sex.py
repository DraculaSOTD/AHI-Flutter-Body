from hypymedia import (
    div_,
    fieldset_,
    img_,
    input_,
    label_,
    legend_,
    p_,
    section_,
)


def section_biological_sex(section_header: callable) -> str:
    return section_(
        {
            "data-index": True,
            "class": "hidden flex flex-col gap-4 fade-in",
        },
        section_header(
            title="Biological Sex",
        ),
        fieldset_(
            {"class": "flex flex-row grow justify-around"},
            legend_({"class": "sr-only"}, "Biological Sex:"),
            # Male
            label_(
                {
                    "for": "male",
                    "class": "border rounded p-4 bg-white hover:bg-[#57C0EF]/20",
                },
                input_(
                    {
                        "type": "radio",
                        "id": "male",
                        "name": "biological_sex",
                        "value": "male",
                        "required": True,
                    }
                ),
                div_(
                    {"class": "flex flex-col justify-center items-center"},
                    img_({"src": "/assets/images/male.svg"}),
                    p_("Male"),
                ),
            ),
            # Female
            label_(
                {
                    "for": "female",
                    "class": "border rounded p-4 bg-white hover:bg-[#57C0EF]/20",
                },
                input_(
                    {
                        "type": "radio",
                        "id": "female",
                        "name": "biological_sex",
                        "value": "female",
                        "required": True,
                    }
                ),
                div_(
                    {"class": "flex flex-col justify-center items-center"},
                    img_({"src": "/assets/images/female.svg"}),
                    p_("Female"),
                ),
            ),
        ),
    )
