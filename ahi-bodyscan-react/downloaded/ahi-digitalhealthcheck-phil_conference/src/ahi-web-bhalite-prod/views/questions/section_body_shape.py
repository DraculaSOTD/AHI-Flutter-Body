from hypymedia import fieldset_, input_, label_, legend_, section_


def section_body_shape(section_header: callable) -> str:
    return section_(
        {"data-index": True, "class": "hidden flex flex-col gap-4 fade-in"},
        section_header(
            title="Your body",
            direction="Select shape",
        ),
        fieldset_(
            {"class": "flex flex-col gap-2"},
            legend_({"class": "sr-only"}, "Body shape:"),
            label_(
                {
                    "for": "ectomorphic",
                    "class": "flex flex-row border rounded-xl justify-between p-4 hover:bg-[#57C0EF]/20",
                },
                "Ectomorphic (Rectangle)",
                input_(
                    {
                        "type": "radio",
                        "id": "ectomorphic",
                        "name": "body_shape",
                        "value": "ectomorphic",
                        "required": True,
                    }
                ),
            ),
            label_(
                {
                    "for": "mesomorphic",
                    "class": "flex flex-row border rounded-xl justify-between p-4 hover:bg-[#57C0EF]/20",
                },
                "Mesomorphic (Triangle)",
                input_(
                    {
                        "type": "radio",
                        "id": "mesomorphic",
                        "name": "body_shape",
                        "value": "mesomorphic",
                        "required": True,
                    }
                ),
            ),
            label_(
                {
                    "for": "endomorphic",
                    "class": "flex flex-row border rounded-xl justify-between p-4 hover:bg-[#57C0EF]/20",
                },
                "Endomorphic (Circle)",
                input_(
                    {
                        "type": "radio",
                        "id": "endomorphic",
                        "name": "body_shape",
                        "value": "endomorphic",
                        "required": True,
                    }
                ),
            ),
        ),
    )
