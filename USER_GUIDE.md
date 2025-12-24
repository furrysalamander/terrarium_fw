# Terrarium Lighting Controller

---

## Table of Contents
1. [Quick Start](#quick-start)
2. [What's in the Box](#whats-in-the-box)
3. [Initial Setup](#initial-setup)
4. [Dashboard Features](#dashboard-features)
5. [Advanced Configuration](#advanced-configuration)
6. [Understanding Status Indicators](#understanding-status-indicators)
7. [Troubleshooting](#troubleshooting)

---

## Quick Start

**First time? Follow these 4 steps:**

1. **Power**: Connect 12V power supply → controller powers on
2. **Connect**: Join "Terrarium-Setup" Wi-Fi (password: `terra1234`)
3. **Configure**: Browse to `192.168.4.1` → enter your home Wi-Fi credentials
4. **Access**: Visit `http://terrarium.local` from your home network

**Already set up?** Just visit `http://terrarium.local` from any device on your network.

---

### What's in the Box
- **Terrarium**

### What You'll Need
- **Wi-Fi Network** (2.4GHz, WPA/WPA2 security supported)
- **Device with Browser** (phone, tablet, or computer)
- **12V Power Supply** (barrel jack adapter)

---

## Initial Setup

### Step 1: Physical Setup
1. **Position the controller** on or near your terrarium where the lights can illuminate your plants
2. **Plug in the 12V power supply** to the controller
3. **Verify power** - the status LED should briefly flash, then either:
   - Turn **solid** (if Wi-Fi is already configured)
   - Start **slow breathing** (setup mode is active)

### Step 2: Wi-Fi Configuration (First Time Only)

**When you see the breathing LED:**

1. **Join Setup Network**
   - Network name: `Terrarium-Setup`
   - Password: `terra1234`
   - Connect from any Wi-Fi device

2. **Open Configuration Page**
   - Go to: `http://192.168.4.1`
   - The page should load automatically (captive portal)

3. **Enter Your Wi-Fi Details**
   - **Network name (SSID)**: Your home Wi-Fi name
   - **Password**: Your home Wi-Fi password
   - Click **Save & Reboot**

4. **Wait for Connection**
   - Controller restarts and joins your home network
   - Breathing LED stops, status LED goes solid
   - Setup network disappears

**⚠️ Important:** The controller only supports 2.4GHz Wi-Fi networks. If you have a dual-band router, make sure you're connecting to the 2.4GHz network or a network that supports both bands.

### Step 3: Access the Dashboard

1. **Ensure Same Network**: Your device must be on the same Wi-Fi network
2. **Open Browser**: Go to `http://terrarium.local`
3. **Bookmark for Later**: Save this address for quick access

**Can't reach terrarium.local?** See [Finding Your Controller's IP Address](#finding-your-controllers-ip-address) below.

---

## Dashboard Features

### Brightness Control

**Immediate Control Options:**
- **Slider**: Drag for quick adjustments (0-100%)
- **Numeric Input**: Type exact percentage for precision
- **Real-time Updates**: Light responds instantly to changes

**Key Features:**
- Settings persist through power outages
- Smooth dimming for gentle low-light control
- Override capability - manual changes pause active schedules

**⚠️ Important:** For optimal LED lifespan, brightness above 50% is not recommended for extended periods.

### Automated Scheduling

**Enable Smart Automation:**
1. Check **"Enable automated schedule"**
2. Set **On Time** (when lights turn on)
3. Set **Off Time** (when lights turn off)
4. Click **Save Schedule**

**Schedule Behavior:**
- Uses 24-hour format (e.g., 07:30, 19:45)
- Automatically handles day transitions (e.g., on at 22:00, off at 06:00)
- Manual brightness changes temporarily override schedule
- Status shows "Schedule Active" or "Outside Schedule Window"

**Smart Features:**
- Timezone automatic detection from your browser
- Daylight Saving Time support
- Schedule persists through power outages
- Requires internet time sync to function

### Device Status Monitor

**Connection Information:**
- **IP Address**: Controller's network address
- **Network Status**: Wi-Fi connection state
- **Local Time**: Controller's current time display

**System Health:**
- **Clock Synced**: Shows if time has been synchronized from internet
- **Schedule Status**: Whether automation is currently active
- **Light Output**: Current brightness percentage

---

## Advanced Configuration

### Timezone Management

The controller automatically detects your timezone when you load the dashboard, but you can verify accuracy:

1. **Check "Local Time"** displayed on dashboard
2. **If incorrect**: Refresh the page to resend timezone data
3. **Persistent Issues**: Controller needs internet access for time sync

### Network Management

**Changing Wi-Fi Networks:**
- Use **"Launch Wi-Fi Setup"** button on dashboard
- Or unplug power for 10 seconds, then plug back in

**Network Troubleshooting:**
- Controller requires 2.4GHz Wi-Fi
- WPA/WPA2 personal security supported
- Enterprise networks not supported
- Hidden networks supported (enter exact SSID)

### Finding Your Controller's IP Address

**If terrarium.local doesn't work:**

1. **Check Router Admin Panel**
   - Look for "terrarium" in connected devices
   - Note the assigned IP address (e.g., 192.168.1.42)
   - Browse directly to that IP

2. **Network Scanner Apps**
   - Use apps like "Fing" or "WiFi Analyzer"
   - Look for "terrarium" in the device list

3. **Force Setup Mode**
   - Unplug power for 10 seconds, then plug back in
   - If Wi-Fi fails to connect, setup network will appear automatically
   - Rejoin setup network and reconfigure

---

## Understanding Status Indicators

### Status LED Patterns

| Pattern | Meaning | Action Needed |
|---------|---------|---------------|
| **Solid ON** | Connected to Wi-Fi, operating normally | None - system ready |
| **Solid OFF** | Light is off (manual or schedule) | Normal operation |
| **Slow Breathing** | Setup mode active | Join Terrarium-Setup network |

### Dashboard Status Messages

**Connection States:**
- ✅ **"Connected"** - All systems operational
- ⚠️ **"Connecting..."** - Attempting Wi-Fi connection
- ❌ **"Disconnected"** - No network connection

**Clock Sync States:**
- ✅ **"Yes"** - Time synchronized, schedules active
- ❌ **"No"** - No time sync, schedules paused
- ⏳ **"Syncing..."** - Time synchronization in progress

---

## Troubleshooting

### Common Issues

**Dashboard Won't Load**
- ✅ Confirm controller has power (status LED on)
- ✅ Verify you're on the same Wi-Fi network
- ✅ Try the direct IP address instead of terrarium.local
- ✅ Clear browser cache and refresh

**Setup Network Not Appearing**
- ✅ Unplug power for 10 seconds, then plug back in
- ✅ Wait 30 seconds for network to appear
- ✅ Check that no other devices are using same hotspot name

**Light Not Responding**
- ✅ Verify 12V power supply is connected and working
- ✅ Check that brightness isn't set to 0%
- ✅ Try power cycling the controller
- ✅ Check that the status LED is on (confirms controller has power)

**Schedule Not Working**
- ✅ Check that "Clock synced" shows "Yes"
- ✅ Verify internet connection is available
- ✅ Confirm schedule times are set correctly
- ✅ Check that "Enable automated schedule" is checked

**Timezone Wrong**
- ✅ Refresh dashboard page to resend timezone
- ✅ Check internet connection for time sync
- ✅ Verify computer/phone has correct time

---

*All settings are stored on the controller and will be remembered after power outages.*
