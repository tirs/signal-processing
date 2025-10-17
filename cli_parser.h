#ifndef CLI_PARSER_H
#define CLI_PARSER_H

#include <string>
#include <map>
#include <vector>
#include <iostream>
#include <sstream>

class CLIParser {
public:
    struct Options {
        double sample_rate = 1e6;
        size_t num_symbols = 1000;
        double snr_db = 8.0;
        double doppler_hz = 5000.0;
        double carrier_freq_hz = 10e9;
        double ground_lat = 40.0;
        double ground_lon = -105.0;
        double ground_elev = 1600.0;
        std::string output_format = "text";
        bool enable_benchmark = false;
        bool enable_tests = false;
        bool enable_json_output = false;
        bool enable_csv_output = false;
        bool verbose = false;
        std::string output_file = "";
    };

    static Options parse(int argc, char* argv[]) {
        Options opts;
        
        for (int i = 1; i < argc; ++i) {
            std::string arg = argv[i];
            
            if (arg == "--help" || arg == "-h") {
                print_help();
                exit(0);
            }
            else if (arg == "--sample-rate" && i + 1 < argc) {
                opts.sample_rate = std::stod(argv[++i]);
            }
            else if (arg == "--snr" && i + 1 < argc) {
                opts.snr_db = std::stod(argv[++i]);
            }
            else if (arg == "--doppler" && i + 1 < argc) {
                opts.doppler_hz = std::stod(argv[++i]);
            }
            else if (arg == "--symbols" && i + 1 < argc) {
                opts.num_symbols = std::stoull(argv[++i]);
            }
            else if (arg == "--carrier-freq" && i + 1 < argc) {
                opts.carrier_freq_hz = std::stod(argv[++i]);
            }
            else if (arg == "--lat" && i + 1 < argc) {
                opts.ground_lat = std::stod(argv[++i]);
            }
            else if (arg == "--lon" && i + 1 < argc) {
                opts.ground_lon = std::stod(argv[++i]);
            }
            else if (arg == "--elev" && i + 1 < argc) {
                opts.ground_elev = std::stod(argv[++i]);
            }
            else if (arg == "--output-format" && i + 1 < argc) {
                opts.output_format = argv[++i];
            }
            else if (arg == "--output-file" && i + 1 < argc) {
                opts.output_file = argv[++i];
            }
            else if (arg == "--json") {
                opts.enable_json_output = true;
            }
            else if (arg == "--csv") {
                opts.enable_csv_output = true;
            }
            else if (arg == "--benchmark") {
                opts.enable_benchmark = true;
            }
            else if (arg == "--test" || arg == "--tests") {
                opts.enable_tests = true;
            }
            else if (arg == "--verbose" || arg == "-v") {
                opts.verbose = true;
            }
        }
        
        return opts;
    }

private:
    static void print_help() {
        std::cout << "\nSatellite Signal Processing System - Command Line Interface\n";
        std::cout << "============================================================\n\n";
        std::cout << "USAGE:\n";
        std::cout << "  satellite_signal_demo [OPTIONS]\n\n";
        std::cout << "OPTIONS:\n";
        std::cout << "  --help, -h                Print this help message\n";
        std::cout << "  --sample-rate RATE        Sample rate in Hz (default: 1000000)\n";
        std::cout << "  --snr SNR_DB              Signal-to-noise ratio in dB (default: 8.0)\n";
        std::cout << "  --doppler HZ              Doppler shift in Hz (default: 5000.0)\n";
        std::cout << "  --symbols COUNT           Number of symbols (default: 1000)\n";
        std::cout << "  --carrier-freq HZ         Carrier frequency in Hz (default: 10e9)\n";
        std::cout << "  --lat DEGREES             Ground station latitude (default: 40.0)\n";
        std::cout << "  --lon DEGREES             Ground station longitude (default: -105.0)\n";
        std::cout << "  --elev METERS             Ground station elevation (default: 1600.0)\n";
        std::cout << "  --output-format FORMAT    Output format: text|json|csv (default: text)\n";
        std::cout << "  --output-file FILE        Output file path\n";
        std::cout << "  --json                    Enable JSON output\n";
        std::cout << "  --csv                     Enable CSV output\n";
        std::cout << "  --benchmark               Run benchmark suite\n";
        std::cout << "  --test                    Run unit tests\n";
        std::cout << "  --verbose, -v             Enable verbose output\n\n";
        std::cout << "EXAMPLES:\n";
        std::cout << "  satellite_signal_demo --snr 12.0 --doppler 3000\n";
        std::cout << "  satellite_signal_demo --benchmark\n";
        std::cout << "  satellite_signal_demo --test\n";
        std::cout << "  satellite_signal_demo --json --output-file results.json\n";
    }
};

#endif