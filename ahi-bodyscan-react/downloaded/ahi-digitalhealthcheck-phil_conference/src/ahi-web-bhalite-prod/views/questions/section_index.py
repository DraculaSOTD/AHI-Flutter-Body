from hypymedia import (
    a_,
    div_,
    h2_,
    h3_,
    header_,
    p_,
    section_,
)


def section_index(action: callable) -> str:
    return section_(
        {"data-index": True, "class": "flex flex-col gap-2"},
        a_(
            {
                "href": "https://cfbha.ahi.zone/",
                "class": "flex mt-5 items-center justify-center w-8 h-8 bg-white drop-shadow-md rounded-lg cursor-pointer select-none",
                "id": "back-button",
            },
            "<",
        ),
        p_({"class": "text-xs mt-6"}, "🕐 2 - 3 minutes"),
        header_(
            {"class": "text-2xl text-[#2607E7]"},
            h2_("When to do the Digital Health Check"),
        ),       
        div_(
            {"class": "mt-6"},
            h3_(
                {"class": ""},
                "<b>Are you ill?</b> Choose a time when you're feeling your usual self, not when you're feeling unwell.",
            ),            
        ),
        div_(
            {"class": "mt-6"},
            h3_(
                {"class": ""},
                "<b>Exercised recently?</b> Wait at least an hour after exercising before beginning.",
            ),            
        ),
        div_(
            {"class": "mt-6"},
            h3_(
                {"class": ""},
                "<b>Had caffeine?</b> Wait at least an hour after consuming caffeine before beginning.",
            ),            
        ),
        div_(
            {"class": "mt-6"},
            h3_(
                {"class": ""},
                "<b>Comfortable?</b> Use the restroom if needed, sit in a supportive chair with your feet flat on the floor, and ensure good, even, lighting on your face.",
            ),            
        ),
        div_(
            {"class": "mt-6"},
            h3_(
                {"class": ""},
                "",
            ),             
            p_(
                {"class": "text-sm block mt-2 text-left"},
                "Completing the Digital Health Check at the right time will help you get the best results. This is a quick snapshot of your estimated health risk and does not replace the more extensive Digital Health Assessment.",
            ),
        ),
        action(),
    )