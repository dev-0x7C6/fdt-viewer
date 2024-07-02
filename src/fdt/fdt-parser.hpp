#pragma once

#include "fdt/fdt-header.hpp"
#include "fdt/fdt-parser-v2.hpp"

#include <optional>
#include <string>
#include <string_view>
#include <expected>

namespace fdt {

enum class error {
    bad_header,
    bad_magic,
    data_truncated,
    data_unaligned,
    not_supported_version,
    bad_token,
};

namespace tokenizer {
auto generator(std::string_view view, std::string_view root_name) -> std::expected<fdt::tokenizer::token_list, error>;
auto validate(const fdt::tokenizer::token_list &tokens) -> bool;
} // namespace tokenizer
} // namespace fdt
