#!/bin/bash

echo "=== Setting up Certbot Auto-Renewal ==="
echo
echo "This script will configure automatic SSL certificate renewal."
echo "Certbot will check twice daily and renew if needed."
echo
echo "To set up auto-renewal, run these commands with sudo:"
echo
echo "1. Create systemd timer for certbot:"
echo "   sudo systemctl enable certbot-renew.timer"
echo "   sudo systemctl start certbot-renew.timer"
echo
echo "2. Verify timer is active:"
echo "   sudo systemctl status certbot-renew.timer"
echo
echo "3. Test renewal (dry run):"
echo "   sudo certbot renew --dry-run"
echo
echo "Alternative: Use cron (if systemd timer doesn't exist):"
echo "   sudo crontab -e"
echo "   Add line: 0 0,12 * * * certbot renew --quiet --post-hook 'systemctl reload nginx'"
echo
echo "Your certificate expires: 2025-10-24"
echo "Auto-renewal will attempt 30 days before expiry."