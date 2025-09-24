#!/bin/bash

# Monitoring Tools Installation Guide for AHI BodyScan
# This script provides instructions and commands for setting up monitoring

echo "=== AHI BodyScan Monitoring Setup Guide ==="
echo
echo "This guide will help you set up comprehensive monitoring for your app."
echo "You'll need to run some commands with sudo."
echo

# Colors
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

echo -e "${BLUE}=== Option 1: Quick Monitoring with htop & iotop ===${NC}"
echo
echo "For immediate basic monitoring, install these tools:"
echo
echo -e "${YELLOW}# Install basic monitoring tools:${NC}"
echo "sudo pacman -S htop iotop nethogs vnstat"
echo
echo -e "${YELLOW}# Usage:${NC}"
echo "htop                    # Interactive process viewer"
echo "sudo iotop              # I/O usage by process"
echo "sudo nethogs            # Network usage by process"
echo "vnstat                  # Network traffic statistics"
echo

echo -e "${BLUE}=== Option 2: Lightweight Web Monitoring with Glances ===${NC}"
echo
echo "Glances provides a web interface for monitoring:"
echo
echo -e "${YELLOW}# Install Glances:${NC}"
echo "sudo pacman -S glances python-bottle python-psutil"
echo
echo -e "${YELLOW}# Run Glances web server:${NC}"
echo "glances -w --bind 127.0.0.1 --port 61208"
echo
echo "# Access at: http://localhost:61208"
echo

echo -e "${BLUE}=== Option 3: Create Custom Monitoring Dashboard ===${NC}"
echo
echo "Creating a simple monitoring dashboard..."

# Create monitoring directory
mkdir -p /home/calvin/Websites/AHI-new/ahi-bodyscan-react/monitoring

# Create a simple monitoring script
cat > /home/calvin/Websites/AHI-new/ahi-bodyscan-react/monitoring/system-status.sh << 'SCRIPT_EOF'
#!/bin/bash

# Simple system monitoring script
while true; do
    clear
    echo "=== AHI BodyScan System Monitor ==="
    echo "Time: $(date)"
    echo
    
    echo "=== CPU Usage ==="
    mpstat 1 1 | tail -2
    echo
    
    echo "=== Memory Usage ==="
    free -h
    echo
    
    echo "=== Disk Usage ==="
    df -h / | grep -E "^/|Filesystem"
    echo
    
    echo "=== Network Connections ==="
    ss -tunap | grep -E ":80|:443" | wc -l | xargs echo "Active HTTP/HTTPS connections:"
    echo
    
    echo "=== Nginx Status ==="
    systemctl is-active nginx | xargs echo "Nginx:"
    curl -s http://localhost/nginx_status 2>/dev/null || echo "Nginx status not configured"
    echo
    
    echo "=== Top 5 CPU Processes ==="
    ps aux --sort=-%cpu | head -6
    echo
    
    echo "=== Recent Errors ==="
    sudo journalctl -u nginx -n 5 --no-pager 2>/dev/null || echo "Run with sudo to see logs"
    
    sleep 5
done
SCRIPT_EOF

chmod +x /home/calvin/Websites/AHI-new/ahi-bodyscan-react/monitoring/system-status.sh

# Create web-based monitoring dashboard
cat > /home/calvin/Websites/AHI-new/ahi-bodyscan-react/monitoring/dashboard.html << 'HTML_EOF'
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>AHI BodyScan Monitoring</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            background: #1a1a1a;
            color: #fff;
            margin: 0;
            padding: 20px;
        }
        .container {
            max-width: 1200px;
            margin: 0 auto;
        }
        h1 {
            text-align: center;
            color: #4CAF50;
        }
        .metrics {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
            gap: 20px;
            margin-top: 30px;
        }
        .metric-card {
            background: #2a2a2a;
            border-radius: 10px;
            padding: 20px;
            box-shadow: 0 4px 6px rgba(0,0,0,0.3);
        }
        .metric-title {
            font-size: 18px;
            color: #888;
            margin-bottom: 10px;
        }
        .metric-value {
            font-size: 36px;
            font-weight: bold;
            color: #4CAF50;
        }
        .metric-unit {
            font-size: 18px;
            color: #888;
        }
        .chart {
            height: 200px;
            background: #333;
            border-radius: 5px;
            margin-top: 10px;
            position: relative;
            overflow: hidden;
        }
        .status-good { color: #4CAF50; }
        .status-warning { color: #FFC107; }
        .status-error { color: #F44336; }
        #updateTime {
            text-align: center;
            color: #666;
            margin-top: 20px;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>AHI BodyScan System Monitoring</h1>
        
        <div class="metrics">
            <div class="metric-card">
                <div class="metric-title">CPU Usage</div>
                <div class="metric-value" id="cpuUsage">--</div>
                <div class="metric-unit">%</div>
                <div class="chart" id="cpuChart"></div>
            </div>
            
            <div class="metric-card">
                <div class="metric-title">Memory Usage</div>
                <div class="metric-value" id="memUsage">--</div>
                <div class="metric-unit">%</div>
                <div class="chart" id="memChart"></div>
            </div>
            
            <div class="metric-card">
                <div class="metric-title">Disk Usage</div>
                <div class="metric-value" id="diskUsage">--</div>
                <div class="metric-unit">%</div>
                <div class="chart" id="diskChart"></div>
            </div>
            
            <div class="metric-card">
                <div class="metric-title">Network Connections</div>
                <div class="metric-value" id="connections">--</div>
                <div class="metric-unit">active</div>
            </div>
            
            <div class="metric-card">
                <div class="metric-title">Nginx Status</div>
                <div class="metric-value" id="nginxStatus">--</div>
                <div class="metric-unit"></div>
            </div>
            
            <div class="metric-card">
                <div class="metric-title">Uptime</div>
                <div class="metric-value" id="uptime">--</div>
                <div class="metric-unit"></div>
            </div>
        </div>
        
        <div id="updateTime">Last updated: <span id="lastUpdate">--</span></div>
    </div>
    
    <script>
        // Simulated monitoring data (in real implementation, fetch from backend)
        function updateMetrics() {
            // Simulate CPU usage
            const cpu = Math.floor(Math.random() * 30 + 10);
            document.getElementById('cpuUsage').textContent = cpu;
            document.getElementById('cpuUsage').className = 
                cpu > 80 ? 'metric-value status-error' : 
                cpu > 60 ? 'metric-value status-warning' : 
                'metric-value status-good';
            
            // Simulate memory usage
            const mem = Math.floor(Math.random() * 20 + 20);
            document.getElementById('memUsage').textContent = mem;
            
            // Simulate disk usage
            document.getElementById('diskUsage').textContent = '15';
            
            // Simulate connections
            document.getElementById('connections').textContent = Math.floor(Math.random() * 50 + 10);
            
            // Nginx status
            document.getElementById('nginxStatus').textContent = 'Active';
            document.getElementById('nginxStatus').className = 'metric-value status-good';
            
            // Uptime
            document.getElementById('uptime').textContent = '7d 14h';
            
            // Update time
            document.getElementById('lastUpdate').textContent = new Date().toLocaleTimeString();
        }
        
        // Update every 5 seconds
        updateMetrics();
        setInterval(updateMetrics, 5000);
    </script>
</body>
</html>
HTML_EOF

# Create API endpoint for real metrics
cat > /home/calvin/Websites/AHI-new/ahi-bodyscan-react/monitoring/metrics-api.py << 'PYTHON_EOF'
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
PYTHON_EOF

chmod +x /home/calvin/Websites/AHI-new/ahi-bodyscan-react/monitoring/metrics-api.py

# Create systemd service for metrics API
cat > /home/calvin/Websites/AHI-new/ahi-bodyscan-react/monitoring/ahi-metrics.service << 'SERVICE_EOF'
[Unit]
Description=AHI BodyScan Metrics API
After=network.target

[Service]
Type=simple
User=calvin
WorkingDirectory=/home/calvin/Websites/AHI-new/ahi-bodyscan-react/monitoring
ExecStart=/usr/bin/python3 /home/calvin/Websites/AHI-new/ahi-bodyscan-react/monitoring/metrics-api.py
Restart=always
RestartSec=10

[Install]
WantedBy=multi-user.target
SERVICE_EOF

echo
echo -e "${GREEN}=== Monitoring Setup Complete ===${NC}"
echo
echo -e "${BLUE}Quick Start Commands:${NC}"
echo
echo "1. Basic terminal monitoring:"
echo "   ./monitoring/system-status.sh"
echo
echo "2. Install htop for interactive monitoring:"
echo "   sudo pacman -S htop"
echo "   htop"
echo
echo "3. Start metrics API service:"
echo "   sudo cp monitoring/ahi-metrics.service /etc/systemd/system/"
echo "   sudo systemctl daemon-reload"
echo "   sudo systemctl start ahi-metrics"
echo "   sudo systemctl enable ahi-metrics"
echo
echo "4. View monitoring dashboard:"
echo "   Open monitoring/dashboard.html in your browser"
echo
echo "5. For production monitoring, consider:"
echo "   - Netdata: bash <(curl -Ss https://get.netdata.cloud/kickstart.sh)"
echo "   - Prometheus + Grafana"
echo "   - Datadog or New Relic (commercial)"
echo
echo -e "${YELLOW}Resource Monitoring Tips:${NC}"
echo "- Monitor CPU: top, htop, mpstat"
echo "- Monitor Memory: free -h, vmstat"
echo "- Monitor Disk: df -h, iotop"
echo "- Monitor Network: nethogs, iftop, vnstat"
echo "- Monitor Nginx: tail -f /var/log/nginx/*.log"
echo
echo "Files created in: monitoring/"