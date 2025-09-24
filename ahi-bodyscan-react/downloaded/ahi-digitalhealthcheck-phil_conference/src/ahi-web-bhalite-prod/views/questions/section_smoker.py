from hypymedia import fieldset_, input_, label_, legend_, section_


def section_smoker(section_header: callable) -> str:
    return section_(
        {"data-index": True, "class": "hidden flex flex-col gap-4 fade-in"},
        section_header(title="Do you smoke?"),
        fieldset_(
            {"class": "flex flex-col gap-2"},
            legend_({"class": "sr-only"}, "Smoking status:"),
            label_(
                {
                    "for": "smoker-negative",
                    "class": "flex flex-row border rounded-xl justify-between p-4 hover:bg-[#57C0EF]/20",
                },
                "No, or more than 5 years ago",
                input_(
                    {
                        "type": "radio",
                        "id": "smoker-negative",
                        "name": "smoker",
                        "value": "never",
                        "required": True,
                    }
                ),
            ),
            label_(
                {
                    "for": "smoker-former",
                    "class": "flex flex-row border rounded-xl justify-between p-4 hover:bg-[#57C0EF]/20",
                },
                "Within past 5 years",
                input_(
                    {
                        "type": "radio",
                        "id": "smoker-former",
                        "name": "smoker",
                        "value": "former",
                        "required": True,
                    }
                ),
            ),
            label_(
                {
                    "for": "smoker-current",
                    "class": "flex flex-row border rounded-xl justify-between p-4 hover:bg-[#57C0EF]/20",
                },
                "Yes, Currently",
                input_(
                    {
                        "type": "radio",
                        "id": "smoker-current",
                        "name": "smoker",
                        "value": "current",
                        "required": True,
                    }
                ),
            ),
        ),  # end fieldset
    )
