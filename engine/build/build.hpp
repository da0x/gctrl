#pragma once

#include "ui/terminal.hpp"
#include <filesystem>

namespace build {

    void create_build_directory() {
        const std::string build_dir = "build";
        if (!std::filesystem::exists(build_dir)) {
            std::filesystem::create_directory(build_dir);
        }
    }

}