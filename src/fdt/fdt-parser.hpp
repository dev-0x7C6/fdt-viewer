#pragma once

#include "fdt/fdt-header.hpp"
#include "fdt/fdt-parser-tokens.hpp"

#include <string_view>
#include <expected>
#include <vector>
#include <cstdint>

namespace fdt::parser {

enum class error {
    invalid_header,
    invalid_magic,
    invalid_token,
    data_truncated,
    data_unaligned,
    unsupported_version,
};

struct result {
    std::uint64_t offset{};
    fdt::decode::header header;
    fdt::parser::tokens tokens;
};

auto parse(std::string_view data) -> std::expected<result, error>;
auto parse_multiple_offsets(std::string_view data) -> std::vector<std::expected<result, error>>;
auto validate(const result &) -> bool;
auto validate(const tokens &) -> bool;
auto rename_root(tokens &, std::string_view name) -> bool;

} // namespace fdt::parser
