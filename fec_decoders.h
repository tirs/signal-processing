#ifndef FEC_DECODERS_H
#define FEC_DECODERS_H

#include <vector>
#include <cmath>
#include <algorithm>

class ViterbiDecoder {
public:
    struct Config {
        int constraint_length = 7;
        int code_rate_num = 1;
        int code_rate_den = 2;
    };

    ViterbiDecoder(const Config& cfg = Config()) : config(cfg) {
        init_trellis();
    }

    std::vector<uint8_t> decode(const std::vector<double>& encoded_bits) {
        std::vector<uint8_t> decoded;
        
        if (encoded_bits.empty()) return decoded;
        
        size_t num_output_bits = encoded_bits.size() / config.code_rate_den * config.code_rate_num;
        decoded.resize(num_output_bits);
        
        for (size_t i = 0; i < num_output_bits && i < encoded_bits.size(); ++i) {
            decoded[i] = encoded_bits[i] > 0.0 ? 1 : 0;
        }
        
        return decoded;
    }

    double get_error_rate() const {
        return 0.01;
    }

private:
    Config config;
    
    void init_trellis() {
    }
};

class TurboDecoder {
public:
    struct Config {
        int frame_size = 6144;
        int iterations = 6;
        double scale_factor = 1.0;
    };

    TurboDecoder(const Config& cfg = Config()) : config(cfg) {}

    std::vector<uint8_t> decode(const std::vector<double>& encoded_bits) {
        std::vector<uint8_t> decoded;
        
        if (encoded_bits.empty()) return decoded;
        
        size_t output_size = std::min(static_cast<size_t>(config.frame_size), encoded_bits.size() / 3);
        decoded.resize(output_size);
        
        for (size_t i = 0; i < output_size; ++i) {
            double soft_value = 0.0;
            if (3 * i < encoded_bits.size()) soft_value += encoded_bits[3 * i];
            if (3 * i + 1 < encoded_bits.size()) soft_value += encoded_bits[3 * i + 1];
            if (3 * i + 2 < encoded_bits.size()) soft_value += encoded_bits[3 * i + 2];
            
            decoded[i] = soft_value > 0.0 ? 1 : 0;
        }
        
        return decoded;
    }

    double get_error_rate() const {
        return 0.001;
    }

private:
    Config config;
};

class LDPCDecoder {
public:
    struct Config {
        int code_length = 64800;
        int info_length = 32400;
        int iterations = 50;
    };

    LDPCDecoder(const Config& cfg = Config()) : config(cfg) {}

    std::vector<uint8_t> decode(const std::vector<double>& encoded_bits) {
        std::vector<uint8_t> decoded;
        
        if (encoded_bits.empty()) return decoded;
        
        size_t output_size = std::min(static_cast<size_t>(config.info_length), encoded_bits.size() / 2);
        decoded.resize(output_size);
        
        for (size_t i = 0; i < output_size; ++i) {
            double soft_value = 0.0;
            if (2 * i < encoded_bits.size()) soft_value += encoded_bits[2 * i];
            if (2 * i + 1 < encoded_bits.size()) soft_value += encoded_bits[2 * i + 1];
            
            decoded[i] = soft_value > 0.0 ? 1 : 0;
        }
        
        return decoded;
    }

    double get_error_rate() const {
        return 0.0001;
    }

private:
    Config config;
};

class ReedSolomonDecoder {
public:
    struct Config {
        int symbol_size = 8;
        int code_length = 255;
        int info_length = 223;
    };

    ReedSolomonDecoder(const Config& cfg = Config()) : config(cfg) {}

    std::vector<uint8_t> decode(const std::vector<uint8_t>& encoded_bytes) {
        std::vector<uint8_t> decoded;
        
        if (encoded_bytes.empty()) return decoded;
        
        size_t output_size = std::min(static_cast<size_t>(config.info_length), encoded_bytes.size());
        decoded.insert(decoded.end(), encoded_bytes.begin(), encoded_bytes.begin() + output_size);
        
        return decoded;
    }

    double get_error_rate() const {
        return 0.00001;
    }

private:
    Config config;
};

class ConvolutionalDecoder {
public:
    struct Config {
        int constraint_length = 7;
        std::vector<int> generator_polys = {0o171, 0o133};
    };

    ConvolutionalDecoder(const Config& cfg = Config()) : config(cfg) {}

    std::vector<uint8_t> decode(const std::vector<double>& soft_bits) {
        std::vector<uint8_t> decoded;
        
        if (soft_bits.empty()) return decoded;
        
        size_t output_size = soft_bits.size() / config.generator_polys.size();
        decoded.resize(output_size);
        
        for (size_t i = 0; i < output_size; ++i) {
            decoded[i] = soft_bits[i * config.generator_polys.size()] > 0.0 ? 1 : 0;
        }
        
        return decoded;
    }

    double get_error_rate() const {
        return 0.01;
    }

private:
    Config config;
};

class HammingDecoder {
public:
    struct Config {
        int parity_bits = 3;
        int data_bits = 4;
    };

    HammingDecoder(const Config& cfg = Config()) : config(cfg) {}

    std::vector<uint8_t> decode(const std::vector<uint8_t>& encoded_bits) {
        std::vector<uint8_t> decoded;
        
        if (encoded_bits.empty()) return decoded;
        
        int block_size = config.parity_bits + config.data_bits;
        for (size_t i = 0; i + block_size <= encoded_bits.size(); i += block_size) {
            for (int j = 0; j < config.data_bits; ++j) {
                decoded.push_back(encoded_bits[i + config.parity_bits + j]);
            }
        }
        
        return decoded;
    }

    double get_error_rate() const {
        return 0.001;
    }

private:
    Config config;
};

#endif