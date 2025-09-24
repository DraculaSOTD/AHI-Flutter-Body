#!/bin/bash

poetry export -f requirements.txt --output requirements.txt
pip install -r requirements.txt --target ./ahi-web-bhalite-prod/lib/ --upgrade
