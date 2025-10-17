#ifndef DOPPLER_COMPENSATOR_H
#define DOPPLER_COMPENSATOR_H

#define _USE_MATH_DEFINES
#include <cmath>
#include <vector>
#include <complex>

namespace SatelliteSignal {

// Real-time Doppler compensation for satellite signals
class DopplerCompensator {
private:
    double current_doppler_shift;
    double target_doppler_shift;
    double phase_accumulator;
    double sample_rate;
    double smoothing_factor;
    double max_rate_of_change;  // Hz/second

public:
    DopplerCompensator(double sample_rate_hz, double smoothing = 0.95)
        : current_doppler_shift(0.0), target_doppler_shift(0.0),
          phase_accumulator(0.0), sample_rate(sample_rate_hz),
          smoothing_factor(smoothing), max_rate_of_change(1000.0) {}

    // Update target Doppler shift and apply smoothing
    void update_doppler(double doppler_hz) {
        double delta = doppler_hz - target_doppler_shift;
        
        // Limit rate of change to prevent abrupt frequency jumps
        if (delta > max_rate_of_change / sample_rate) {
            delta = max_rate_of_change / sample_rate;
        } else if (delta < -max_rate_of_change / sample_rate) {
            delta = -max_rate_of_change / sample_rate;
        }
        
        target_doppler_shift += delta;
        current_doppler_shift = smoothing_factor * current_doppler_shift +
                              (1.0 - smoothing_factor) * target_doppler_shift;
    }

    // Process single sample with Doppler compensation
    std::complex<double> compensate(std::complex<double> sample) {
        double phase_increment = current_doppler_shift * 2.0 * M_PI / sample_rate;
        phase_accumulator += phase_increment;
        
        // Wrap phase to prevent overflow
        while (phase_accumulator >= 2.0 * M_PI) {
            phase_accumulator -= 2.0 * M_PI;
        }
        while (phase_accumulator < 0.0) {
            phase_accumulator += 2.0 * M_PI;
        }
        
        std::complex<double> corrector = std::polar(1.0, phase_accumulator);
        return sample * corrector;
    }

    // Batch process samples
    std::vector<std::complex<double>> compensate_buffer(
        const std::vector<std::complex<double>>& samples) {
        std::vector<std::complex<double>> output(samples.size());
        for (size_t i = 0; i < samples.size(); ++i) {
            output[i] = compensate(samples[i]);
        }
        return output;
    }

    // Get current Doppler frequency shift
    double get_current_doppler() const { return current_doppler_shift; }

    // Get estimated Doppler rate (Hz/s) for next update
    double get_doppler_rate() const {
        return (target_doppler_shift - current_doppler_shift) * 
               sample_rate * smoothing_factor;
    }

    // Reset compensator state
    void reset() {
        current_doppler_shift = 0.0;
        target_doppler_shift = 0.0;
        phase_accumulator = 0.0;
    }

    // Get phase accumulator for debugging
    double get_phase() const { return phase_accumulator; }
};

// Frequency search for initial acquisition
class FrequencySearcher {
public:
    struct SearchResult {
        double best_frequency;
        double best_metric;
        bool found;
    };

    static SearchResult search_frequency_range(
        const std::vector<std::complex<double>>& samples,
        double center_freq, double search_width, double step_size,
        double sample_rate) {
        
        SearchResult result = {center_freq, 0.0, false};
        
        double start_freq = center_freq - search_width / 2.0;
        double end_freq = center_freq + search_width / 2.0;
        
        for (double freq = start_freq; freq <= end_freq; freq += step_size) {
            double correlation = compute_correlation(samples, freq, sample_rate);
            
            if (correlation > result.best_metric) {
                result.best_metric = correlation;
                result.best_frequency = freq;
                result.found = true;
            }
        }
        
        return result;
    }

private:
    static double compute_correlation(
        const std::vector<std::complex<double>>& samples,
        double frequency, double sample_rate) {
        
        double phase = 0.0;
        double phase_increment = frequency * 2.0 * M_PI / sample_rate;
        double real_sum = 0.0;
        double imag_sum = 0.0;
        
        for (const auto& sample : samples) {
            std::complex<double> local_oscillator = 
                std::polar(1.0, phase);
            std::complex<double> correlated = sample * local_oscillator;
            
            real_sum += correlated.real();
            imag_sum += correlated.imag();
            phase += phase_increment;
        }
        
        return sqrt(real_sum*real_sum + imag_sum*imag_sum) / samples.size();
    }
};

}  // namespace SatelliteSignal

#endif // DOPPLER_COMPENSATOR_H