#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

using u32 = std::uint32_t;

namespace fdt::parser::token_types {

struct node_begin {
    std::string name;
};
struct node_end {};
struct property {
    std::string_view name;
    std::string_view data;
};
struct nop {};
struct end {};

constexpr auto id_of(node_begin) -> u32 { return 0x01; };
constexpr auto id_of(node_end) -> u32 { return 0x02; };
constexpr auto id_of(property) -> u32 { return 0x03; };
constexpr auto id_of(nop) -> u32 { return 0x04; };
constexpr auto id_of(end) -> u32 { return 0x09; };

} // namespace fdt::parser::token_types

namespace fdt::parser {

using token = std::variant<
    token_types::property,   //
    token_types::node_begin, //
    token_types::node_end,   //
    token_types::nop,        //
    token_types::end         //
    >;

using tokens = std::vector<token>;

template <typename T>
concept Tokenizable = requires(T t) {
    { fdt::parser::token_types::id_of(t) } -> std::convertible_to<u32>;
};

} // namespace fdt::parser
