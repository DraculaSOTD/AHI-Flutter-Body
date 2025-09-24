from hypymedia import fieldset_, input_, label_, legend_, section_


def section_chronic_medications(section_header: callable) -> str:
    return section_(
        {"data-index": True, "class": "hidden flex flex-col gap-4 fade-in"},
        section_header(
            title="Do you use chronic medication?",
        ),
        fieldset_(
            {"class": "flex flex-col gap-2"},
            legend_({"class": "sr-only"}, "Are you on Chronic Medication?"),
        ),
        label_(
            {
                "for": "medication_chronic_none",
                "class": "flex flex-row border rounded-xl justify-between p-4 hover:bg-[#57C0EF]/20",
            },
            "No",
            input_(
                {
                    "type": "radio",
                    "id": "medication_chronic_none",
                    "name": "chronic_medication",
                    "value": "none",
                    "required": True,
                }
            ),
        ),
        label_(
            {
                "for": "medication_chronic_oneOrTwoDiseases",
                "class": "flex flex-row border rounded-xl justify-between p-4 hover:bg-[#57C0EF]/20",
            },
            "Medication for one or two diseases",
            input_(
                {
                    "type": "radio",
                    "id": "medication_chronic_oneOrTwoDiseases",
                    "name": "chronic_medication",
                    "value": "oneOrTwoDiseases",
                    "required": True,
                }
            ),
        ),
        label_(
            {
                "for": "medication_chronic_threeOrMoreDiseases",
                "class": "flex flex-row border rounded-xl justify-between p-4 hover:bg-[#57C0EF]/20",
            },
            "Medication for three or more diseases",
            input_(
                {
                    "type": "radio",
                    "id": "medication_chronic_threeOrMoreDiseases",
                    "name": "chronic_medication",
                    "value": "threeOrMoreDiseases",
                    "required": True,
                }
            ),
        ),
    )
