from typing import Optional

from hypymedia import body_, head_, html_, meta_, script_

head = [
    meta_({"charset": "UTF-8"}),
    meta_({"name": "viewport", "content": "width=device-width, initial-scale=1.0"}),
    script_({"src": "https://cdn.tailwindcss.com"}),
    script_(
        {
            "src": "https://unpkg.com/htmx.org@1.9.12",
            "integrity": "sha384-ujb1lZYygJmzgSwoxRggbCHcjc0rB2XoQrxeTUQyRjrOnlCoYta87iKBWq3EsdM2",
            "crossorigin": "anonymous",
            "defer": True,
        }
    ),
    script_(
        {
            "src": "https://unpkg.com/htmx.org/dist/ext/response-targets.js",
            "defer": True,
        }
    ),
    script_(
        {
            "src": "https://cdnjs.cloudflare.com/ajax/libs/flowbite/2.4.1/flowbite.js",
            "integrity": "sha512-HoR86EEia6LhUfMMFBzFB+nWZeEpYdYcpU4qe0SIEvqj1nqoaQniIyfxkwfGZ8fO5NFwMEL3cjdc0pVb8FHHfA==",
            "crossorigin": "anonymous",
            "referrerpolicy": "no-referrer",
        }
    ),
]


def template(*args, extra_headers: Optional[list[str]] = None, body_style: str = None):
    content = []
    for arg in args:
        if isinstance(arg, str):
            content.append(arg)

    headers = head + extra_headers if extra_headers else head

    return "<!DOCTYPE html>" + (
        html_(
            {"lang": "en", "class": f"{body_style}"},
            head_(*headers),
            body_(
                {"hx-ext": "response-targets"},
                *content,
            ),
        )
    )
