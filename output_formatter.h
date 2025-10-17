#ifndef OUTPUT_FORMATTER_H
#define OUTPUT_FORMATTER_H

#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <cmath>

class OutputFormatter {
public:
    enum class Format {
        TEXT,
        JSON,
        CSV,
        XML
    };

    struct Metrics {
        std::map<std::string, double> values;
        std::map<std::string, std::string> strings;
    };

    static std::string format_metrics(const Metrics& metrics, Format fmt) {
        switch (fmt) {
            case Format::JSON:
                return format_json(metrics);
            case Format::CSV:
                return format_csv(metrics);
            case Format::XML:
                return format_xml(metrics);
            case Format::TEXT:
            default:
                return format_text(metrics);
        }
    }

    static void write_to_file(const std::string& filename, const std::string& content) {
        std::ofstream file(filename, std::ios::app);
        if (file.is_open()) {
            file << content;
            file.close();
        }
    }

private:
    static std::string format_text(const Metrics& metrics) {
        std::stringstream ss;
        ss << "\nMetrics Report\n";
        ss << "==============\n\n";

        for (const auto& pair : metrics.values) {
            ss << std::left << std::setw(30) << pair.first << ": "
               << std::fixed << std::setprecision(6) << pair.second << "\n";
        }

        for (const auto& pair : metrics.strings) {
            ss << std::left << std::setw(30) << pair.first << ": "
               << pair.second << "\n";
        }

        ss << "\n";
        return ss.str();
    }

    static std::string format_json(const Metrics& metrics) {
        std::stringstream ss;
        ss << "{\n";

        bool first = true;
        for (const auto& pair : metrics.values) {
            if (!first) ss << ",\n";
            ss << "  \"" << escape_json(pair.first) << "\": "
               << std::fixed << std::setprecision(6) << pair.second;
            first = false;
        }

        for (const auto& pair : metrics.strings) {
            if (!first) ss << ",\n";
            ss << "  \"" << escape_json(pair.first) << "\": \""
               << escape_json(pair.second) << "\"";
            first = false;
        }

        ss << "\n}\n";
        return ss.str();
    }

    static std::string format_csv(const Metrics& metrics) {
        std::stringstream ss;
        
        for (const auto& pair : metrics.values) {
            ss << escape_csv(pair.first) << ","
               << std::fixed << std::setprecision(6) << pair.second << "\n";
        }

        for (const auto& pair : metrics.strings) {
            ss << escape_csv(pair.first) << ","
               << escape_csv(pair.second) << "\n";
        }

        return ss.str();
    }

    static std::string format_xml(const Metrics& metrics) {
        std::stringstream ss;
        ss << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
        ss << "<metrics>\n";

        for (const auto& pair : metrics.values) {
            ss << "  <metric name=\"" << escape_xml(pair.first) << "\" type=\"double\">"
               << std::fixed << std::setprecision(6) << pair.second
               << "</metric>\n";
        }

        for (const auto& pair : metrics.strings) {
            ss << "  <metric name=\"" << escape_xml(pair.first) << "\" type=\"string\">"
               << escape_xml(pair.second)
               << "</metric>\n";
        }

        ss << "</metrics>\n";
        return ss.str();
    }

    static std::string escape_json(const std::string& str) {
        std::stringstream ss;
        for (char c : str) {
            switch (c) {
                case '"':  ss << "\\\""; break;
                case '\\': ss << "\\\\"; break;
                case '\b': ss << "\\b";  break;
                case '\f': ss << "\\f";  break;
                case '\n': ss << "\\n";  break;
                case '\r': ss << "\\r";  break;
                case '\t': ss << "\\t";  break;
                default:   ss << c;      break;
            }
        }
        return ss.str();
    }

    static std::string escape_csv(const std::string& str) {
        if (str.find(',') != std::string::npos || str.find('\"') != std::string::npos) {
            std::string escaped;
            for (char c : str) {
                if (c == '"') escaped += "\"\"";
                else escaped += c;
            }
            return "\"" + escaped + "\"";
        }
        return str;
    }

    static std::string escape_xml(const std::string& str) {
        std::stringstream ss;
        for (char c : str) {
            switch (c) {
                case '&':  ss << "&amp;";  break;
                case '<':  ss << "&lt;";   break;
                case '>':  ss << "&gt;";   break;
                case '"':  ss << "&quot;"; break;
                case '\'': ss << "&apos;"; break;
                default:   ss << c;        break;
            }
        }
        return ss.str();
    }
};

#endif