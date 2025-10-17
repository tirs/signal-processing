#define _USE_MATH_DEFINES
#include <iostream>
#include <iomanip>
#include <vector>
#include <complex>
#include <random>
#include <cmath>
#include <fstream>
#include <chrono>
#include <sstream>
#include "signal_processor.h"
#include "satellite_ephemeris.h"
#include "doppler_compensator.h"
#include "cli_parser.h"
#include "output_formatter.h"

using namespace SatelliteSignal;

// Global output file stream
std::ofstream g_output_file;

// Helper function to write to both console and file
void write_output(const std::string& text) {
    std::cout << text;
    if (g_output_file.is_open()) {
        g_output_file << text;
        g_output_file.flush();
    }
}

// Generate synthetic QPSK modulated signal with noise
std::vector<ComplexSample> generate_qpsk_signal(
    size_t num_symbols, double snr_db, double doppler_hz, double sample_rate) {
    
    std::vector<ComplexSample> signal(num_symbols);
    std::mt19937 gen(12345);
    std::uniform_real_distribution<> dis(-1.0, 1.0);
    
    double signal_power = 1.0;
    double noise_power = signal_power / std::pow(10.0, snr_db / 10.0);
    double noise_std = std::sqrt(noise_power);
    
    std::normal_distribution<> noise(0.0, noise_std);
    
    double phase = 0.0;
    double phase_increment = doppler_hz * 2.0 * M_PI / sample_rate;
    
    const std::complex<double> qpsk_symbols[] = {
        std::complex<double>(1.0/std::sqrt(2), 1.0/std::sqrt(2)),
        std::complex<double>(-1.0/std::sqrt(2), 1.0/std::sqrt(2)),
        std::complex<double>(-1.0/std::sqrt(2), -1.0/std::sqrt(2)),
        std::complex<double>(1.0/std::sqrt(2), -1.0/std::sqrt(2))
    };
    
    for (size_t i = 0; i < num_symbols; ++i) {
        int symbol_idx = i % 4;
        std::complex<double> symbol = qpsk_symbols[symbol_idx];
        std::complex<double> carrier = std::polar(1.0, phase);
        
        std::complex<double> received = symbol * carrier;
        received += std::complex<double>(noise(gen), noise(gen));
        
        signal[i] = received;
        phase += phase_increment;
    }
    
    return signal;
}

// Demonstrate satellite ephemeris and Doppler calculation
void demonstrate_ephemeris_and_doppler() {
    write_output("\n+========== SATELLITE EPHEMERIS & DOPPLER DEMO ==========+\n\n");
    
    // Create a sample TLE (ISS approximate elements)
    TLE tle;
    tle.satellite_name = {'I', 'S', 'S', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '\0'};
    tle.catalog_number = 25544;      // ISS catalog number
    tle.epoch_year = 24;             // Epoch year (2024)
    tle.epoch_day = 1.0;             // Epoch day
    tle.ndot = 0.00003;              // First derivative of mean motion
    tle.nddot = 0.0;                 // Second derivative of mean motion
    tle.bstar = 0.00003;             // Drag term
    tle.element_set_num = 1;         // Element set number
    tle.mean_motion = 15.49;         // Mean motion (rev/day)
    tle.element_number = 1;          // Element number
    
    // Set orbital elements (degrees converted to radians in usage)
    tle.inclination = 51.6 * DEG_TO_RAD;
    tle.raan = 120.0 * DEG_TO_RAD;
    tle.eccentricity = 0.0001;
    tle.argument_perigee = 180.0 * DEG_TO_RAD;
    tle.mean_anomaly = 90.0 * DEG_TO_RAD;
    
    SatelliteEphemeris ephemeris(tle);
    GroundStation ground_station = {40.0, -105.0, 1600.0};  // Boulder, Colorado
    
    std::stringstream ss;
    ss << "Ground Station: " << ground_station.latitude << " deg N, "
       << -ground_station.longitude << " deg W\n";
    ss << "Elevation: " << ground_station.elevation << " m\n\n";
    write_output(ss.str());
    
    write_output("+----------+--------+----------+----------+-------------------+\n");
    write_output("| Time(min)| Azimuth| Elevation|  Range   | Doppler(Hz@10GHz) |\n");
    write_output("| (float)  |  (deg) |  (deg)   |  (km)    |    (float)        |\n");
    write_output("+----------+--------+----------+----------+-------------------+\n");
    
    double carrier_freq_hz = 10e9;  // 10 GHz
    
    for (double t = 0; t <= 15; t += 3) {
        StateVector sat_pos = ephemeris.get_position(t);
        TopocentricView view = ephemeris.get_topocentric(sat_pos, ground_station);
        double doppler = ephemeris.calculate_doppler_shift(view, carrier_freq_hz);
        
        std::stringstream row;
        row << std::fixed << std::setprecision(1);
        row << "| " << std::setw(8) << t << " | "
            << std::setw(6) << view.azimuth << " | "
            << std::setw(8) << view.elevation << " | "
            << std::setw(8) << view.range << " | "
            << std::setw(17) << doppler << " |\n";
        write_output(row.str());
    }
    write_output("+----------+--------+----------+----------+-------------------+\n");
}

// Demonstrate signal processing pipeline
void demonstrate_signal_processing() {
    write_output("\n+========== SIGNAL PROCESSING PIPELINE DEMO ==========+\n\n");
    
    const double SAMPLE_RATE = 1e6;  // 1 MHz
    const size_t NUM_SYMBOLS = 1000;
    const double SNR_DB = 8.0;
    const double DOPPLER_HZ = 5000.0;  // 5 kHz Doppler shift
    
    std::stringstream config;
    config << "+------- CONFIGURATION -------+\n";
    config << "| Parameter          | Value    |\n";
    config << "+--------------------+----------+\n";
    config << "| Sample Rate        | " << std::fixed << std::setprecision(1) 
            << std::setw(7) << SAMPLE_RATE / 1e6 << " MHz |\n";
    config << "| Number of Symbols  | " << std::setw(7) << NUM_SYMBOLS << "     |\n";
    config << "| SNR                | " << std::setw(7) << SNR_DB << " dB   |\n";
    config << "| Doppler Shift      | " << std::setw(7) << DOPPLER_HZ / 1000.0 << " kHz  |\n";
    config << "+--------------------+----------+\n\n";
    write_output(config.str());
    
    // Generate signal
    auto signal = generate_qpsk_signal(NUM_SYMBOLS, SNR_DB, DOPPLER_HZ, SAMPLE_RATE);
    write_output("[OK] Generated QPSK signal with Doppler and noise\n");
    
    // Analyze before processing
    auto metrics_before = SignalAnalyzer::analyze(signal);
    std::stringstream before;
    before << "\n+------- BEFORE PROCESSING -------+\n";
    before << "| Metric         | Value        |\n";
    before << "+----------------+--------------+\n";
    before << "| SNR            | " << std::fixed << std::setprecision(2)
            << std::setw(11) << metrics_before.snr_db << " dB |\n";
    before << "| Signal Power   | " << std::setw(11) << metrics_before.signal_power << "    |\n";
    before << "| Lock Status    | " << std::setw(11) 
            << (metrics_before.is_locked ? "LOCKED" : "UNLOCKED") << " |\n";
    before << "+----------------+--------------+\n";
    write_output(before.str());
    
    // Apply Doppler compensation
    DopplerCompensator doppler_comp(SAMPLE_RATE);
    doppler_comp.update_doppler(DOPPLER_HZ);
    auto doppler_corrected = doppler_comp.compensate_buffer(signal);
    write_output("\n[OK] Applied Doppler compensation\n");
    
    // Apply lowpass filter
    double cutoff_freq = 0.1;  // Normalized frequency
    LowpassFilter lpf(cutoff_freq, 64);
    std::vector<double> lpf_input(doppler_corrected.size());
    for (size_t i = 0; i < doppler_corrected.size(); ++i) {
        lpf_input[i] = doppler_corrected[i].real();
    }
    auto filtered = lpf.process_buffer(lpf_input);
    write_output("[OK] Applied lowpass filter (64-tap FIR)\n");
    
    // Convert filtered back to complex for demodulation
    std::vector<ComplexSample> filtered_complex(filtered.size());
    for (size_t i = 0; i < filtered.size(); ++i) {
        filtered_complex[i] = ComplexSample(filtered[i], 0.0);
    }
    
    // Demodulation
    QPSKDemodulator demod(DOPPLER_HZ);
    auto bits = demod.demodulate(filtered_complex);
    std::stringstream demod_info;
    demod_info << "[OK] QPSK demodulation complete\n";
    demod_info << "     Recovered Carrier Frequency: " << demod.get_carrier_frequency() << " Hz\n";
    write_output(demod_info.str());
    
    // Analyze after processing
    auto metrics_after = SignalAnalyzer::analyze(doppler_corrected);
    std::stringstream after;
    after << "\n+------- AFTER PROCESSING -------+\n";
    after << "| Metric         | Value        |\n";
    after << "+----------------+--------------+\n";
    after << "| SNR            | " << std::fixed << std::setprecision(2)
          << std::setw(11) << metrics_after.snr_db << " dB |\n";
    after << "| Signal Power   | " << std::setw(11) << metrics_after.signal_power << "    |\n";
    after << "| Lock Status    | " << std::setw(11)
          << (metrics_after.is_locked ? "LOCKED" : "UNLOCKED") << " |\n";
    after << "+----------------+--------------+\n";
    write_output(after.str());
    
    // Statistics
    std::stringstream stats;
    stats << "\n+------- SIGNAL STATISTICS -------+\n";
    stats << "| Metric         | Value        |\n";
    stats << "+----------------+--------------+\n";
    stats << "| Peak Amplitude | " << std::setw(11) << metrics_after.peak_amplitude << "    |\n";
    stats << "| Mean Amplitude | " << std::setw(11) << metrics_after.mean_amplitude << "    |\n";
    stats << "| Bits Recovered | " << std::setw(11) << bits.size() << "    |\n";
    stats << "+----------------+--------------+\n";
    write_output(stats.str());
}

// Demonstrate frequency search
void demonstrate_frequency_search() {
    write_output("\n+========== FREQUENCY SEARCH DEMO ==========+\n\n");
    
    const double SAMPLE_RATE = 1e6;
    const size_t NUM_SYMBOLS = 500;
    const double SNR_DB = 10.0;
    const double TRUE_DOPPLER = 3500.0;  // Unknown to searcher
    const double CENTER_FREQ = 0.0;
    const double SEARCH_WIDTH = 10000.0;  // +/- 5 kHz search
    const double STEP_SIZE = 100.0;
    
    auto signal = generate_qpsk_signal(NUM_SYMBOLS, SNR_DB, TRUE_DOPPLER, SAMPLE_RATE);
    
    std::stringstream params;
    params << "+------- SEARCH PARAMETERS -------+\n";
    params << "| Parameter          | Value      |\n";
    params << "+--------------------+------------+\n";
    params << "| TRUE Doppler       | " << std::fixed << std::setprecision(1)
           << std::setw(8) << TRUE_DOPPLER << " Hz |\n";
    params << "| Search Range       | +/- " << std::setw(7) << SEARCH_WIDTH/2 << " Hz |\n";
    params << "| Step Size          | " << std::setw(8) << STEP_SIZE << " Hz |\n";
    params << "+--------------------+------------+\n\n";
    write_output(params.str());
    
    auto result = FrequencySearcher::search_frequency_range(
        signal, CENTER_FREQ, SEARCH_WIDTH, STEP_SIZE, SAMPLE_RATE);
    
    std::stringstream results;
    results << "+------- SEARCH RESULTS -------+\n";
    results << "| Metric            | Value      |\n";
    results << "+-------------------+------------+\n";
    
    if (result.found) {
        results << "[OK] Frequency search completed\n\n";
        results << "| Estimated Doppler | " << std::fixed << std::setprecision(1)
                << std::setw(9) << result.best_frequency << " Hz |\n";
        results << "| Correlation Metric| " << std::setw(9) << result.best_metric << "    |\n";
        results << "| Frequency Error   | " << std::setw(9) 
                << (result.best_frequency - TRUE_DOPPLER) << " Hz |\n";
        double accuracy = 100.0 * (1.0 - std::fabs(result.best_frequency - TRUE_DOPPLER) / std::fabs(TRUE_DOPPLER));
        results << "| Accuracy          | " << std::setw(9) << accuracy << " %  |\n";
        results << "+-------------------+------------+\n";
    } else {
        results << "[FAIL] Frequency search did not converge\n";
        results << "+-------------------+------------+\n";
    }
    write_output(results.str());
}

// Main demonstration
int main(int argc, char* argv[]) {
    CLIParser::Options opts = CLIParser::parse(argc, argv);
    
    // Note: Benchmarks and tests are run via separate executables
    // (benchmark_suite.exe and unit_tests.exe) to avoid linking issues
    
    // Generate timestamped filename for results
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    
    std::string filename;
    if (!opts.output_file.empty()) {
        filename = opts.output_file;
    } else {
        ss << "satellite_signal_results_" << std::put_time(std::localtime(&time_t_now), "%Y%m%d_%H%M%S") << ".txt";
        filename = ss.str();
    }
    
    // Open output file
    g_output_file.open(filename, std::ios::out);
    if (!g_output_file.is_open()) {
        std::cerr << "Error: Could not open output file " << filename << std::endl;
        return 1;
    }
    
    std::string header = "\n";
    header += "================================================================\n";
    header += "      SATELLITE SIGNAL PROCESSING SYSTEM DEMONSTRATION         \n";
    header += "    Demonstrating real-time signal processing for LEO/GEO      \n";
    header += "================================================================\n";
    header += "\nConfiguration:\n";
    header += "  Sample Rate: " + std::to_string(opts.sample_rate / 1e6) + " Msps\n";
    header += "  SNR: " + std::to_string(opts.snr_db) + " dB\n";
    header += "  Doppler: " + std::to_string(opts.doppler_hz) + " Hz\n";
    header += "  Carrier Frequency: " + std::to_string(opts.carrier_freq_hz / 1e9) + " GHz\n\n";
    
    write_output(header);
    
    try {
        demonstrate_ephemeris_and_doppler();
        demonstrate_signal_processing();
        demonstrate_frequency_search();
        
        std::string footer = "\n================================================================\n";
        footer += "              DEMONSTRATION COMPLETED SUCCESSFULLY            \n";
        footer += "================================================================\n\n";
        write_output(footer);
        
        std::cout << "\nResults saved to: " << filename << "\n";
        if (opts.verbose) {
            std::cout << "Output format: " << opts.output_format << "\n";
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        g_output_file.close();
        return 1;
    }
    
    g_output_file.close();
    return 0;
}