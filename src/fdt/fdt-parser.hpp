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
    invalid_structure,
    data_truncated,
    data_unaligned,
    unsupported_version,
};

struct status {
    std::int64_t node_scope_depth{};
    std::int64_t node_begin_count{};
    std::int64_t node_end_count{};
    std::int64_t property_count{};
    std::int64_t nop_count{};
    std::int64_t end_count{};
};

struct result {
    std::uint64_t offset{};
    fdt::decode::header header;
    fdt::parser::tokens tokens;
    fdt::parser::status status;
};

auto parse(std::string_view data) -> std::expected<result, error>;
auto parse_multiple_offsets(std::string_view data) -> std::vector<std::expected<result, error>>;
auto rename_root(tokens &, std::string_view name) -> bool;

} // namespace fdt::parser
