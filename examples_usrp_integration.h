#ifndef EXAMPLES_USRP_INTEGRATION_H
#define EXAMPLES_USRP_INTEGRATION_H

#include <iostream>
#include <vector>
#include <complex>

using ComplexSample = std::complex<double>;

class USRPIntegration {
public:
    struct USRPConfig {
        std::string device_args;
        std::string subdev_spec;
        double center_frequency_hz;
        double sample_rate_hz;
        double gain_db;
        double bandwidth_hz;
        std::string antenna;
    };

    static USRPConfig get_default_config() {
        USRPConfig cfg;
        cfg.device_args = "";
        cfg.subdev_spec = "A:0";
        cfg.center_frequency_hz = 10e9;
        cfg.sample_rate_hz = 1e6;
        cfg.gain_db = 30.0;
        cfg.bandwidth_hz = 1e6;
        cfg.antenna = "TX/RX";
        return cfg;
    }

    static void example_receive_iss(const USRPConfig& cfg) {
        std::cout << "\nUSRP Integration Example: Receive ISS Signals\n";
        std::cout << "=====================================================\n";
        std::cout << "Configuration:\n";
        std::cout << "  Center Frequency: " << cfg.center_frequency_hz / 1e9 << " GHz\n";
        std::cout << "  Sample Rate: " << cfg.sample_rate_hz / 1e6 << " Msps\n";
        std::cout << "  Gain: " << cfg.gain_db << " dB\n";
        std::cout << "  Bandwidth: " << cfg.bandwidth_hz / 1e6 << " MHz\n";
        std::cout << "  Antenna: " << cfg.antenna << "\n\n";
        
        std::cout << "In a real implementation, this would:\n";
        std::cout << "  1. Initialize USRP hardware\n";
        std::cout << "  2. Configure receiver chain\n";
        std::cout << "  3. Set up DMA transfers\n";
        std::cout << "  4. Start streaming\n";
        std::cout << "  5. Receive samples in real-time\n";
        std::cout << "  6. Process through signal pipeline\n";
        std::cout << "  7. Perform Doppler compensation\n";
        std::cout << "  8. Demodulate and extract data\n";
    }

    static void example_transmit_commands(const USRPConfig& cfg) {
        std::cout << "\nUSRP Integration Example: Transmit Commands\n";
        std::cout << "=====================================================\n";
        std::cout << "Configuration:\n";
        std::cout << "  Center Frequency: " << cfg.center_frequency_hz / 1e9 << " GHz\n";
        std::cout << "  Sample Rate: " << cfg.sample_rate_hz / 1e6 << " Msps\n";
        std::cout << "  Gain: " << cfg.gain_db << " dB\n\n";
        
        std::cout << "In a real implementation, this would:\n";
        std::cout << "  1. Generate command sequence\n";
        std::cout << "  2. Modulate with QPSK/PSK\n";
        std::cout << "  3. Apply Doppler pre-compensation\n";
        std::cout << "  4. Configure transmit chain\n";
        std::cout << "  5. Upload waveform to USRP\n";
        std::cout << "  6. Synchronize timing\n";
        std::cout << "  7. Stream samples to transmitter\n";
        std::cout << "  8. Monitor transmission\n";
    }

    static void example_full_duplex_session(const USRPConfig& rx_cfg, const USRPConfig& tx_cfg) {
        std::cout << "\nUSRP Integration Example: Full-Duplex Session\n";
        std::cout << "=====================================================\n";
        std::cout << "RX Configuration:\n";
        std::cout << "  Center Frequency: " << rx_cfg.center_frequency_hz / 1e9 << " GHz\n";
        std::cout << "  Sample Rate: " << rx_cfg.sample_rate_hz / 1e6 << " Msps\n";
        std::cout << "\nTX Configuration:\n";
        std::cout << "  Center Frequency: " << tx_cfg.center_frequency_hz / 1e9 << " GHz\n";
        std::cout << "  Sample Rate: " << tx_cfg.sample_rate_hz / 1e6 << " Msps\n\n";
        
        std::cout << "In a real implementation, this would:\n";
        std::cout << "  1. Initialize dual-channel USRP\n";
        std::cout << "  2. Set up simultaneous RX/TX\n";
        std::cout << "  3. Synchronize clocks\n";
        std::cout << "  4. Monitor antenna switching\n";
        std::cout << "  5. Process command uplink\n";
        std::cout << "  6. Process telemetry downlink\n";
        std::cout << "  7. Maintain link budget\n";
        std::cout << "  8. Log all transactions\n";
    }

    static std::string get_pseudocode() {
        return R"(
// Pseudocode for real USRP integration
#include <uhd/usrp/multi_usrp.hpp>
#include <uhd/utils/safe_main.hpp>

void receive_from_usrp() {
    // Create USRP device
    uhd::usrp::multi_usrp::sptr usrp = uhd::usrp::multi_usrp::make(device_args);
    
    // Configure receiver
    usrp->set_rx_freq(center_freq, 0);
    usrp->set_rx_rate(sample_rate, 0);
    usrp->set_rx_gain(gain_db, 0);
    usrp->set_rx_antenna(antenna, 0);
    
    // Create RX streamer
    uhd::stream_args_t stream_args("fc64", "sc64");
    uhd::rx_streamer::sptr rx_stream = usrp->get_rx_stream(stream_args);
    
    // Start streaming
    uhd::stream_cmd_t stream_cmd(uhd::stream_cmd_t::STREAM_MODE_START_CONTINUOUS);
    stream_cmd.stream_now = true;
    rx_stream->issue_stream_cmd(stream_cmd);
    
    // Receive samples
    const size_t samps_per_buff = rx_stream->get_max_num_samps();
    std::vector<ComplexSample> buff(samps_per_buff);
    uhd::rx_metadata_t md;
    
    for (size_t i = 0; i < num_received_samples; i += samps_per_buff) {
        size_t num_rx_samps = rx_stream->recv(&buff.front(), samps_per_buff, md);
        
        // Process signal through DSP pipeline
        auto compensated = doppler_compensator.compensate_buffer(buff);
        auto filtered = filter.process_buffer(compensated);
        auto demodulated = demodulator.demodulate(filtered);
    }
    
    // Stop streaming
    stream_cmd.stream_mode = uhd::stream_cmd_t::STREAM_MODE_STOP_CONTINUOUS;
    rx_stream->issue_stream_cmd(stream_cmd);
}
        )";
    }
};

#endif