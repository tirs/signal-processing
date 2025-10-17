#ifndef ADAPTIVE_PROCESSOR_H
#define ADAPTIVE_PROCESSOR_H

#include <vector>
#include <complex>
#include <cmath>
#include <algorithm>

using ComplexSample = std::complex<double>;

class AdaptiveProcessor {
public:
    struct SystemState {
        double snr_db;
        double link_margin_db;
        double predicted_snr_db;
        double atmospheric_attenuation_db;
        double rain_attenuation_db;
        double interference_power;
        double signal_power;
    };

    struct AdaptiveConfig {
        double filter_bandwidth_min_hz = 100.0;
        double filter_bandwidth_max_hz = 1e6;
        double sample_rate_min_sps = 1e5;
        double sample_rate_max_sps = 10e6;
        double pll_loop_bandwidth_min_hz = 10.0;
        double pll_loop_bandwidth_max_hz = 10000.0;
    };

    AdaptiveProcessor(const AdaptiveConfig& cfg = AdaptiveConfig()) : config(cfg), state{} {}

    void update_system_state(const SystemState& new_state) {
        state = new_state;
        adjust_parameters();
    }

    struct AdjustmentRecommendations {
        double recommended_filter_bandwidth_hz;
        double recommended_sample_rate_sps;
        double recommended_pll_bandwidth_hz;
        double recommended_antenna_power_w;
        std::string antenna_adjustment_direction;
        bool should_increase_gain;
        bool should_enable_fec;
        std::string status_message;
    };

    AdjustmentRecommendations get_recommended_adjustments() {
        AdjustmentRecommendations rec;
        
        if (state.snr_db < 5.0) {
            rec.recommended_filter_bandwidth_hz = config.filter_bandwidth_min_hz;
            rec.should_increase_gain = true;
            rec.should_enable_fec = true;
            rec.status_message = "CRITICAL: Link quality degraded, enable FEC";
        } 
        else if (state.snr_db < 10.0) {
            rec.recommended_filter_bandwidth_hz = 50000.0;
            rec.should_increase_gain = true;
            rec.should_enable_fec = true;
            rec.status_message = "WARNING: Link margin low";
        }
        else if (state.snr_db > 20.0) {
            rec.recommended_filter_bandwidth_hz = config.filter_bandwidth_max_hz * 0.8;
            rec.should_increase_gain = false;
            rec.should_enable_fec = false;
            rec.status_message = "OPTIMAL: Link conditions excellent";
        }
        else {
            rec.recommended_filter_bandwidth_hz = 500000.0;
            rec.should_increase_gain = false;
            rec.should_enable_fec = false;
            rec.status_message = "NORMAL: Link stable";
        }
        
        rec.recommended_sample_rate_sps = adapt_sample_rate();
        rec.recommended_pll_bandwidth_hz = adapt_pll_bandwidth();
        rec.recommended_antenna_power_w = adapt_transmit_power();
        rec.antenna_adjustment_direction = get_antenna_adjustment();
        
        return rec;
    }

    void predict_link_degradation(double time_horizon_minutes) {
        std::cout << "\nLink Degradation Prediction (next " << time_horizon_minutes << " minutes)\n";
        std::cout << "=======================================================\n";
        
        double rain_probability = 0.3;
        double atmospheric_fade = 0.5;
        
        std::cout << "Current SNR: " << state.snr_db << " dB\n";
        
        double predicted_snr = state.snr_db 
            - (rain_probability * 5.0) 
            - (atmospheric_fade * 2.0);
        
        std::cout << "Predicted SNR: " << predicted_snr << " dB\n";
        
        if (predicted_snr < 5.0) {
            std::cout << "WARNING: Link may become unavailable\n";
            std::cout << "Recommendation: Increase antenna gain or reduce data rate\n";
        } 
        else if (predicted_snr < 10.0) {
            std::cout << "CAUTION: Link quality will degrade\n";
            std::cout << "Recommendation: Prepare FEC parameters\n";
        }
        else {
            std::cout << "OK: Link should remain stable\n";
        }
    }

    void print_adaptive_status() {
        std::cout << "\nAdaptive Signal Processing Status\n";
        std::cout << "===============================================\n";
        std::cout << "SNR: " << state.snr_db << " dB\n";
        std::cout << "Link Margin: " << state.link_margin_db << " dB\n";
        std::cout << "Predicted SNR: " << state.predicted_snr_db << " dB\n";
        std::cout << "Signal Power: " << state.signal_power << " dBm\n";
        std::cout << "Interference Power: " << state.interference_power << " dBm\n";
        std::cout << "Atmospheric Attenuation: " << state.atmospheric_attenuation_db << " dB\n";
        std::cout << "Rain Attenuation: " << state.rain_attenuation_db << " dB\n\n";
    }

private:
    AdaptiveConfig config;
    SystemState state;

    void adjust_parameters() {
    }

    double adapt_sample_rate() {
        if (state.snr_db < 5.0) {
            return 500000.0;
        } 
        else if (state.snr_db < 10.0) {
            return 1000000.0;
        }
        else if (state.snr_db > 20.0) {
            return 5000000.0;
        }
        else {
            return 2000000.0;
        }
    }

    double adapt_pll_bandwidth() {
        if (state.snr_db < 5.0) {
            return config.pll_loop_bandwidth_min_hz;
        } 
        else if (state.snr_db < 10.0) {
            return 100.0;
        }
        else if (state.snr_db > 20.0) {
            return 1000.0;
        }
        else {
            return 500.0;
        }
    }

    double adapt_transmit_power() {
        double base_power = 10.0;
        
        if (state.link_margin_db < 0.0) {
            return base_power * 2.0;
        } 
        else if (state.link_margin_db < 5.0) {
            return base_power * 1.5;
        }
        else {
            return base_power;
        }
    }

    std::string get_antenna_adjustment() {
        if (state.snr_db < 5.0) {
            return "INCREASE_ELEVATION_TO_MAXIMUM";
        } 
        else if (state.snr_db < 10.0) {
            return "FINE_TUNE_ELEVATION";
        }
        else {
            return "MAINTAIN_CURRENT_POINTING";
        }
    }
};

#endif