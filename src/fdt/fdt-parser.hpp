#pragma once

#include "fdt/fdt-parser-v2.hpp"

#include <string_view>
#include <expected>

namespace fdt {

enum class error {
    invalid_header,
    invalid_magic,
    invalid_token,
    data_truncated,
    data_unaligned,
    unsupported_version,
};

namespace tokenizer {
auto generator(std::string_view view, std::string_view root_name) -> std::expected<fdt::tokenizer::token_list, error>;
auto validate(const fdt::tokenizer::token_list &tokens) -> bool;
} // namespace tokenizer
} // namespace fdt
