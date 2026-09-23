#include <print>

#include <demix/demix.hpp>

namespace demix {
    bool is_ext(const fs::path& path, std::string_view ext) {
        const auto src = path.extension().string();
        return src.size() == ext.size() && std::ranges::equal(src, ext, [](const char ca, const char cb) {
            return std::tolower(ca) == std::tolower(cb);
        });
    }

    void help() {
        std::println("Version: {}\n\nUsage:\n", VERSION);
        std::println("{} --infer <cache_dir> <device> <input_dir> <output_dir>", NAME);
        std::println("{} --cache <model_dir> <device> <cache_dir> ", NAME);
    }
}
