import base64
from datetime import datetime, timezone
from uuid import uuid4

import boto3
from boto3.dynamodb.conditions import Attr
from models.questions_model import FormInput

dynamodb = boto3.resource("dynamodb", region_name="us-east-1")
table = dynamodb.Table("ahi-bha-lite-web-session-db")


def get_time() -> int:
    return int(datetime.now(timezone.utc).timestamp())


def create_session_id() -> str:
    return str(uuid4())


def decode_email64(email64: str) -> str:
    return base64.b64decode(email64).decode("utf-8")


def encode_email64(email: str) -> str:
    return base64.b64encode(email.encode("utf-8")).decode()


def create_session() -> None:
    sid = create_session_id()
    item = {
        "session_id": sid,
        "created_at": get_time(),
    }

    try:
        table.put_item(
            Item=item,
            ConditionExpression=Attr("session_id").not_exists(),
        )
    except Exception as e:
        print(e)

    return sid


# For Conference / Sales
def update_landing_post(session_id: str, parsed_body: dict) -> str:
    item = dict()
    item['first_name'] = parsed_body.get("first-name", [None])[0]
    item['last_name'] = parsed_body.get("last-name", [None])[0]
    item['org_name'] = parsed_body.get("organisation-name", [None])[0]
    item['web_url'] = parsed_body.get("website", [None])[0]
    item['email'] = parsed_body.get("email", [None])[0]
    item['contact_number'] = parsed_body.get("contact-number", [None])[0]
    item['agree_subscribe'] = parsed_body.get("subscribe", [None])[0]
    item["form_entered_at"] = get_time()

    # append pass-through keys
    for key, value in parsed_body.items():
        if key.startswith("ef_"):
            item[key] = parsed_body.get(key, [None])[0]

    update_expression = "SET " + ", ".join(f"{k} = :{k}" for k in item.keys())
    expression_attribute_values = {f":{k}": v for k, v in item.items()}

    try:
        table.update_item(
            Key={"session_id": session_id},
            UpdateExpression=update_expression,
            ExpressionAttributeValues=expression_attribute_values,
        )
    except Exception as e:
        print(e)

    return session_id


def update_session_questions(session_id: str, parsed_body: dict) -> str:
    biological_sex = parsed_body.get("biological_sex", [None])[0]
    dob = parsed_body.get("dob")[0]
    height = parsed_body.get("height", [None])[0]
    weight = parsed_body.get("weight", [None])[0]
    # body_shape = parsed_body.get("body_shape", [None])[0]
    smoker_status = parsed_body.get("smoker", [None])[0]
    activity = parsed_body.get("activity", [None])[0]
    chronic_medication = parsed_body.get("chronic_medication", [None])[0]
    bp_medication = parsed_body.get("bp_medication", [None])[0]
    email = parsed_body.get("email", [None])[0]
    url = parsed_body.get("ResultUrl", [None])[0]

    try:
        formInput = FormInput(
            email=email,
            enum_ent_sex=biological_sex,
            date_ent_dob=dob,
            cm_ent_height=height,
            kg_ent_weight=weight,
            # enum_ent_bodyshape=body_shape,
            enum_ent_smoker=smoker_status,
            enum_ent_activityLevel=activity,
            enum_ent_chronicMedication=chronic_medication,
            bool_ent_bpMedication=bp_medication,
            ResultUrl=url
        )
    except ValueError as e:
        print(e)
        return None

    item = formInput.model_dump()
    item["questions_at"] = get_time()

    # append pass-through keys
    for key, value in parsed_body.items():
        if key.startswith("ef_"):
            item[key] = parsed_body.get(key, [None])[0]

    update_expression = "SET " + ", ".join(f"{k} = :{k}" for k in item.keys())
    expression_attribute_values = {f":{k}": v for k, v in item.items()}

    try:
        table.update_item(
            Key={"session_id": session_id},
            UpdateExpression=update_expression,
            ExpressionAttributeValues=expression_attribute_values,
        )
    except Exception as e:
        print(e)

    return session_id


def update_session(session_id: str, data: dict) -> None:
    data["updated_at"] = get_time()

    update_expression = "SET " + ", ".join(f"{k} = :{k}" for k in data.keys())
    expression_attribute_values = {f":{k}": v for k, v in data.items()}

    try:
        table.update_item(
            Key={"session_id": session_id},
            UpdateExpression=update_expression,
            ExpressionAttributeValues=expression_attribute_values,
        )
    except Exception as e:
        print(e)


def get_session(session_id: str) -> dict:
    response = table.get_item(Key={"session_id": session_id})
    return response.get("Item", {})
