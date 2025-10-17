#ifndef TEST_FRAMEWORK_H
#define TEST_FRAMEWORK_H

#include <iostream>
#include <vector>
#include <string>
#include <functional>
#include <chrono>
#include <iomanip>
#include <cmath>

class TestFramework {
public:
    struct TestResult {
        std::string name;
        bool passed;
        std::string error_message;
        double duration_ms;
    };

    static TestFramework& instance() {
        static TestFramework tf;
        return tf;
    }

    void register_test(const std::string& name, std::function<void()> test_func) {
        tests.push_back({name, test_func});
    }

    int run_all_tests() {
        std::cout << "\n======================================\n";
        std::cout << "Running Unit Tests\n";
        std::cout << "======================================\n\n";

        int passed = 0;
        int failed = 0;

        for (auto& test_info : tests) {
            auto start = std::chrono::high_resolution_clock::now();
            
            try {
                current_test_name = test_info.name;
                test_info.func();
                
                auto end = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() / 1000.0;
                
                results.push_back(TestResult{test_info.name, true, "", duration});
                std::cout << "[PASS] " << test_info.name << " (" << duration << " ms)\n";
                passed++;
            }
            catch (const std::exception& e) {
                auto end = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() / 1000.0;
                
                results.push_back(TestResult{test_info.name, false, e.what(), duration});
                std::cout << "[FAIL] " << test_info.name << " - " << e.what() << " (" << duration << " ms)\n";
                failed++;
            }
        }

        std::cout << "\n======================================\n";
        std::cout << "Test Results: " << passed << " passed, " << failed << " failed\n";
        std::cout << "======================================\n\n";

        return failed;
    }

    void assert_equal(double a, double b, double tolerance = 1e-6, const std::string& msg = "") {
        if (std::fabs(a - b) > tolerance) {
            std::string error = "Assertion failed: " + std::to_string(a) + " != " + std::to_string(b);
            if (!msg.empty()) error += " (" + msg + ")";
            throw std::runtime_error(error);
        }
    }

    void assert_true(bool condition, const std::string& msg = "") {
        if (!condition) {
            std::string error = "Assertion failed: condition is false";
            if (!msg.empty()) error += " (" + msg + ")";
            throw std::runtime_error(error);
        }
    }

    void assert_false(bool condition, const std::string& msg = "") {
        if (condition) {
            std::string error = "Assertion failed: condition is true";
            if (!msg.empty()) error += " (" + msg + ")";
            throw std::runtime_error(error);
        }
    }

    template<typename T>
    void assert_close(T a, T b, double tolerance = 0.01) {
        double diff = std::fabs(static_cast<double>(a - b));
        if (diff > tolerance) {
            throw std::runtime_error("Values are not close: " + std::to_string(diff));
        }
    }

    const std::vector<TestResult>& get_results() const {
        return results;
    }

private:
    struct TestInfo {
        std::string name;
        std::function<void()> func;
    };

    std::vector<TestInfo> tests;
    std::vector<TestResult> results;
    std::string current_test_name;
};

#define TEST(name) \
    void test_##name(); \
    namespace { struct Register##name { \
        Register##name() { \
            TestFramework::instance().register_test(#name, test_##name); \
        } \
    } register_##name; } \
    void test_##name()

#define ASSERT_EQ(a, b, tol) TestFramework::instance().assert_equal(a, b, tol)
#define ASSERT_TRUE(cond) TestFramework::instance().assert_true(cond)
#define ASSERT_FALSE(cond) TestFramework::instance().assert_false(cond)
#define ASSERT_CLOSE(a, b, tol) TestFramework::instance().assert_close(a, b, tol)

#endif