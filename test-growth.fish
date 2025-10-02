#!/usr/bin/env fish
# Vine Growth Test Script
# This script helps you test the vine growth at different times of day
# WARNING: This temporarily changes your system time!

echo "🌱 Hyprland Vines - Growth Test Script"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""
echo "This script will temporarily change your system time to demonstrate"
echo "vine growth at different hours. Make sure you have sudo privileges."
echo ""
read -P "Continue? (y/n): " -n 1 confirm

if test "$confirm" != "y"
    echo ""
    echo "Cancelled."
    exit 0
end

echo ""
echo ""

# Store original time sync state
set original_ntp_state (timedatectl show --property=NTP --value)

# Disable NTP temporarily
echo "📋 Disabling time synchronization..."
sudo timedatectl set-ntp false

echo ""
echo "🕐 Testing vine growth states..."
echo ""

# Midnight - No vines
echo "1. Midnight (00:00) - Progress: 0% - Just starting"
sudo date -s "00:00:00" > /dev/null
read -P "   Press Enter to see this state..." -n 1
echo ""

sleep 2

# Early morning - Top edge
echo "2. Early Morning (06:00) - Progress: ~35% - Top edge growing"
sudo date -s "06:00:00" > /dev/null
read -P "   Press Enter to see this state..." -n 1
echo ""

sleep 2

# Midday - Right edge
echo "3. Midday (09:00) - Progress: ~53% - Top complete, right growing"
sudo date -s "09:00:00" > /dev/null
read -P "   Press Enter to see this state..." -n 1
echo ""

sleep 2

# Afternoon - Bottom edge
echo "4. Afternoon (12:00) - Progress: ~71% - Bottom edge filling"
sudo date -s "12:00:00" > /dev/null
read -P "   Press Enter to see this state..." -n 1
echo ""

sleep 2

# Late afternoon - Nearly complete
echo "5. Late Afternoon (15:00) - Progress: ~88% - Almost full"
sudo date -s "15:00:00" > /dev/null
read -P "   Press Enter to see this state..." -n 1
echo ""

sleep 2

# Evening - Full coverage, still green
echo "6. Early Evening (16:30) - Progress: ~97% - Full coverage (green)"
sudo date -s "16:30:00" > /dev/null
read -P "   Press Enter to see this state..." -n 1
echo ""

sleep 2

# After 5 PM - Orange transformation
echo "7. Evening (18:00) - Progress: 100% - ORANGE SUNSET! 🍂"
sudo date -s "18:00:00" > /dev/null
read -P "   Press Enter to see this state..." -n 1
echo ""

sleep 2

# Night - Still orange
echo "8. Night (21:00) - Progress: 100% - Maintaining orange"
sudo date -s "21:00:00" > /dev/null
read -P "   Press Enter to see this state..." -n 1
echo ""

echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "✨ Test complete!"
echo ""
echo "🔄 Restoring time synchronization..."

# Restore NTP
if test "$original_ntp_state" = "yes"
    sudo timedatectl set-ntp true
end

# Force time sync
sudo systemctl restart systemd-timesyncd 2>/dev/null || echo "Note: Using ntpd or chrony for time sync"

echo "✅ System time restored"
echo ""
echo "💡 Tip: Watch your window borders change as time progresses naturally!"
