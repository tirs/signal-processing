#include <iostream>
#include <vector>
#include <complex>
#include <random>
#include <cmath>
#include "signal_processor.h"
#include "satellite_ephemeris.h"
#include "doppler_compensator.h"
#include "benchmark_framework.h"

using namespace SatelliteSignal;

void benchmark_fir_filter() {
    const size_t BUFFER_SIZE = 10000;
    std::vector<double> input(BUFFER_SIZE);
    std::mt19937 gen(42);
    std::normal_distribution<> dis(0.0, 1.0);
    
    for (size_t i = 0; i < BUFFER_SIZE; ++i) {
        input[i] = dis(gen);
    }

    LowpassFilter filter(0.1, 64);
    
    BENCHMARK("FIR Filter (10k samples, 64-tap)", [&]() {
        filter.process_buffer(input);
    }, 100);
}

void benchmark_doppler_compensation() {
    const size_t BUFFER_SIZE = 10000;
    std::vector<ComplexSample> signal(BUFFER_SIZE);
    std::mt19937 gen(42);
    std::uniform_real_distribution<> dis(-1.0, 1.0);
    
    for (size_t i = 0; i < BUFFER_SIZE; ++i) {
        signal[i] = ComplexSample(dis(gen), dis(gen));
    }

    DopplerCompensator compensator(1e6);
    compensator.update_doppler(5000.0);
    
    BENCHMARK("Doppler Compensation (10k samples)", [&]() {
        compensator.compensate_buffer(signal);
    }, 100);
}

void benchmark_qpsk_demodulation() {
    const size_t BUFFER_SIZE = 1000;
    std::vector<ComplexSample> signal(BUFFER_SIZE);
    std::mt19937 gen(42);
    std::uniform_real_distribution<> dis(-1.0, 1.0);
    
    for (size_t i = 0; i < BUFFER_SIZE; ++i) {
        signal[i] = ComplexSample(dis(gen), dis(gen));
    }

    QPSKDemodulator demod(0.0);
    
    BENCHMARK("QPSK Demodulation (1k symbols)", [&]() {
        demod.demodulate(signal);
    }, 100);
}

void benchmark_signal_analysis() {
    const size_t BUFFER_SIZE = 10000;
    std::vector<ComplexSample> signal(BUFFER_SIZE);
    std::mt19937 gen(42);
    std::uniform_real_distribution<> dis(-1.0, 1.0);
    
    for (size_t i = 0; i < BUFFER_SIZE; ++i) {
        signal[i] = ComplexSample(dis(gen), dis(gen));
    }
    
    BENCHMARK("Signal Analysis (10k samples)", [&]() {
        SignalAnalyzer::analyze(signal);
    }, 100);
}

void benchmark_frequency_search() {
    const size_t BUFFER_SIZE = 500;
    std::vector<ComplexSample> signal(BUFFER_SIZE);
    std::mt19937 gen(42);
    std::uniform_real_distribution<> dis(-1.0, 1.0);
    
    for (size_t i = 0; i < BUFFER_SIZE; ++i) {
        signal[i] = ComplexSample(dis(gen), dis(gen));
    }
    
    BENCHMARK("Frequency Search (500 symbols, +-5kHz)", [&]() {
        FrequencySearcher::search_frequency_range(signal, 0.0, 10000.0, 100.0, 1e6);
    }, 10);
}

void benchmark_ephemeris_calculation() {
    TLE tle;
    tle.satellite_name = {'I', 'S', 'S', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
                          ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
                          ' ', '\0'};
    tle.catalog_number = 25544;
    tle.epoch_year = 24;
    tle.epoch_day = 1.0;
    tle.ndot = 0.00003;
    tle.nddot = 0.0;
    tle.bstar = 0.00003;
    tle.element_set_num = 1;
    tle.mean_motion = 15.49;
    tle.element_number = 1;
    tle.inclination = 51.6 * DEG_TO_RAD;
    tle.raan = 120.0 * DEG_TO_RAD;
    tle.eccentricity = 0.0001;
    tle.argument_perigee = 180.0 * DEG_TO_RAD;
    tle.mean_anomaly = 90.0 * DEG_TO_RAD;

    SatelliteEphemeris ephemeris(tle);
    GroundStation ground_station = {40.0, -105.0, 1600.0};
    
    BENCHMARK("Ephemeris Position Calculation", [&]() {
        for (double t = 0; t < 15; t += 1) {
            ephemeris.get_position(t);
        }
    }, 100);
}

void benchmark_topocentric_conversion() {
    TLE tle;
    tle.satellite_name = {'I', 'S', 'S', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
                          ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
                          ' ', '\0'};
    tle.catalog_number = 25544;
    tle.epoch_year = 24;
    tle.epoch_day = 1.0;
    tle.ndot = 0.00003;
    tle.nddot = 0.0;
    tle.bstar = 0.00003;
    tle.element_set_num = 1;
    tle.mean_motion = 15.49;
    tle.element_number = 1;
    tle.inclination = 51.6 * DEG_TO_RAD;
    tle.raan = 120.0 * DEG_TO_RAD;
    tle.eccentricity = 0.0001;
    tle.argument_perigee = 180.0 * DEG_TO_RAD;
    tle.mean_anomaly = 90.0 * DEG_TO_RAD;

    SatelliteEphemeris ephemeris(tle);
    GroundStation ground_station = {40.0, -105.0, 1600.0};
    StateVector sat_pos = ephemeris.get_position(0.0);
    
    BENCHMARK("Topocentric Conversion (AZ/EL/Range)", [&]() {
        ephemeris.get_topocentric(sat_pos, ground_station);
    }, 1000);
}

void benchmark_doppler_shift_calculation() {
    TLE tle;
    tle.satellite_name = {'I', 'S', 'S', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
                          ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
                          ' ', '\0'};
    tle.catalog_number = 25544;
    tle.epoch_year = 24;
    tle.epoch_day = 1.0;
    tle.ndot = 0.00003;
    tle.nddot = 0.0;
    tle.bstar = 0.00003;
    tle.element_set_num = 1;
    tle.mean_motion = 15.49;
    tle.element_number = 1;
    tle.inclination = 51.6 * DEG_TO_RAD;
    tle.raan = 120.0 * DEG_TO_RAD;
    tle.eccentricity = 0.0001;
    tle.argument_perigee = 180.0 * DEG_TO_RAD;
    tle.mean_anomaly = 90.0 * DEG_TO_RAD;

    SatelliteEphemeris ephemeris(tle);
    GroundStation ground_station = {40.0, -105.0, 1600.0};
    StateVector sat_pos = ephemeris.get_position(0.0);
    TopocentricView view = ephemeris.get_topocentric(sat_pos, ground_station);
    
    BENCHMARK("Doppler Shift Calculation", [&]() {
        ephemeris.calculate_doppler_shift(view, 10e9);
    }, 10000);
}

void run_all_benchmarks() {
    std::cout << "\n";
    std::cout << "================================================================\n";
    std::cout << "              SATELLITE SIGNAL PROCESSING BENCHMARKS           \n";
    std::cout << "================================================================\n";
    std::cout << "\nRunning comprehensive performance analysis...\n";
    
    benchmark_fir_filter();
    benchmark_doppler_compensation();
    benchmark_qpsk_demodulation();
    benchmark_signal_analysis();
    benchmark_frequency_search();
    benchmark_ephemeris_calculation();
    benchmark_topocentric_conversion();
    benchmark_doppler_shift_calculation();
    
    BenchmarkFramework::instance().print_all_results();
    BenchmarkFramework::instance().export_json("benchmark_results.json");
}

int main() {
    run_all_benchmarks();
    return 0;
}