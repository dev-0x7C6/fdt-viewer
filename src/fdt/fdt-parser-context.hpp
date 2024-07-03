
#pragma once

#include "fdt/fdt-parser-tokens.hpp"

namespace fdt::parser {

struct context {
    std::string_view structs;
    std::string_view strings;
    fdt::parser::tokens &tokens;

    struct {
        const char *data{nullptr};
        u32 skip{};
    } state;
};

} // namespace fdt::parser
