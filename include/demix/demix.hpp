#pragma once

#include <demix/model.hpp>
#include <demix/audio.hpp>

namespace demix {
    constexpr std::string VERSION = "1.0.0";
    constexpr std::string NAME = "demix";

    bool is_ext(const fs::path& path, std::string_view ext);
    void help();
}