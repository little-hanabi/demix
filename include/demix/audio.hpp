#pragma once

#include <memory>
#include <filesystem>
#include <span>

#include <fftw3.h>

namespace fs = std::filesystem;

namespace demix {
    class Audio {
        bool good = false;
        std::unique_ptr<fftwf_complex[], decltype(&fftwf_free)> spec{nullptr, &fftwf_free};
        uint64_t L_ = 0;
        uint64_t T_ = 0;
        uint64_t N_ = 0;
        void stft(const std::vector<std::vector<float>>& wav, std::vector<float>& tsr) const;
        void mask(std::span<const float> tsr, std::vector<std::vector<float>>& wav) const;
    public:
        Audio(const fs::path& pth, std::vector<float>& tsr);
        void write(std::span<const float> tsr, const fs::path& pth) const;
        [[nodiscard]] uint64_t get_n() const { return good ? N_ : 0; }
        explicit operator bool() const { return good; }
    };
}
