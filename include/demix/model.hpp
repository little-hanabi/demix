#pragma once

#include <span>
#include <string>
#include <filesystem>

#include <openvino/openvino.hpp>

namespace fs = std::filesystem;

namespace demix {
    class Model {
        ov::Core core;
        std::array<ov::InferRequest, 4> sess;
        bool good = false;
    public:
        Model(const fs::path& dir, const std::string& device);
        void infer(std::span<const float> src, uint64_t num, std::vector<float>& dst);
        static void cache(const fs::path& dir_i, const fs::path& dir_o, const std::string& device);
        explicit operator bool() const { return good; }
    };
}
