# Quick Start Guide - Satellite Signal Processing System

## Build Instructions

### Windows (Visual Studio 2022)
```powershell
cd c:\Users\simba\Desktop\Signal-Processing
cmake -S . -B build -G "Visual Studio 17 2022"
cmake --build build --config Release
.\build\Release\satellite_signal_demo.exe
```

### Windows (MinGW)
```powershell
cmake -S . -B build -G "MinGW Makefiles"
cmake --build build
.\build\satellite_signal_demo.exe
```

### Linux/macOS
```bash
cd ~/Signal-Processing
cmake -S . -B build
cmake --build build
./build/satellite_signal_demo
```

## Project Files

| File | Purpose |
|------|---------|
| `signal_processor.h` | DSP algorithms (FIR, PLL, QPSK demodulation) |
| `satellite_ephemeris.h` | Orbital mechanics and Doppler calculation |
| `doppler_compensator.h` | Real-time frequency correction |
| `main.cpp` | Demonstration program |
| `SATELLITE_SIGNAL_PROCESSING_GUIDE.txt` | Comprehensive technical documentation |
| `README.md` | Architecture and integration guide |

## What the Demo Shows

### 1. Ephemeris Calculation
Predicts ISS satellite position, visibility, and Doppler shift at 10 GHz over Boulder, Colorado. Shows how orbital mechanics feed into frequency correction needs.

**Output:**
- Time, Azimuth, Elevation, Range
- Doppler frequency shift for accurate tuning

### 2. Signal Processing Pipeline
Processes synthetic QPSK signal with realistic noise and Doppler:
- **Input**: Signal with 8 dB SNR and 5000 Hz Doppler shift
- **Processing**: Doppler compensation → FIR filtering → Demodulation
- **Output**: Recovered bits with signal quality metrics

**Key Metrics:**
- SNR before/after processing
- Lock status (LOCKED/UNLOCKED)
- Signal power and amplitude statistics

### 3. Frequency Search
Blind frequency acquisition across ±5 kHz search range using correlation-based detection.

## Using the Code in Your Project

### Basic Signal Processing
```cpp
#include "signal_processor.h"
using namespace SatelliteSignal;

// Create a lowpass filter
LowpassFilter filter(0.1, 64);  // Cutoff freq, tap count

// Process samples
std::vector<double> signal = get_signal_samples();
auto filtered = filter.process_buffer(signal);
```

### Satellite Tracking
```cpp
#include "satellite_ephemeris.h"

// Load TLE and create ephemeris propagator
TLE tle = load_tle_from_file("iss.tle");
SatelliteEphemeris ephemeris(tle);

// Get current satellite position
StateVector pos = ephemeris.get_position(minutes_from_epoch);

// Calculate view from ground station
GroundStation station = {40.0, -105.0, 1600.0};
TopocentricView view = ephemeris.get_topocentric(pos, station);

// Calculate required frequency correction
double doppler = ephemeris.calculate_doppler_shift(view, 10e9);
```

### Doppler Compensation
```cpp
#include "doppler_compensator.h"

// Create compensator at 1 MHz sample rate
DopplerCompensator compensator(1e6);

// Update Doppler estimate and compensate signal
compensator.update_doppler(calculated_doppler_hz);
auto corrected = compensator.compensate_buffer(raw_signal);
```

## System Requirements

**Compiler:** C++17 or later
- MSVC 2019+
- GCC 8.0+
- Clang 7.0+

**Dependencies:** None (uses only standard library)

**Memory:** <10 MB for typical buffers

**Performance:**
- Real-time at 1 Msps on modern CPU
- Scalable to 100+ Msps with optimization

## Configuration Parameters

Edit header files or main.cpp to change:

| Parameter | Location | Effect |
|-----------|----------|--------|
| Sample rate | `main.cpp` | Sets timing for all algorithms |
| Filter tap count | `main.cpp` | Trades latency vs. attenuation |
| PLL gains | `signal_processor.h` | Controls lock convergence |
| Doppler search range | `main.cpp` | Initial acquisition uncertainty |
| TLE orbital elements | `main.cpp` | Satellite identity |
| Ground station position | `main.cpp` | View angle calculations |

## Next Steps

1. **Integrate with RF Frontend**: Connect to SDR (e.g., USRP, BladeRF)
2. **Add FEC Decoding**: Implement Viterbi or Turbo decoders
3. **Multi-Channel**: Process multiple satellites in parallel
4. **Database Integration**: Store telemetry and housekeeping data
5. **Real-time Scheduling**: Use deterministic OS for predictable timing

## Troubleshooting

**Build Fails on M_PI**
- Ensure `#define _USE_MATH_DEFINES` is before `#include <cmath>`

**Signal Not Locking**
- Reduce Doppler error (improve frequency estimate)
- Increase PLL loop bandwidth
- Check SNR is above 3 dB

**Poor Frequency Estimate**
- Increase search buffer size
- Reduce step size in frequency search
- Verify TLE is recent (< 7 days old)

## Performance Tips

1. **Use Release builds** (-O2 or -O3 optimization)
2. **Pre-allocate buffers** to avoid allocation overhead
3. **Batch process** for better cache utilization
4. **Profile with**: `perf record`, `Intel VTune`, or Visual Studio Profiler

## Documentation

- **SATELLITE_SIGNAL_PROCESSING_GUIDE.txt**: Comprehensive technical overview
- **README.md**: Architecture details and algorithm specifics
- **Code comments**: Implementation details in header files

## Example: Complete Signal Reception Chain

```cpp
#include "signal_processor.h"
#include "satellite_ephemeris.h"
#include "doppler_compensator.h"

// 1. Initialize satellite ephemeris
TLE tle = {/*...*/};
SatelliteEphemeris ephemeris(tle);
GroundStation station = {40.0, -105.0, 1600.0};

// 2. Setup signal processing
DopplerCompensator doppler_comp(1e6);
LowpassFilter filter(0.1, 64);
QPSKDemodulator demod(0.0);

// Main receive loop
while (satellite_in_view) {
    // Update satellite position and Doppler
    StateVector pos = ephemeris.get_position(current_time);
    TopocentricView view = ephemeris.get_topocentric(pos, station);
    double doppler = ephemeris.calculate_doppler_shift(view, 10e9);
    
    // Steer antenna
    steer_antenna(view.azimuth, view.elevation);
    set_receiver_frequency(carrier_freq + doppler);
    
    // Receive and process
    auto raw_samples = receiver.get_samples(BUFFER_SIZE);
    doppler_comp.update_doppler(doppler);
    auto compensated = doppler_comp.compensate_buffer(raw_samples);
    auto filtered = filter.process_buffer(compensated);
    auto bits = demod.demodulate(filtered);
    
    // Decode and store telemetry
    process_telemetry(bits);
}
```
