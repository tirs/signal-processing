#ifndef BENCHMARK_FRAMEWORK_H
#define BENCHMARK_FRAMEWORK_H

#include <chrono>
#include <vector>
#include <string>
#include <iostream>
#include <iomanip>
#include <numeric>
#include <algorithm>
#include <functional>
#include <fstream>
#include <ctime>
#include <sstream>

class BenchmarkFramework {
public:
    struct BenchmarkResult {
        std::string name;
        std::vector<double> times_ms;
        double min_ms;
        double max_ms;
        double mean_ms;
        double median_ms;
        double stddev_ms;
    };

    static BenchmarkFramework& instance() {
        static BenchmarkFramework bf;
        return bf;
    }

    void run(const std::string& name, std::function<void()> func, int iterations = 1000) {
        std::vector<double> times;
        
        for (int i = 0; i < iterations; ++i) {
            auto start = std::chrono::high_resolution_clock::now();
            func();
            auto end = std::chrono::high_resolution_clock::now();
            
            double duration_ms = std::chrono::duration<double, std::milli>(end - start).count();
            times.push_back(duration_ms);
        }

        BenchmarkResult result = compute_statistics(name, times);
        results.push_back(result);
        
        print_result(result);
    }

    void print_all_results() {
        std::cout << "\n================================================================\n";
        std::cout << "              BENCHMARK RESULTS\n";
        std::cout << "================================================================\n\n";
        
        for (const auto& result : results) {
            print_result(result);
        }
    }

    const std::vector<BenchmarkResult>& get_results() const {
        return results;
    }

    void export_json(const std::string& filename = "benchmark_results.json") {
        std::ofstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Failed to open file: " << filename << std::endl;
            return;
        }

        // Get current timestamp
        auto now = std::time(nullptr);
        auto tm = *std::localtime(&now);
        std::ostringstream oss;
        oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
        std::string timestamp = oss.str();

        file << "{\n";
        file << "  \"timestamp\": \"" << timestamp << "\",\n";
        file << "  \"benchmarks\": [\n";

        for (size_t i = 0; i < results.size(); ++i) {
            const auto& result = results[i];
            file << "    {\n";
            file << "      \"name\": \"" << escape_json(result.name) << "\",\n";
            file << "      \"min_ms\": " << std::fixed << std::setprecision(6) << result.min_ms << ",\n";
            file << "      \"max_ms\": " << std::fixed << std::setprecision(6) << result.max_ms << ",\n";
            file << "      \"mean_ms\": " << std::fixed << std::setprecision(6) << result.mean_ms << ",\n";
            file << "      \"median_ms\": " << std::fixed << std::setprecision(6) << result.median_ms << ",\n";
            file << "      \"stddev_ms\": " << std::fixed << std::setprecision(6) << result.stddev_ms << ",\n";
            file << "      \"iterations\": " << result.times_ms.size() << "\n";
            file << "    }";
            if (i < results.size() - 1) file << ",";
            file << "\n";
        }

        file << "  ]\n";
        file << "}\n";
        file.close();

        std::cout << "\n✓ Benchmark results exported to: " << filename << "\n";
    }

private:
    std::string escape_json(const std::string& str) {
        std::string result;
        for (char c : str) {
            switch (c) {
                case '"': result += "\\\""; break;
                case '\\': result += "\\\\"; break;
                case '\b': result += "\\b"; break;
                case '\f': result += "\\f"; break;
                case '\n': result += "\\n"; break;
                case '\r': result += "\\r"; break;
                case '\t': result += "\\t"; break;
                default: result += c;
            }
        }
        return result;
    }
    std::vector<BenchmarkResult> results;

    BenchmarkResult compute_statistics(const std::string& name, const std::vector<double>& times) {
        BenchmarkResult result;
        result.name = name;
        result.times_ms = times;

        result.min_ms = *std::min_element(times.begin(), times.end());
        result.max_ms = *std::max_element(times.begin(), times.end());
        result.mean_ms = std::accumulate(times.begin(), times.end(), 0.0) / times.size();

        std::vector<double> sorted_times = times;
        std::sort(sorted_times.begin(), sorted_times.end());
        if (sorted_times.size() % 2 == 0) {
            result.median_ms = (sorted_times[sorted_times.size() / 2 - 1] + sorted_times[sorted_times.size() / 2]) / 2.0;
        } else {
            result.median_ms = sorted_times[sorted_times.size() / 2];
        }

        double variance = 0.0;
        for (double t : times) {
            variance += (t - result.mean_ms) * (t - result.mean_ms);
        }
        variance /= times.size();
        result.stddev_ms = std::sqrt(variance);

        return result;
    }

    void print_result(const BenchmarkResult& result) {
        std::cout << std::left << std::setw(40) << result.name << "\n";
        std::cout << "  Min:    " << std::fixed << std::setprecision(6) << result.min_ms << " ms\n";
        std::cout << "  Max:    " << std::fixed << std::setprecision(6) << result.max_ms << " ms\n";
        std::cout << "  Mean:   " << std::fixed << std::setprecision(6) << result.mean_ms << " ms\n";
        std::cout << "  Median: " << std::fixed << std::setprecision(6) << result.median_ms << " ms\n";
        std::cout << "  StdDev: " << std::fixed << std::setprecision(6) << result.stddev_ms << " ms\n";
        std::cout << "  Iter:   " << result.times_ms.size() << "\n\n";
    }
};

#define BENCHMARK(name, func, iters) \
    BenchmarkFramework::instance().run(name, func, iters)

#endif