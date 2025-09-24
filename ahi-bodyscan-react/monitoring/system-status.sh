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
