# Start Satellite Signal Processing Dashboard
# Run this PowerShell script to start the web server and open the dashboard

Write-Host ""
Write-Host "==========================================" -ForegroundColor Cyan
Write-Host "Satellite Signal Processing Dashboard" -ForegroundColor Cyan
Write-Host "==========================================" -ForegroundColor Cyan
Write-Host ""

# Set location to script directory
Set-Location (Split-Path -Parent $MyInvocation.MyCommand.Path)

Write-Host "Starting web server on port 8000..." -ForegroundColor Green
Write-Host "Opening dashboard in browser..." -ForegroundColor Green
Write-Host ""

# Start Python HTTP server in background
$job = Start-Job -ScriptBlock { python -m http.server 8000 } -WorkingDirectory (Get-Location)

# Wait for server to start
Start-Sleep -Seconds 2

# Open browser
Start-Process "http://localhost:8000/examples_web_dashboard.html"

Write-Host "==========================================" -ForegroundColor Cyan
Write-Host "Dashboard running at:" -ForegroundColor Cyan
Write-Host "http://localhost:8000/examples_web_dashboard.html" -ForegroundColor Yellow
Write-Host "==========================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "Server is running..." -ForegroundColor Green
Write-Host "Press Ctrl+C to stop" -ForegroundColor Yellow
Write-Host ""

# Keep the job alive
Wait-Job $job