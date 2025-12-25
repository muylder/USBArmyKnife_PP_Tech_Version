@echo off
echo ============================================
echo   USB Army Knife Agent Launcher
echo ============================================
echo.
echo Conectando ao T-Dongle...
echo VID: 303A (ESP32-S3)
echo PID: 1001
echo.
echo O Agent vai rodar ate voce pressionar Ctrl+C
echo ============================================
echo.

cd "%~dp0tools\Agent"
dotnet run vid=303A pid=1001

pause
