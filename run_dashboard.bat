@echo off
REM Start Satellite Signal Processing Dashboard
REM This script starts a local web server and opens the dashboard in your browser

echo.
echo ==========================================
echo Satellite Signal Processing Dashboard
echo ==========================================
echo.
echo Starting web server on port 8000...
echo.
echo Opening dashboard in browser...
echo.

REM Change to script directory
cd /d "%~dp0"

REM Start Python HTTP server in background
python -m http.server 8000 >nul 2>&1 &

REM Small delay for server to start
timeout /t 2 /nobreak >nul

REM Open browser
start http://localhost:8000/examples_web_dashboard.html

REM Keep command window open
timeout /t 5 /nobreak

echo.
echo ==========================================
echo Dashboard running at:
echo http://localhost:8000/examples_web_dashboard.html
echo ==========================================
echo.
echo Press Ctrl+C in the command window to stop the server
echo.

REM Start the server in foreground
python -m http.server 8000