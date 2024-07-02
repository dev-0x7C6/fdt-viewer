#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

using u32 = std::uint32_t;

namespace fdt::tokenizer {

namespace types {
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

} // namespace types

using token = std::variant<types::property, types::node_begin, types::node_end, types::nop, types::end>;
using token_list = std::vector<token>;

struct context {
    std::string_view structs;
    std::string_view strings;
    token_list &tokens;

    struct {
        const char *data{nullptr};
        int skip{};
    } state;
};

template <typename T>
concept Tokenizable = requires(T t) {
    { fdt::tokenizer::types::id_of(t) } -> std::convertible_to<u32>;
};

} // namespace fdt::tokenizer
