import json
import decimal

from controllers.EdgeResponse import HTMLresponse, RedirectResponse
from controllers.EdgeRouter import EdgeRouter
from controllers.headlessBHAController import get_bha_results
from controllers.sessionController import update_session
from models.healthscore_model import get_healthscore

app = EdgeRouter()


# INDEX


@app.get("/")
def index(**kwargs):
    from views.index_view import content_model

    content = content_model()
    return HTMLresponse(
        body=content,
        status="200",
        statusDescription="OK",
    )


# CONFERENCE LANDING FORM


@app.post("/submit-form")
def post_form(**kwargs):
    from urllib.parse import parse_qs
    # Quickly create a session_id to tie it all together
    from controllers.sessionController import create_session
    from controllers.sessionController import update_landing_post

    body = kwargs.get("body", None)
    parsed_body = parse_qs(body) if body else None

    print(f"PARSED BODY: {parsed_body}")

    session_id = create_session()

    if parsed_body:
        try:
            update_landing_post(session_id, parsed_body)
        except ValueError:
            return RedirectResponse(f"/error/{session_id}")

    return RedirectResponse(f"/questions/{session_id}")


# QUESTIONS


@app.get("/questions")
def get_questions(**kwargs):
    # Quickly create a session_id to tie it all together
    from controllers.sessionController import create_session

    session_id = create_session()

    return RedirectResponse(f"/questions/{session_id}")


@app.get("/questions/{session_id}")
def get_questions_session(session_id: str, **kwargs):
    from views.questions.questions_view import content_model

    content = content_model(session_id)
    return HTMLresponse(
        body=content,
        status="200",
        statusDescription="OK",
    )


@app.post("/questions/{session_id}")
def post_questions(session_id: str, **kwargs):
    from urllib.parse import parse_qs

    from controllers.sessionController import update_session_questions

    body = kwargs.get("body", None)
    parsed_body = parse_qs(body) if body else None

    # print(f"PARSED BODY: {parsed_body}")

    if parsed_body:
        try:
            update_session_questions(session_id, parsed_body)
        except ValueError:
            return RedirectResponse(f"/error/{session_id}")

    return RedirectResponse(f"/scan/{session_id}")


#  SCAN


@app.get("/scan/{session_id}")
def get_scan(session_id: str, **kwargs):
    from views.scan_view import content_model

    content = content_model(session_id)
    return HTMLresponse(
        body=content,
        status="200",
        statusDescription="OK",
    )


@app.post("/scan/{session_id}")
def post_scan(session_id: str, **kwargs):
    from urllib.parse import parse_qs

    body = kwargs.get("body", None)
    parsed_body = parse_qs(body) if body else None

    # print(f"PARSED BODY SCAN: {parsed_body}")

    parsed_body = {k: v[0] for k, v in parse_qs(body).items()} if body else None

    # ... then we get results from headless BHA
    bha_data = json.loads(
        get_bha_results(
            session_id=session_id,
            extra_data=parsed_body,
        )
    )

    print(f"BHA: {bha_data}")

    risks = bha_data.get("risks", {})        
    
    # Assign health score    
    health_score = str(get_healthscore(bha_data))
    
    # Ensure 'risks' key exists in bha_data
    bha_data.setdefault("risks", {})

    # Store health score in multiple places
    risks["health_score"] = health_score
    parsed_body["health_score"] = health_score
    bha_data["risks"]["health_score"] = health_score

    # Merge without overwriting risks
    bha_data.update(parsed_body)

    combined_data = {**risks, **parsed_body}

    # ... then we update the database again.
    update_session(session_id, combined_data)

    return RedirectResponse(
        f"/result/{session_id}",
        headers={
            "HX-Redirect": [{"key": "HX-Redirect", "value": f"/result/{session_id}"}]
        },
    )


# RESULT

class DecimalEncoder(json.JSONEncoder):
    def default(self, o):
        if isinstance(o, decimal.Decimal):
            return str(o)
        return super().default(o)


@app.get("/result/{session_id}")
def get_result(session_id: str, **kwargs):
    from views.result_view import content_model
    from controllers.sessionController import get_session

    session = get_session(session_id)
    if "ResultUrl" in session and session["ResultUrl"] is not None:
        # Base64 encode result
        import base64
        url = session["ResultUrl"]
        json_bytes = json.dumps(session, cls=DecimalEncoder).encode('utf-8')
        base64_str = base64.urlsafe_b64encode(json_bytes).decode('utf-8').rstrip('=')
        url_result = f"{url}?result={base64_str}"
        return {
            "status": "303",
            "statusDescription": "See Other",
            "headers": {
                "Location": [{"key": "Location", "value": url_result}],
                "HX-Redirect": [{"key": "HX-Redirect", "value": url_result}]
            },
            "body": "",
        }
    
    # Not redirect, so display result page instead 

    content = content_model(session_id=session_id)

    return HTMLresponse(
        body=content,
        status="200",
        statusDescription="OK",
    )


# ERROR


@app.get("/error/{session_id}")
def get_error(session_id: str, **kwargs):
    from views.error_view import content_model
    from controllers.sessionController import get_session

    session = get_session(session_id)
    if "ResultUrl" in session and session["ResultUrl"] is not None:
        # Base64 encode result
        import base64
        url = session["ResultUrl"]
        json_bytes = json.dumps(session, cls=DecimalEncoder).encode('utf-8')
        base64_str = base64.urlsafe_b64encode(json_bytes).decode('utf-8').rstrip('=')
        url_result = f"{url}?error={base64_str}"
        return {
            "status": "303",
            "statusDescription": "See Other",
            "headers": {
                "Location": [{"key": "Location", "value": url_result}],
                "HX-Redirect": [{"key": "HX-Redirect", "value": url_result}]
            },
            "body": "",
        }
    
    # Not redirect, so display result page instead 

    content = content_model()
    return HTMLresponse(
        body=content,
        status="200",
        statusDescription="OK",
    )


def handler(event: dict, context):
    # Extract the request from the CloudFront event
    request = event["Records"][0]["cf"]["request"]
    body = request.get("body", {}).get("data", None)

    return app.dispatch(request, body=body)
