import datetime

from hypymedia import input_, label_, section_


def section_dob(section_header: callable, action: callable) -> str:
    return section_(
        {
            "data-index": True,
            "class": "hidden flex flex-col gap-4 fade-in",
        },
        section_header(
            title="Date of Birth",
            direction="Please enter your date of birth",
        ),
        label_({"for": "dob", "class": "sr-only"}, "Date of Birth:"),
        input_(
            {
                "type": "date",
                "id": "dob",
                "name": "dob",
                "required": True,
                "max": (
                    datetime.datetime.now() - datetime.timedelta(days=18 * 365)
                ).strftime("%Y-%m-%d"),
                "placeholder": (
                    datetime.datetime.now() - datetime.timedelta(days=18 * 365)
                ).strftime("%d-%m-%Y"),
                "class": "w-full p-2 border border-gray-300 rounded bg-white p-4",
                "style": "-webkit-min-logical-width: calc(100% - 16px);",  # Fix for Safari
            }
        ),
        action(),
    )
