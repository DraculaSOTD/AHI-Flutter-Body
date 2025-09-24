from hypymedia import fieldset_, input_, label_, legend_, section_


def section_bp_medications(section_header: callable) -> str:
    return section_(
        {"data-index": True, "class": "hidden flex flex-col gap-4 fade-in"},
        section_header(
            title="Do you take blood pressure medication?",
        ),
        fieldset_(
            {"class": "flex flex-col gap-2"},
            legend_({"class": "sr-only"}, "Are you on Blood Pressure Medication?"),
        ),
        label_(
            {
                "for": "medication_bp_false",
                "class": "flex flex-row border rounded-xl justify-between p-4 hover:bg-[#57C0EF]/20",
            },
            "No",
            input_(
                {
                    "type": "radio",
                    "id": "medication_bp_false",
                    "name": "bp_medication",
                    "value": "false",
                    "required": True,
                }
            ),
        ),
        label_(
            {
                "for": "medication_bp_true",
                "class": "flex flex-row border rounded-xl justify-between p-4 hover:bg-[#57C0EF]/20",
            },
            "Yes",
            input_(
                {
                    "type": "radio",
                    "id": "medication_bp_true",
                    "name": "bp_medication",
                    "value": "true",
                    "required": True,
                }
            ),
        ),
    )
