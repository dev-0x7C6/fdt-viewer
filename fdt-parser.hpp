#pragma once

#include <fdt-header.hpp>

#include <functional>
#include <optional>
#include <string>
#include <string_view>

struct fdt_generator {
    std::function<void(std::string_view &&view)> begin_node{[](auto...) {}};
    std::function<void()> end_node{[](auto...) {}};
    std::function<void(std::string_view &&name, std::string_view &&data)> insert_property{[](auto...) {}};
};

class fdt_parser {
public:
    fdt_parser(const char *data, u64 size, fdt_generator &generator);
    constexpr bool is_valid() noexcept { return m_header.has_value(); }

private:
    void parse(const fdt_header header, fdt_generator &generator);

private:
    std::optional<fdt_header> m_header;
    std::string_view m_dt_struct;
    std::string_view m_dt_strings;

    const char *const m_data;
    const u64 m_size;
};
