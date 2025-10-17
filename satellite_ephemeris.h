#ifndef SATELLITE_EPHEMERIS_H
#define SATELLITE_EPHEMERIS_H

#define _USE_MATH_DEFINES
#include <cmath>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#include <array>

namespace SatelliteSignal {

constexpr double EARTH_RADIUS_KM = 6371.0;
constexpr double EARTH_MU = 398600.4418;  // GM in km^3/s^2
constexpr double DEG_TO_RAD = 3.14159265358979323846 / 180.0;
constexpr double RAD_TO_DEG = 180.0 / 3.14159265358979323846;

// Satellite position and velocity in ECI (Earth-Centered Inertial)
struct StateVector {
    double x, y, z;      // Position in km
    double vx, vy, vz;   // Velocity in km/s
};

// Ground station location
struct GroundStation {
    double latitude;     // degrees
    double longitude;    // degrees
    double elevation;    // meters
};

// Topocentric view (from ground station perspective)
struct TopocentricView {
    double azimuth;      // degrees (0-360)
    double elevation;    // degrees (-90 to +90)
    double range;        // km
    double range_rate;   // km/s (positive = receding)
};

// Two-Line Element (TLE) set for SGP4 propagation
struct TLE {
    std::array<char, 24> satellite_name;
    long catalog_number;
    int epoch_year;
    double epoch_day;
    double ndot;         // First derivative of mean motion
    double nddot;        // Second derivative of mean motion
    double bstar;        // Drag term
    int element_set_num;
    double mean_motion;
    long element_number;
    
    // Orbital elements
    double inclination;
    double raan;         // Right ascension of ascending node
    double eccentricity;
    double argument_perigee;
    double mean_anomaly;
};

// Simplified SGP4 propagator
class SatelliteEphemeris {
private:
    TLE tle;
    double semi_major_axis;
    double mean_motion_radians;

public:
    SatelliteEphemeris(const TLE& tle_data) : tle(tle_data) {
        initialize();
    }

    StateVector get_position(double minutes_from_epoch) {
        double mean_anomaly = tle.mean_anomaly + mean_motion_radians * minutes_from_epoch;
        mean_anomaly = std::fmod(mean_anomaly, 2.0 * M_PI);
        
        double eccentric_anomaly = solve_kepler_equation(mean_anomaly, tle.eccentricity);
        
        double true_anomaly = 2.0 * std::atan2(
            std::sqrt(1.0 + tle.eccentricity) * std::sin(eccentric_anomaly / 2.0),
            std::sqrt(1.0 - tle.eccentricity) * std::cos(eccentric_anomaly / 2.0)
        );
        
        double radius = semi_major_axis * (1.0 - tle.eccentricity * std::cos(eccentric_anomaly));
        
        return perifocal_to_eci(true_anomaly, radius);
    }

    TopocentricView get_topocentric(const StateVector& satellite_pos, 
                                    const GroundStation& station) {
        StateVector station_pos = ground_station_to_eci(station);
        
        double dx = satellite_pos.x - station_pos.x;
        double dy = satellite_pos.y - station_pos.y;
        double dz = satellite_pos.z - station_pos.z;
        
        double range = std::sqrt(dx*dx + dy*dy + dz*dz);
        
        double lat_rad = station.latitude * DEG_TO_RAD;
        double lon_rad = station.longitude * DEG_TO_RAD;
        
        double sin_lat = std::sin(lat_rad);
        double cos_lat = std::cos(lat_rad);
        double sin_lon = std::sin(lon_rad);
        double cos_lon = std::cos(lon_rad);
        
        double south = sin_lat * cos_lon * dx + 
                       sin_lat * sin_lon * dy - 
                       cos_lat * dz;
        
        double east = -sin_lon * dx + cos_lon * dy;
        
        double zenith = cos_lat * cos_lon * dx + 
                        cos_lat * sin_lon * dy + 
                        sin_lat * dz;
        
        double azimuth = std::atan2(east, south) * RAD_TO_DEG;
        if (azimuth < 0) azimuth += 360.0;
        
        double elevation = std::asin(zenith / range) * RAD_TO_DEG;
        
        double range_rate = ((satellite_pos.vx - station_pos.vx) * dx +
                            (satellite_pos.vy - station_pos.vy) * dy +
                            (satellite_pos.vz - station_pos.vz) * dz) / range;
        
        return {azimuth, elevation, range, range_rate};
    }

    double calculate_doppler_shift(const TopocentricView& view, 
                                  double carrier_frequency_hz) {
        // Negative because satellite is moving relative to ground station
        double doppler_shift = -view.range_rate * 1000.0 * carrier_frequency_hz / 
                              (SPEED_OF_LIGHT * 1000.0);
        return doppler_shift;
    }

private:
    void initialize() {
        double mean_motion_rev_per_day = tle.mean_motion;
        mean_motion_radians = mean_motion_rev_per_day * 2.0 * M_PI / 1440.0;
        
        double n = mean_motion_radians;
        semi_major_axis = std::pow(EARTH_MU / (n * n), 1.0 / 3.0);
    }

    double solve_kepler_equation(double mean_anomaly, double eccentricity) {
        double E = mean_anomaly;
        const double TOLERANCE = 1e-8;
        const int MAX_ITERATIONS = 50;
        
        for (int i = 0; i < MAX_ITERATIONS; ++i) {
            double f = E - eccentricity * std::sin(E) - mean_anomaly;
            double df = 1.0 - eccentricity * std::cos(E);
            double E_new = E - f / df;
            
            if (std::fabs(E_new - E) < TOLERANCE) {
                return E_new;
            }
            E = E_new;
        }
        
        return E;
    }

    StateVector perifocal_to_eci(double true_anomaly, double radius) {
        double cos_ta = std::cos(true_anomaly);
        double sin_ta = std::sin(true_anomaly);
        
        double p_x = radius * cos_ta;
        double p_y = radius * sin_ta;
        
        double v_magnitude = std::sqrt(EARTH_MU / semi_major_axis * 
                                       (2.0 / radius - 1.0 / semi_major_axis));
        double v_x = -v_magnitude * sin_ta;
        double v_y = v_magnitude * (tle.eccentricity + cos_ta);
        
        double cos_raan = std::cos(tle.raan);
        double sin_raan = std::sin(tle.raan);
        double cos_inc = std::cos(tle.inclination);
        double sin_inc = std::sin(tle.inclination);
        double cos_arg = std::cos(tle.argument_perigee);
        double sin_arg = std::sin(tle.argument_perigee);
        
        StateVector eci;
        eci.x = (cos_raan * cos_arg - sin_raan * sin_arg * cos_inc) * p_x +
                (-cos_raan * sin_arg - sin_raan * cos_arg * cos_inc) * p_y;
        eci.y = (sin_raan * cos_arg + cos_raan * sin_arg * cos_inc) * p_x +
                (-sin_raan * sin_arg + cos_raan * cos_arg * cos_inc) * p_y;
        eci.z = sin_inc * sin_arg * p_x + sin_inc * cos_arg * p_y;
        
        eci.vx = (cos_raan * cos_arg - sin_raan * sin_arg * cos_inc) * v_x +
                 (-cos_raan * sin_arg - sin_raan * cos_arg * cos_inc) * v_y;
        eci.vy = (sin_raan * cos_arg + cos_raan * sin_arg * cos_inc) * v_x +
                 (-sin_raan * sin_arg + cos_raan * cos_arg * cos_inc) * v_y;
        eci.vz = sin_inc * sin_arg * v_x + sin_inc * cos_arg * v_y;
        
        return eci;
    }

    StateVector ground_station_to_eci(const GroundStation& station) {
        double lat_rad = station.latitude * DEG_TO_RAD;
        double lon_rad = station.longitude * DEG_TO_RAD;
        double alt_km = station.elevation / 1000.0;
        
        double radius = EARTH_RADIUS_KM + alt_km;
        double cos_lat = std::cos(lat_rad);
        double sin_lat = std::sin(lat_rad);
        
        StateVector station_eci;
        station_eci.x = radius * cos_lat * std::cos(lon_rad);
        station_eci.y = radius * cos_lat * std::sin(lon_rad);
        station_eci.z = radius * sin_lat;
        
        double earth_rotation_rate = 7.292115e-5;  // rad/s
        station_eci.vx = -earth_rotation_rate * station_eci.y;
        station_eci.vy = earth_rotation_rate * station_eci.x;
        station_eci.vz = 0.0;
        
        return station_eci;
    }
};

}  // namespace SatelliteSignal

#endif // SATELLITE_EPHEMERIS_H