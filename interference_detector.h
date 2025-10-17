#ifndef INTERFERENCE_DETECTOR_H
#define INTERFERENCE_DETECTOR_H

#include <vector>
#include <complex>
#include <cmath>
#include <algorithm>
#include <numeric>

using ComplexSample = std::complex<double>;

class InterferenceDetector {
public:
    struct DetectionResult {
        bool rfi_detected;
        bool multipath_detected;
        std::string signal_type;
        double interference_power;
        double confidence;
    };

    static DetectionResult detect(const std::vector<ComplexSample>& signal) {
        DetectionResult result;
        
        result.rfi_detected = detect_rfi(signal);
        result.multipath_detected = detect_multipath(signal);
        result.signal_type = classify_signal_type(signal);
        result.interference_power = estimate_interference_power(signal);
        result.confidence = estimate_confidence(signal);
        
        return result;
    }

private:
    static bool detect_rfi(const std::vector<ComplexSample>& signal) {
        if (signal.empty()) return false;
        
        std::vector<double> magnitudes(signal.size());
        for (size_t i = 0; i < signal.size(); ++i) {
            magnitudes[i] = std::abs(signal[i]);
        }
        
        double mean = std::accumulate(magnitudes.begin(), magnitudes.end(), 0.0) / magnitudes.size();
        double variance = 0.0;
        for (double m : magnitudes) {
            variance += (m - mean) * (m - mean);
        }
        variance /= magnitudes.size();
        double std_dev = std::sqrt(variance);
        
        int outliers = 0;
        for (double m : magnitudes) {
            if (std::fabs(m - mean) > 3.0 * std_dev) {
                outliers++;
            }
        }
        
        return outliers > static_cast<int>(magnitudes.size() * 0.05);
    }

    static bool detect_multipath(const std::vector<ComplexSample>& signal) {
        if (signal.size() < 10) return false;
        
        std::vector<double> autocorr(10);
        for (size_t lag = 0; lag < 10; ++lag) {
            double corr = 0.0;
            for (size_t i = 0; i + lag < signal.size(); ++i) {
                corr += std::real(signal[i] * std::conj(signal[i + lag]));
            }
            autocorr[lag] = std::fabs(corr);
        }
        
        double peak = autocorr[0];
        for (size_t i = 1; i < autocorr.size(); ++i) {
            if (autocorr[i] > peak * 0.3) {
                return true;
            }
        }
        
        return false;
    }

    static std::string classify_signal_type(const std::vector<ComplexSample>& signal) {
        if (signal.empty()) return "UNKNOWN";
        
        double iq_power = 0.0;
        for (const auto& s : signal) {
            double i = s.real();
            double q = s.imag();
            iq_power += i * i + q * q;
        }
        iq_power /= signal.size();
        
        double i_power = 0.0;
        for (const auto& s : signal) {
            double i = s.real();
            i_power += i * i;
        }
        i_power /= signal.size();
        
        double q_power = 0.0;
        for (const auto& s : signal) {
            double q = s.imag();
            q_power += q * q;
        }
        q_power /= signal.size();
        
        double iq_imbalance = std::fabs(i_power - q_power) / iq_power;
        
        if (iq_imbalance > 0.3) {
            return "BPSK";
        } else if (iq_imbalance < 0.1) {
            return "QPSK";
        } else {
            return "GENERIC_PSK";
        }
    }

    static double estimate_interference_power(const std::vector<ComplexSample>& signal) {
        if (signal.empty()) return 0.0;
        
        std::vector<double> magnitudes(signal.size());
        for (size_t i = 0; i < signal.size(); ++i) {
            magnitudes[i] = std::abs(signal[i]);
        }
        
        std::sort(magnitudes.begin(), magnitudes.end());
        
        double q3_index = magnitudes.size() * 0.75;
        double q3 = magnitudes[static_cast<size_t>(q3_index)];
        
        double q1_index = magnitudes.size() * 0.25;
        double q1 = magnitudes[static_cast<size_t>(q1_index)];
        
        double iqr = q3 - q1;
        
        int outliers = 0;
        for (double m : magnitudes) {
            if (m > q3 + 1.5 * iqr) {
                outliers++;
            }
        }
        
        return static_cast<double>(outliers) / magnitudes.size();
    }

    static double estimate_confidence(const std::vector<ComplexSample>& signal) {
        if (signal.size() < 100) return 0.5;
        
        double confidence = 0.0;
        
        if (signal.size() >= 1000) confidence += 0.3;
        else if (signal.size() >= 500) confidence += 0.2;
        else confidence += 0.1;
        
        std::vector<double> magnitudes(signal.size());
        for (size_t i = 0; i < signal.size(); ++i) {
            magnitudes[i] = std::abs(signal[i]);
        }
        
        double mean = std::accumulate(magnitudes.begin(), magnitudes.end(), 0.0) / magnitudes.size();
        double variance = 0.0;
        for (double m : magnitudes) {
            variance += (m - mean) * (m - mean);
        }
        variance /= magnitudes.size();
        
        if (variance > 0.01) confidence += 0.2;
        if (variance > 0.05) confidence += 0.2;
        if (variance > 0.1) confidence += 0.2;
        
        return std::min(confidence, 1.0);
    }
};

#endif