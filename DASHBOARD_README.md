# Satellite Signal Processing Dashboard - Quick Start

## 🚀 Fast Setup (Recommended)

### Option 1: PowerShell (Easiest)
```powershell
Set-Location "c:\Users\simba\Desktop\Signal-Processing"
.\run_dashboard.ps1
```

### Option 2: Command Prompt (Windows)
```bash
cd c:\Users\simba\Desktop\Signal-Processing
run_dashboard.bat
```

### Option 3: Manual Python Start
```bash
cd c:\Users\simba\Desktop\Signal-Processing
python -m http.server 8000
```
Then open: http://localhost:8000/examples_web_dashboard.html

## 📊 What You'll See

✅ **Real-Time Metrics Cards**
- Signal Power, Doppler Shift, SNR
- Satellite position (Az/El/Range)
- System status (CPU, Memory)

✅ **Live Charts** (30-second history)
- Signal Power Over Time (green)
- Doppler Shift Tracking (cyan)  
- SNR Evolution (yellow)

✅ **Performance Benchmarks Table**
- 8 benchmark results
- Color-coded health status
- Statistics: Min/Mean/Max/StdDev
- System health summary

## 🔴 If Benchmarks Don't Show

1. **Run the benchmark suite:**
   ```bash
   ./build/Release/benchmark_suite.exe
   ```
   This creates `benchmark_results.json`

2. **Refresh the dashboard** (press F5)

3. **Check browser console** (F12) for error messages

## 🎨 Dashboard Features

| Feature | Status |
|---------|--------|
| Real-time metric updates | ✓ Every 2 seconds |
| Live signal charts | ✓ Auto-scaling |
| Performance benchmarks | ✓ Color-coded |
| Auto-refresh | ✓ ON (toggle button) |
| Data export | ✓ Via JSON/CSV buttons |

## ⚙️ Text Brightness Fixed

- **Metric labels:** Bright white (#ffffff)
- **Values:** Bright cyan/green (#00ff88, #00ffff)
- **Chart text:** White with bold fonts
- **Benchmark table:** White text on dark background
- **All text:** Clearly visible

## 📝 Troubleshooting

**"benchmark_results.json not found"**
→ Run: `./build/Release/benchmark_suite.exe`

**Charts show but no data**
→ Wait 2 seconds for auto-refresh, then check if live data appears

**Black text on dark background**
→ This has been fixed - refresh your browser (Ctrl+F5)

## 🌐 Browser Support

Works on:
- Chrome/Edge
- Firefox  
- Safari
- Any modern browser with JavaScript enabled

## 🔗 Note on File Access

The dashboard must run on a **local web server** (not file://) to load JSON data securely. The provided scripts handle this automatically.

---

**Need help?** Check the browser console (F12) for detailed error messages.