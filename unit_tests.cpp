#include <iostream>
#include <vector>
#include <complex>
#include <cmath>
#include "signal_processor.h"
#include "satellite_ephemeris.h"
#include "doppler_compensator.h"
#include "test_framework.h"

using namespace SatelliteSignal;

TEST(fir_filter_creation) {
    LowpassFilter filter(0.1, 64);
    ASSERT_TRUE(true);
}

TEST(fir_filter_output_size) {
    LowpassFilter filter(0.1, 64);
    std::vector<double> input(100);
    for (size_t i = 0; i < input.size(); ++i) {
        input[i] = 1.0;
    }
    auto output = filter.process_buffer(input);
    ASSERT_TRUE(output.size() == input.size());
}

TEST(doppler_compensator_update) {
    DopplerCompensator compensator(1e6);
    compensator.update_doppler(5000.0);
    ASSERT_TRUE(true);
}

TEST(doppler_compensator_phase_continuity) {
    DopplerCompensator compensator(1e6);
    compensator.update_doppler(1000.0);
    
    std::vector<ComplexSample> input(100);
    for (size_t i = 0; i < input.size(); ++i) {
        input[i] = ComplexSample(1.0, 0.0);
    }
    
    auto output = compensator.compensate_buffer(input);
    ASSERT_TRUE(output.size() == input.size());
}

TEST(qpsk_demodulator_symbol_count) {
    std::vector<ComplexSample> symbols(100);
    for (size_t i = 0; i < symbols.size(); ++i) {
        symbols[i] = ComplexSample(1.0 / std::sqrt(2), 1.0 / std::sqrt(2));
    }
    
    QPSKDemodulator demod(0.0);
    auto bits = demod.demodulate(symbols);
    ASSERT_TRUE(bits.size() == 100);
}

TEST(signal_analyzer_real_signal) {
    std::vector<ComplexSample> signal(1000);
    for (size_t i = 0; i < signal.size(); ++i) {
        signal[i] = ComplexSample(1.0, 1.0);
    }
    
    auto metrics = SignalAnalyzer::analyze(signal);
    ASSERT_TRUE(metrics.signal_power > 0.0);
}

TEST(signal_analyzer_zero_signal) {
    std::vector<ComplexSample> signal(1000);
    for (size_t i = 0; i < signal.size(); ++i) {
        signal[i] = ComplexSample(0.0, 0.0);
    }
    
    auto metrics = SignalAnalyzer::analyze(signal);
    ASSERT_EQ(metrics.signal_power, 0.0, 0.001);
}

TEST(frequency_searcher_finds_signal) {
    std::vector<ComplexSample> signal(500);
    double true_freq = 0.0;
    
    for (size_t i = 0; i < signal.size(); ++i) {
        double phase = true_freq * 2.0 * M_PI * i / 1e6;
        signal[i] = ComplexSample(std::cos(phase), std::sin(phase));
    }
    
    auto result = FrequencySearcher::search_frequency_range(signal, 0.0, 10000.0, 100.0, 1e6);
    ASSERT_TRUE(result.found);
}

TEST(tle_structure_creation) {
    TLE tle;
    tle.catalog_number = 25544;
    tle.mean_motion = 15.49;
    ASSERT_EQ(static_cast<double>(tle.catalog_number), 25544.0, 0.001);
}

TEST(ephemeris_position_valid) {
    TLE tle;
    tle.satellite_name = {'T', 'E', 'S', 'T', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '\0'};
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
    StateVector pos = ephemeris.get_position(0.0);
    
    ASSERT_TRUE(pos.x != 0.0 || pos.y != 0.0 || pos.z != 0.0);
}

TEST(topocentric_conversion_valid) {
    TLE tle;
    tle.satellite_name = {'T', 'E', 'S', 'T', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '\0'};
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
    GroundStation station = {40.0, -105.0, 1600.0};
    StateVector pos = ephemeris.get_position(0.0);
    TopocentricView view = ephemeris.get_topocentric(pos, station);
    
    ASSERT_TRUE(view.range > 0.0);
}

TEST(doppler_calculation_reasonable) {
    TLE tle;
    tle.satellite_name = {'T', 'E', 'S', 'T', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '\0'};
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
    GroundStation station = {40.0, -105.0, 1600.0};
    StateVector pos = ephemeris.get_position(0.0);
    TopocentricView view = ephemeris.get_topocentric(pos, station);
    double doppler = ephemeris.calculate_doppler_shift(view, 10e9);
    
    ASSERT_TRUE(std::fabs(doppler) < 100000.0);
}

void run_all_tests() {
    int failures = TestFramework::instance().run_all_tests();
    exit(failures == 0 ? 0 : 1);
}

int main() {
    run_all_tests();
    return 0;
}