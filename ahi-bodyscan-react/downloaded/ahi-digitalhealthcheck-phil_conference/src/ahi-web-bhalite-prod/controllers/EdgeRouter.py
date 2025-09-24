import base64
import inspect
import re


class EdgeRouter:
    def __init__(self):
        self.routes = []

    def add_route(self, method, path, handler):
        pattern = self._path_to_regex(path)
        self.routes.append((method, pattern, handler))

    def _path_to_regex(self, path):
        path_regex = re.sub(r"{(\w+)}", r"(?P<\1>[^/]+)", path)
        return re.compile(f"^{path_regex}$")

    def route(self, method, path):
        def decorator(func):
            self.add_route(method, path, func)
            return func

        return decorator

    def get(self, path):
        return self.route("GET", path)

    def post(self, path):
        return self.route("POST", path)

    def _get_handler_kwargs(self, handler, match):
        sig = inspect.signature(handler)
        return {
            param.name: match.group(param.name)
            for param in sig.parameters.values()
            if param.name in match.groupdict()
        }

    def _get_body_data(self, body):
        if body is not None and isinstance(body, dict):
            if body.get("encoding") == "base64":
                return base64.b64decode(body.get("data", "")).decode("utf-8")
            else:
                return body.get("data", "")
        else:
            return body

    def dispatch(self, request, body=None):
        method = request["method"]
        uri = request["uri"]
        body = request.get("body", body)  # Extract cookies from the request

        for route_method, pattern, handler in self.routes:
            if method == route_method:
                match = pattern.fullmatch(uri)
                if match:
                    kwargs = self._get_handler_kwargs(handler, match)
                    body_data = self._get_body_data(body)
                    response = handler(
                        **kwargs,
                        body=body_data,
                    )

                    return response

        return {
            "status": "404",
            "statusDescription": "Not Found",
            "headers": {
                "content-type": [{"key": "Content-Type", "value": "text/plain"}]
            },
            "body": "404 Not Found",
        }
