@echo off
REM Quick launcher for the USB Army Knife Agent
REM This script runs the Agent to enable VNC and Bridge attacks

set AGENT_PATH=%~dp0tools\Agent\bin\Debug\net8.0-windows\PortableApp.dll

echo Starting USB Army Knife Agent...
echo VID: 303A (ESP32-S3)
echo PID: 1001
echo.
echo Agent is running. Press Ctrl+C to stop.
echo.

rundll32 "%AGENT_PATH%" Open32 vid=303A pid=1001
