from hypymedia import fieldset_, input_, label_, legend_, section_


def section_history(section_header: callable) -> str:
    return section_(
        {"data-index": True, "class": "hidden flex flex-col gap-4 fade-in"},
        section_header(
            title="Do you have a family history of heart disease, cancer, or diabetes?"
        ),
        fieldset_(
            {"class": "flex flex-col gap-2"},
            legend_({"class": "sr-only"}, "Family History:"),
            # No
            label_(
                {
                    "for": "history-negative",
                    "class": "flex flex-row border rounded-xl justify-between p-4 hover:bg-[#57C0EF]/20",
                },
                "No",
                input_(
                    {
                        "type": "radio",
                        "id": "history-negative",
                        "name": "history",
                        "value": "No",
                        "required": True,
                    }
                ),
            ),
            # Yes
            label_(
                {
                    "for": "history-positive",
                    "class": "flex flex-row border rounded-xl justify-between p-4 hover:bg-[#57C0EF]/20",
                },
                "Yes",
                input_(
                    {
                        "type": "radio",
                        "id": "history-positive",
                        "name": "history",
                        "value": "Yes",
                        "required": True,
                    }
                ),
            ),
            label_(
                {
                    "for": "history-unsure",
                    "class": "flex flex-row border rounded-xl justify-between p-4 hover:bg-[#57C0EF]/20",
                },
                "Not sure",
                input_(
                    {
                        "type": "radio",
                        "id": "history-unsure",
                        "name": "history",
                        "value": "unsure",
                        "required": True,
                    }
                ),
            ),
        ),
    )
