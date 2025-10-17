#ifndef SIGNAL_PROCESSOR_H
#define SIGNAL_PROCESSOR_H

#define _USE_MATH_DEFINES
#include <vector>
#include <complex>
#include <cmath>
#include <algorithm>
#include <array>

namespace SatelliteSignal {

constexpr double PI = 3.14159265358979323846;
constexpr double SPEED_OF_LIGHT = 299792458.0;

// Complex sample type for IQ data
using ComplexSample = std::complex<double>;

// Filter coefficient storage
struct FilterCoefficients {
    std::vector<double> numerator;
    std::vector<double> denominator;
};

// Signal statistics
struct SignalMetrics {
    double snr_db;
    double signal_power;
    double noise_power;
    double peak_amplitude;
    double mean_amplitude;
    bool is_locked;
};

// FIR Filter for baseband signal processing
class FIRFilter {
private:
    std::vector<double> coefficients;
    std::vector<double> delay_line;
    size_t delay_index;

public:
    FIRFilter(const std::vector<double>& taps) 
        : coefficients(taps), delay_line(taps.size(), 0.0), delay_index(0) {}

    double process(double sample) {
        delay_line[delay_index] = sample;
        double output = 0.0;
        
        for (size_t i = 0; i < coefficients.size(); ++i) {
            size_t idx = (delay_index + i) % coefficients.size();
            output += coefficients[i] * delay_line[idx];
        }
        
        delay_index = (delay_index + coefficients.size() - 1) % coefficients.size();
        return output;
    }

    std::vector<double> process_buffer(const std::vector<double>& input) {
        std::vector<double> output(input.size());
        for (size_t i = 0; i < input.size(); ++i) {
            output[i] = process(input[i]);
        }
        return output;
    }
};

// Lowpass filter using rectangular window sinc
class LowpassFilter : public FIRFilter {
public:
    LowpassFilter(double cutoff_freq, size_t num_taps)
        : FIRFilter(generate_sinc_taps(cutoff_freq, num_taps)) {}

private:
    static std::vector<double> generate_sinc_taps(double cutoff, size_t num_taps) {
        std::vector<double> taps(num_taps);
        int center = num_taps / 2;
        double sum = 0.0;
        
        for (int i = 0; i < (int)num_taps; ++i) {
            int n = i - center;
            if (n == 0) {
                taps[i] = 2.0 * cutoff;
            } else {
                taps[i] = sin(2.0 * PI * cutoff * n) / (PI * n);
                taps[i] *= 0.54 - 0.46 * cos(2.0 * PI * i / (num_taps - 1));
            }
            sum += taps[i];
        }
        
        for (auto& tap : taps) tap /= sum;
        return taps;
    }
};

// Phase-Locked Loop for carrier tracking
class PhaseLockedLoop {
private:
    double frequency;
    double phase;
    double kp;  // Proportional gain
    double ki;  // Integral gain
    double integral_error;

public:
    PhaseLockedLoop(double initial_freq, double kp_val = 0.1, double ki_val = 0.01)
        : frequency(initial_freq), phase(0.0), kp(kp_val), ki(ki_val), integral_error(0.0) {}

    ComplexSample process(ComplexSample input) {
        ComplexSample carrier = std::polar(1.0, phase);
        ComplexSample output = input * std::conj(carrier);
        
        double phase_error = std::arg(output);
        integral_error += phase_error;
        double freq_correction = kp * phase_error + ki * integral_error;
        
        frequency += freq_correction * 0.01;
        phase += frequency * 2.0 * PI / 1000000.0;  // Assuming 1 MHz sample rate
        
        return output;
    }

    double get_frequency() const { return frequency; }
    double get_phase() const { return phase; }
};

// QPSK Demodulator
class QPSKDemodulator {
private:
    FIRFilter matched_filter;
    PhaseLockedLoop pll;
    std::vector<int> decisions;

public:
    QPSKDemodulator(double carrier_freq)
        : matched_filter(generate_matched_filter()),
          pll(carrier_freq) {}

    std::vector<int> demodulate(const std::vector<ComplexSample>& symbols) {
        decisions.clear();
        decisions.reserve(symbols.size());
        
        for (const auto& symbol : symbols) {
            ComplexSample tracked = pll.process(symbol);
            
            double i = tracked.real();
            double q = tracked.imag();
            
            int bits = 0;
            if (i > 0) bits |= 0x01;
            if (q > 0) bits |= 0x02;
            
            decisions.push_back(bits);
        }
        
        return decisions;
    }

    double get_carrier_frequency() const { return pll.get_frequency(); }

private:
    static std::vector<double> generate_matched_filter() {
        return std::vector<double>(32, 1.0 / 32.0);
    }
};

// Signal Analysis and Metrics
class SignalAnalyzer {
public:
    static SignalMetrics analyze(const std::vector<ComplexSample>& signal) {
        SignalMetrics metrics;
        
        double signal_power = 0.0;
        double peak = 0.0;
        double mean = 0.0;
        
        for (const auto& sample : signal) {
            double magnitude = std::abs(sample);
            signal_power += magnitude * magnitude;
            peak = std::max(peak, magnitude);
            mean += magnitude;
        }
        
        signal_power /= signal.size();
        mean /= signal.size();
        
        double noise_power = calculate_noise_floor(signal);
        metrics.snr_db = 10.0 * std::log10(signal_power / (noise_power + 1e-12));
        metrics.signal_power = signal_power;
        metrics.noise_power = noise_power;
        metrics.peak_amplitude = peak;
        metrics.mean_amplitude = mean;
        metrics.is_locked = metrics.snr_db > 3.0;
        
        return metrics;
    }

private:
    static double calculate_noise_floor(const std::vector<ComplexSample>& signal) {
        if (signal.size() < 2) return 0.0;
        
        std::vector<double> magnitudes(signal.size());
        std::transform(signal.begin(), signal.end(), magnitudes.begin(),
                      [](const ComplexSample& s) { return std::abs(s); });
        
        std::sort(magnitudes.begin(), magnitudes.end());
        
        double noise = 0.0;
        size_t noise_samples = signal.size() / 10;
        for (size_t i = 0; i < noise_samples; ++i) {
            noise += magnitudes[i] * magnitudes[i];
        }
        
        return noise / noise_samples;
    }
};

}  // namespace SatelliteSignal

#endif // SIGNAL_PROCESSOR_H