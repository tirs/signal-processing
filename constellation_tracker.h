#ifndef CONSTELLATION_TRACKER_H
#define CONSTELLATION_TRACKER_H

#include <vector>
#include <map>
#include <string>
#include <cmath>
#include <algorithm>

struct Satellite {
    int catalog_number;
    std::string name;
    double latitude;
    double longitude;
    double altitude_km;
    double velocity_km_s;
    double signal_strength_dbm;
    bool in_range;
};

struct PassPrediction {
    std::string satellite_name;
    double rise_time_minutes;
    double set_time_minutes;
    double max_elevation_degrees;
    double duration_minutes;
    double doppler_rate_hz_per_s;
};

struct NetworkLink {
    std::string from_satellite;
    std::string to_satellite;
    double link_budget_margin_db;
    bool is_viable;
    double data_rate_mbps;
};

class ConstellationTracker {
public:
    struct ConstellationConfig {
        std::string constellation_name;
        int num_satellites;
        double orbit_altitude_km;
        double orbital_inclination_deg;
        int num_orbital_planes;
        int sats_per_plane;
    };

    ConstellationTracker(const ConstellationConfig& cfg) : config(cfg) {
        initialize_constellation();
    }

    std::vector<Satellite> get_visible_satellites(double current_time_minutes) {
        std::vector<Satellite> visible;
        
        for (auto& sat : satellites) {
            if (is_satellite_visible(sat, current_time_minutes)) {
                visible.push_back(sat);
            }
        }
        
        return visible;
    }

    std::vector<PassPrediction> predict_passes(double start_time_min, double duration_hours) {
        std::vector<PassPrediction> passes;
        
        double end_time = start_time_min + duration_hours * 60.0;
        
        for (const auto& sat : satellites) {
            double pass_time = predict_next_pass(sat.name, start_time_min);
            
            if (pass_time < end_time) {
                PassPrediction pred;
                pred.satellite_name = sat.name;
                pred.rise_time_minutes = pass_time;
                pred.set_time_minutes = pass_time + estimate_pass_duration(sat);
                pred.max_elevation_degrees = estimate_max_elevation(sat);
                pred.duration_minutes = estimate_pass_duration(sat);
                pred.doppler_rate_hz_per_s = estimate_doppler_rate(sat);
                
                passes.push_back(pred);
            }
        }
        
        std::sort(passes.begin(), passes.end(), 
            [](const PassPrediction& a, const PassPrediction& b) {
                return a.rise_time_minutes < b.rise_time_minutes;
            });
        
        return passes;
    }

    std::vector<NetworkLink> analyze_inter_satellite_links() {
        std::vector<NetworkLink> links;
        
        for (size_t i = 0; i < satellites.size(); ++i) {
            for (size_t j = i + 1; j < satellites.size(); ++j) {
                double distance = calculate_distance(satellites[i], satellites[j]);
                double link_budget = calculate_link_budget(distance);
                
                if (link_budget > 0.0) {
                    NetworkLink link;
                    link.from_satellite = satellites[i].name;
                    link.to_satellite = satellites[j].name;
                    link.link_budget_margin_db = link_budget;
                    link.is_viable = link_budget > 3.0;
                    link.data_rate_mbps = estimate_isl_data_rate(link_budget);
                    
                    links.push_back(link);
                }
            }
        }
        
        return links;
    }

    std::vector<PassPrediction> find_handover_events(double duration_hours) {
        std::vector<PassPrediction> handovers;
        
        auto passes = predict_passes(0.0, duration_hours);
        
        for (size_t i = 0; i + 1 < passes.size(); ++i) {
            double gap = passes[i + 1].rise_time_minutes - passes[i].set_time_minutes;
            
            if (gap > 0.0 && gap < 5.0) {
                PassPrediction handover = passes[i];
                handover.satellite_name = passes[i].satellite_name + " -> " + passes[i + 1].satellite_name;
                handover.duration_minutes = gap;
                handovers.push_back(handover);
            }
        }
        
        return handovers;
    }

    void optimize_ground_station_network(double coverage_target_percent = 95.0) {
        int num_stations_required = 1;
        
        for (int n = 1; n <= 10; ++n) {
            double coverage = estimate_global_coverage(n);
            if (coverage >= coverage_target_percent) {
                num_stations_required = n;
                break;
            }
        }
        
        std::cout << "Recommended number of ground stations for " 
                  << coverage_target_percent << "% coverage: " 
                  << num_stations_required << "\n";
        
        std::cout << "Suggested locations:\n";
        generate_optimal_station_locations(num_stations_required);
    }

    double estimate_global_coverage(int num_ground_stations) {
        double coverage = 0.0;
        
        for (int n = 0; n < num_ground_stations; ++n) {
            double coverage_area = 4.0 * 3.14159265359 * std::pow(satellites[0].altitude_km / 1000.0, 2);
            double earth_area = 4.0 * 3.14159265359 * std::pow(6371.0, 2);
            coverage += std::min(coverage_area / earth_area, 1.0);
        }
        
        return std::min(coverage * 100.0, 100.0);
    }

    int get_satellite_count() const {
        return satellites.size();
    }

    void print_constellation_info() {
        std::cout << "\nConstellation: " << config.constellation_name << "\n";
        std::cout << "===============================================\n";
        std::cout << "Number of Satellites: " << satellites.size() << "\n";
        std::cout << "Orbital Altitude: " << config.orbit_altitude_km << " km\n";
        std::cout << "Inclination: " << config.orbital_inclination_deg << " degrees\n";
        std::cout << "Orbital Planes: " << config.num_orbital_planes << "\n";
        std::cout << "Satellites per Plane: " << config.sats_per_plane << "\n\n";
    }

private:
    ConstellationConfig config;
    std::vector<Satellite> satellites;

    void initialize_constellation() {
        satellites.clear();
        
        for (int plane = 0; plane < config.num_orbital_planes; ++plane) {
            for (int sat = 0; sat < config.sats_per_plane; ++sat) {
                Satellite s;
                s.catalog_number = plane * config.sats_per_plane + sat + 1000;
                s.name = config.constellation_name + "-" + std::to_string(s.catalog_number);
                
                double plane_angle = (plane * 360.0) / config.num_orbital_planes;
                double sat_angle = (sat * 360.0) / config.sats_per_plane;
                
                s.latitude = 0.0;
                s.longitude = sat_angle;
                s.altitude_km = config.orbit_altitude_km;
                s.velocity_km_s = std::sqrt(3.986e5 / (6371.0 + s.altitude_km));
                s.signal_strength_dbm = -120.0 + (plane % 3) * 5;
                s.in_range = false;
                
                satellites.push_back(s);
            }
        }
    }

    bool is_satellite_visible(const Satellite& sat, double current_time_min) {
        double elevation = estimate_elevation(sat, current_time_min);
        return elevation > 5.0;
    }

    double estimate_elevation(const Satellite& sat, double time_min) {
        double phase = std::fmod(time_min * 360.0 / 90.0, 360.0);
        double elevation = 80.0 * std::sin(phase * 3.14159265359 / 180.0);
        return elevation;
    }

    double predict_next_pass(const std::string& sat_name, double start_time_min) {
        return start_time_min + 20.0 + (std::hash<std::string>{}(sat_name) % 100) / 10.0;
    }

    double estimate_pass_duration(const Satellite& sat) {
        double altitude_factor = sat.altitude_km / 400.0;
        return 10.0 * altitude_factor;
    }

    double estimate_max_elevation(const Satellite& sat) {
        return 60.0 + (sat.altitude_km / 400.0) * 20.0;
    }

    double estimate_doppler_rate(const Satellite& sat) {
        return sat.velocity_km_s * 100.0;
    }

    double calculate_distance(const Satellite& sat1, const Satellite& sat2) {
        double dlat = sat1.latitude - sat2.latitude;
        double dlon = sat1.longitude - sat2.longitude;
        return std::sqrt(dlat * dlat + dlon * dlon) * 111.32;
    }

    double calculate_link_budget(double distance_km) {
        const double TX_POWER = 30.0;
        const double TX_GAIN = 20.0;
        const double RX_GAIN = 20.0;
        const double PATH_LOSS = 20 * std::log10(distance_km) + 20 * std::log10(10e9 / 3e8);
        const double NOISE_FIGURE = 5.0;
        
        double link_budget = TX_POWER + TX_GAIN + RX_GAIN - PATH_LOSS - NOISE_FIGURE;
        return link_budget;
    }

    double estimate_isl_data_rate(double link_budget_db) {
        const double SNR_REQUIRED = 10.0;
        
        if (link_budget_db < SNR_REQUIRED) return 0.0;
        
        double snr_margin = link_budget_db - SNR_REQUIRED;
        return std::min(1000.0 * (1.0 + snr_margin / 20.0), 10000.0);
    }

    void generate_optimal_station_locations(int num_stations) {
        double spacing = 360.0 / num_stations;
        
        for (int i = 0; i < num_stations; ++i) {
            double longitude = i * spacing;
            double latitude = 45.0;
            
            std::cout << "  Station " << (i + 1) << ": " 
                      << latitude << "N, " << longitude << "E\n";
        }
    }
};

#endif