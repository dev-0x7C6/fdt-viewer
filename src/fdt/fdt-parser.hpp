#pragma once

#include "fdt/fdt-parser-tokens.hpp"

#include <string_view>
#include <expected>

namespace fdt::parser {

enum class error {
    invalid_header,
    invalid_magic,
    invalid_token,
    data_truncated,
    data_unaligned,
    unsupported_version,
};

auto parse(std::string_view view, std::string_view root_name) -> std::expected<tokens, error>;
auto validate(const tokens &) -> bool;
} // namespace fdt::parser
