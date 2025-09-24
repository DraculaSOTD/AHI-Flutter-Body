"""
This file is designed to replicate CloudFront behaviors locally for testing purposes.
It should not be included in the final Lambda distributions as it is only meant for local development and debugging.
"""

import mimetypes
import os
import ssl
import sys
import threading
import time
from http.cookies import SimpleCookie
from http.server import BaseHTTPRequestHandler, HTTPServer
from pathlib import Path

from index import handler  # Import the Lambda handler

# Insert the path to the directory you want to import from at the start of the Python system path
lib_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "lib")
sys.path.insert(0, lib_dir)


class RequestHandler(BaseHTTPRequestHandler):
    """
    A class used to handle HTTP requests.
    """

    def serve_static_file(self, path):
        """
        Serve a static file from the local filesystem.

        Parameters:
        path (str): The path to the file to serve.
        """
        # Remove the leading '/' from the path
        relative_path = path.lstrip("/")
        # Find the absolute path of the file
        file_path = os.path.join(os.path.dirname(__file__), relative_path)
        # Check if the file exists and is a file
        if os.path.isfile(file_path):
            # Determine the content type
            content_type, _ = mimetypes.guess_type(file_path)
            # Read the file content
            with open(file_path, "rb") as file:
                self.send_response(200)
                self.send_header("Content-Type", content_type)
                self.end_headers()
                self.wfile.write(file.read())
        else:
            self.send_error(404, "File not found")

    def parse_cookies(self):
        """
        Parse cookies from the request headers.
        """
        cookie_header = self.headers.get("Cookie")
        if cookie_header:
            cookies = SimpleCookie(cookie_header)
            self.cookies = {morsel.key: morsel.value for morsel in cookies.values()}
        else:
            self.cookies = {}

    def do_GET(self):
        """
        Handle a GET request.
        """
        if self.path.startswith("/assets/"):
            self.serve_static_file(self.path)
            return

        self.parse_cookies()

        try:
            event = self.create_event("GET", None)
            context = {}  # You can add more mock context properties as needed

            response = handler(event, context)  # Call the imported Lambda handler

            self.send_response(int(response["status"]))
            for header_key, header_value in response["headers"].items():
                for header in header_value:
                    self.send_header(header["key"], header["value"])
            self.end_headers()
            self.wfile.write(response["body"].encode("utf-8"))
        except Exception as e:
            self.send_error(500, str(e))
            print(f"Error handling GET request: {e}")

    def do_POST(self):
        """
        Handle a POST request.
        """
        self.parse_cookies()
        try:
            content_length = int(self.headers["Content-Length"])
            post_data = self.rfile.read(content_length).decode("utf-8")
            event = self.create_event("POST", post_data)
            context = {}  # You can add more mock context properties as needed

            response = handler(event, context)  # Call the imported Lambda handler

            self.send_response(int(response["status"]))
            for header_key, header_value in response["headers"].items():
                for header in header_value:
                    self.send_header(header["key"], header["value"])
            self.end_headers()
            self.wfile.write(response["body"].encode("utf-8"))
        except Exception as e:
            self.send_error(500, str(e))
            print(f"Error handling POST request: {e}")

    def create_event(self, method, body):
        """
        Create an event object for the Lambda function.

        Parameters:
        method (str): The HTTP method of the request.
        body (str): The body of the request.

        Returns:
        dict: The event object.
        """

        headers = {
            key: [{"key": key, "value": value}] for key, value in self.headers.items()
        }
        # Add cookies to headers
        headers["cookie"] = [
            {"key": "cookie", "value": f"{key}={value}"}
            for key, value in self.cookies.items()
        ]

        return {
            "Records": [
                {
                    "cf": {
                        "config": {
                            "distributionDomainName": "d111111abcdef8.cloudfront.net",
                            "distributionId": "EDFDVBD6EXAMPLE",
                            "eventType": "origin-request",
                            "requestId": "4TyzHTaYWb1GX1qTfsHhEqV6HUDd_BzoBZnwfnvQc_1oF26ClkoUSEQ==",
                        },
                        "request": {
                            "clientIp": "203.0.113.178",
                            "headers": headers,
                            "method": method,
                            "origin": {
                                "custom": {
                                    "customHeaders": {},
                                    "domainName": "example.org",
                                    "keepaliveTimeout": 5,
                                    "path": "",
                                    "port": 443,
                                    "protocol": "https",
                                    "readTimeout": 30,
                                    "sslProtocols": ["TLSv1", "TLSv1.1", "TLSv1.2"],
                                }
                            },
                            "querystring": self.path.split("?")[1]
                            if "?" in self.path
                            else "",
                            "uri": self.path.split("?")[0],
                            "body": {
                                "action": "read-only",
                                "data": body,
                                "encoding": "text",
                                "inputTruncated": False,
                            },
                        },
                    }
                }
            ]
        }


def run(server_class=HTTPServer, handler_class=RequestHandler, port=8082):
    """
    Run the HTTP server.

    Parameters:
    server_class (HTTPServer): The class to use for the server.
    handler_class (BaseHTTPRequestHandler): The class to use for handling requests.
    port (int): The port to listen on.

    Returns:
    HTTPServer: The running server.
    """
    server_address = ("", port)
    httpd = server_class(server_address, handler_class)

    # Create an SSL context
    context = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)

    # Define the paths to the .pem files
    keyfile_path = (
        Path.home() / "Documents" / "Developer" / "SANDBOX" / "BHALITE" / "key.pem"
    )
    certfile_path = (
        Path.home() / "Documents" / "Developer" / "SANDBOX" / "BHALITE" / "cert.pem"
    )

    context.load_cert_chain(certfile=str(certfile_path), keyfile=str(keyfile_path))

    # Wrap the server socket with SSL
    httpd.socket = context.wrap_socket(httpd.socket, server_side=True)

    print(f"Starting httpsd server on port {port}...")
    server_thread = threading.Thread(target=httpd.serve_forever)
    server_thread.daemon = True
    server_thread.start()
    return httpd


def file_changed(directory):
    """
    Check if any file in a directory has changed.

    Parameters:
    directory (str): The directory to check.

    Returns:
    bool: True if a file has changed, False otherwise.
    """
    directory_path = Path(directory)
    last_modifications = {
        file: file.stat().st_mtime
        for file in directory_path.rglob("*")
        if file.is_file()
    }

    while True:
        time.sleep(1)  # Check every second
        for file, last_mod in last_modifications.items():
            try:
                current_modification = file.stat().st_mtime
                if current_modification != last_mod:
                    return True
            except FileNotFoundError:
                # Handle the case where a file has been deleted
                return True
        # Update last_modifications to handle new files
        last_modifications = {
            file: file.stat().st_mtime
            for file in directory_path.rglob("*")
            if file.is_file()
        }


if __name__ == "__main__":
    """
    Main entry point of the script.
    """
    print(Path(__file__).parent)
    httpd = run()
    try:
        api_directory = Path(__file__).parent
        print(f"Watching for changes in {api_directory}")
        while True:
            try:
                if file_changed(api_directory):
                    print("File changed...")
                    httpd.shutdown()
                    os.execv(sys.executable, ["python"] + sys.argv)
            except Exception as e:
                print(f"An exception occurred: {e}")
                # Handle the exception or decide to restart the server regardless
                httpd.shutdown()
                os.execv(sys.executable, ["python"] + sys.argv)
    except KeyboardInterrupt:
        pass
    finally:
        httpd.server_close()
