import json
import logging
from urllib import request

from controllers.sessionController import get_session

logging.basicConfig(level=logging.INFO)


def get_bha_results(session_id: str, extra_data: dict):
    base_url = "https://o5t5ewcghf.execute-api.ap-southeast-2.amazonaws.com/bha/"

    try:
        session_data = get_session(session_id)
    except Exception as e:
        print(f"Error getting session data: {e}")
        session_data = {}

    person_data = {}

    # Define a dictionary to map session_data keys to person_data keys
    data_keys = {
        "session_data": {
            "enum_ent_activityLevel": str,
            "enum_ent_chronicMedication": str,
            "enum_ent_smoker": str,
            "bool_ent_bpMedication": bool,
            "enum_ent_sex": str,
            "date_ent_dob": str,
            "cm_ent_height": int,
            "kg_ent_weight": float,
            "health_score": float,
        },
        "extra_data": {
            "mmHg_ent_systolicBP": int,
            "mmHg_ent_diastolicBP": int,
            "bpm_ent_restingHeartRate": int,
        },
    }

    def extract_and_convert_data(source_data, keys):
        for key, data_type in keys.items():
            if value := source_data.get(key):
                person_data[key] = data_type(value)

    # Extract and convert data from session_data and extra_data
    extract_and_convert_data(session_data, data_keys["session_data"])
    extract_and_convert_data(extra_data, data_keys["extra_data"])

    req = request.Request(
        url=base_url,
        data=json.dumps(person_data).encode(),
        headers={"Content-Type": "application/json"},
        method="POST",
    )

    try:
        with request.urlopen(req) as response:
            response_body = response.read()
    except Exception as e:
        print(e)

    return response_body.decode()


# def get_bha_results(session_id: str, extra_data: dict):
#     base_url = "https://o5t5ewcghf.execute-api.ap-southeast-2.amazonaws.com/bha/"

#     try:
#         session_data = get_session(session_id)
#     except Exception as e:
#         print(f"Error getting session data: {e}")
#         session_data = {}

#     person_data = {}

#     if activity := session_data.get("enum_ent_activityLevel"):
#         person_data["enum_ent_activityLevel"] = activity

#     if chronic_meds := session_data.get("enum_ent_chronicMedication"):
#         person_data["enum_ent_chronicMedication"] = chronic_meds

#     if smoker := session_data.get("enum_ent_smoker"):
#         person_data["enum_ent_smoker"] = smoker

#     if ent_blood_meds := session_data.get("bool_ent_bpMedication"):
#         person_data["bool_ent_bpMedication"] = bool(ent_blood_meds)

#     if bio_sex := session_data.get("enum_ent_sex"):
#         person_data["enum_ent_sex"] = bio_sex

#     if dob := session_data.get("date_ent_dob"):
#         person_data["date_ent_dob"] = dob

#     if height := session_data.get("cm_ent_height"):
#         person_data["cm_ent_height"] = int(height)

#     if weight := session_data.get("kg_ent_weight"):
#         person_data["kg_ent_weight"] = float(weight)

#     if systolic := int(extra_data.get("mmHg_ent_systolicBP")):
#         person_data["mmHg_ent_systolicBP"] = systolic

#     if diastolic := int(extra_data.get("mmHg_ent_diastolicBP")):
#         person_data["mmHg_ent_diastolicBP"] = diastolic

#     if heartRate := int(extra_data.get("bpm_ent_restingHeartRate")):
#         person_data["bpm_ent_restingHeartRate"] = heartRate

#     req = request.Request(
#         url=base_url,
#         data=json.dumps(person_data).encode(),
#         headers={"Content-Type": "application/json"},
#         method="POST",
#     )

#     try:
#         with request.urlopen(req) as response:
#             response_body = response.read()
#     except Exception as e:
#         print(e)

#     return response_body.decode()
