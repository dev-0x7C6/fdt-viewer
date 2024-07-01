#pragma once

#include <fdt/fdt-header.hpp>

#include "fdt/fdt-parser-v2.hpp"
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include <fdt/fdt-generator.hpp>

class fdt_parser {
public:
    fdt_parser(std::string_view view, fdt::tokenizer::token_list &tokens,
        const std::string &default_root_node);
    constexpr bool is_valid() noexcept { return m_header.has_value(); }

private:
    void parse(const fdt::header header, fdt::tokenizer::token_list &tokens);

private:
    std::optional<fdt::header> m_header;
    const std::string default_root_node_name;

    const std::string_view view;
};
