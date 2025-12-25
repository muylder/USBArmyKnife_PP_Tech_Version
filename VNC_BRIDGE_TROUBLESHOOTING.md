# VNC & Bridge Troubleshooting Guide

## Problem: Agent Not Working

### Root Cause
The T-Dongle is using the **default ESP32-S3 USB VID/PID** (`303A:1001`) instead of the custom one configured in firmware (`CAFE:403F`).

### Solution

**Use the correct launcher script:**
```batch
start_agent.bat
```

This script automatically uses VID `303A` and PID `1001`.

---

## Why the VID/PID Mismatch?

The firmware **configures** custom VID/PID in `platformio.ini` and `USBCore.cpp`:
- Configured: `0xCAFE:0x403F`
- **Actual**: `0x303A:0x1001` (ESP32-S3 default)

**Reason**: The custom VID/PID is only applied when the device is in certain USB modes (HID, Storage). When in **Serial mode** (for Agent communication), it falls back to the ESP32-S3's built-in CDC VID/PID.

---

## Step-by-Step Guide

### 1. Start the Agent
```cmd
cd C:\Users\andre\USBArmyKnife_PP_Tech_Version
start_agent.bat
```

The Agent will:
- Search for a USB Serial device with VID `303A` and PID `1001`
- Connect to COM6
- Wait for commands from the T-Dongle

### 2. Connect to T-Dongle Wi-Fi
- **SSID**: `iPhone14`
- **Password**: `password`

### 3. Access the Web Interface
Open: **http://4.3.2.1:8080**

### 4. Test Bridge Attack
Navigate to: **http://4.3.2.1:8080/runagentcmd?rawCommand=calc.exe**

This should open Calculator on your PC.

### 5. Test VNC
Navigate to: **http://4.3.2.1:8080/vnc/index.html**

Click "Connect" - you should see your PC's screen.

---

## Common Issues

### Issue: "Agent is running" but exits immediately
**Cause**: T-Dongle not connected or wrong VID/PID  
**Fix**: Ensure T-Dongle is plugged in and `start_agent.bat` has correct VID/PID

### Issue: VNC shows black screen
**Cause**: Agent hasn't started capturing yet  
**Fix**: Wait 2-3 seconds after connecting, or refresh the page

### Issue: Bridge commands don't execute
**Cause**: Agent not connected or Serial mode not active  
**Fix**: Check that "agentConnected" shows `true` in the web UI data.json

---

## Advanced: Running Agent as a Service

For persistent operation, use the included `in1.bat`:
```cmd
cd tools\Agent\bin\Debug\net8.0-windows
in1.bat 303A 1001
```

This creates a scheduled task that runs the Agent every minute.
