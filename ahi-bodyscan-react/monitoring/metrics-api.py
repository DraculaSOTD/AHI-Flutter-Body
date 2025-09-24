#!/usr/bin/env python3

import json
import psutil
import time
from http.server import HTTPServer, BaseHTTPRequestHandler

class MetricsHandler(BaseHTTPRequestHandler):
    def do_GET(self):
        if self.path == '/metrics':
            # Gather system metrics
            metrics = {
                'cpu_percent': psutil.cpu_percent(interval=1),
                'memory': {
                    'percent': psutil.virtual_memory().percent,
                    'used_gb': round(psutil.virtual_memory().used / (1024**3), 2),
                    'total_gb': round(psutil.virtual_memory().total / (1024**3), 2)
                },
                'disk': {
                    'percent': psutil.disk_usage('/').percent,
                    'used_gb': round(psutil.disk_usage('/').used / (1024**3), 2),
                    'total_gb': round(psutil.disk_usage('/').total / (1024**3), 2)
                },
                'network_connections': len(psutil.net_connections()),
                'uptime_seconds': int(time.time() - psutil.boot_time()),
                'timestamp': int(time.time())
            }
            
            self.send_response(200)
            self.send_header('Content-Type', 'application/json')
            self.send_header('Access-Control-Allow-Origin', '*')
            self.end_headers()
            self.wfile.write(json.dumps(metrics).encode())
        else:
            self.send_response(404)
            self.end_headers()
    
    def log_message(self, format, *args):
        return  # Suppress logs

if __name__ == '__main__':
    server = HTTPServer(('127.0.0.1', 8888), MetricsHandler)
    print('Metrics API running on http://127.0.0.1:8888/metrics')
    server.serve_forever()
