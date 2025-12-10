#pragma once

#include <string>
#include <memory>

namespace truk::core {

class core_c {
public:
    core_c();
    ~core_c();

    core_c(const core_c&) = delete;
    core_c& operator=(const core_c&) = delete;
    
    core_c(core_c&&) noexcept;
    core_c& operator=(core_c&&) noexcept;

    [[nodiscard]] std::string get_build_hash() const;
    
    [[nodiscard]] bool is_initialized() const;
    
    void initialize();
    void shutdown();

private:
    struct impl;
    std::unique_ptr<impl> pimpl_;
};

}
