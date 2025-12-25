# Start Agent for USB Army Knife
# This launches the .NET Agent directly using PowerShell

$ErrorActionPreference = "Stop"

$agentDll = Join-Path $PSScriptRoot "tools\Agent\bin\Debug\net8.0-windows\PortableApp.dll"

Write-Host "Starting USB Army Knife Agent..." -ForegroundColor Cyan
Write-Host "VID: 303A (ESP32-S3)" -ForegroundColor Yellow
Write-Host "PID: 1001" -ForegroundColor Yellow
Write-Host ""
Write-Host "Agent is running. Press Ctrl+C to stop." -ForegroundColor Green
Write-Host ""

# Load the DLL and call the entry point
Add-Type @"
    using System;
    using System.Runtime.InteropServices;

    public class AgentLoader
    {
        [DllImport("$agentDll", SetLastError = true, CharSet = CharSet.Unicode)]
        public static extern void Open(string args);
    }
"@

try {
    [AgentLoader]::Open("vid=303A pid=1001")
    Write-Host "Agent completed successfully." -ForegroundColor Green
}
catch {
    Write-Host "Error running Agent: $_" -ForegroundColor Red
    Write-Host ""
    Write-Host "Troubleshooting:" -ForegroundColor Yellow
    Write-Host "1. Make sure the T-Dongle is connected to COM6" -ForegroundColor White
    Write-Host "2. Check if the Agent DLL was built: $agentDll" -ForegroundColor White
    Write-Host "3. Try rebuilding: cd tools\Agent && dotnet build" -ForegroundColor White
    exit 1
}
