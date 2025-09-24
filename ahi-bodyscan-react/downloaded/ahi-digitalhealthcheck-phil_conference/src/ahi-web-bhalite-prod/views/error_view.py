from hypymedia import a_, h1_, h2_, header_, main_, p_, section_, span_
from views.template import template


def content_model():
    return content()


def content():
    return template(
        header_(
            h1_("Biometric Health Assessment"),  # Hide for AHI logo
            # Interactive image
        ),
        main_(
            section_(
                h2_(f"Welcome to the the {span_('Biometric health assessment')}"),
                p_("Sadly we are unable to process your request at this time."),
                a_({"href": "https://cfbha.ahi.zone/questions"}, "Restart assessment"),
            ),
        ),
    )
