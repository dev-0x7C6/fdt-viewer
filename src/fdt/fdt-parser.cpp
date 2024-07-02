#include "fdt-parser.hpp"
#include "fdt/fdt-header.hpp"
#include "fdt/fdt-parser-tokens.hpp"
#include "fdt-parser-context.hpp"
#include "endian-conversions.hpp"

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <expected>
#include <iostream>
#include <string_view>
#include <variant>

#include <iostream>

auto align(const std::size_t size) {
    const auto q = size % sizeof(u32);
    const auto w = size / sizeof(u32);
    return w + (q ? 1u : 0u);
};

namespace fdt::parser {

auto parse(token_types::node_begin &&token, context &ctx) -> fdt::parser::token {
    const auto size = std::strlen(ctx.state.data);
    token.name = std::string_view(ctx.state.data, size);
    ctx.state.skip += align(size + 1);
    return {std::move(token)};
}

auto parse(token_types::node_end &&token, context &) -> fdt::parser::token {
    return {std::move(token)};
}

auto parse(token_types::property &&token, context &ctx) -> fdt::parser::token {
    const auto header = read_data_32be<decode::property>(ctx.state.data);
    ctx.state.skip += align(sizeof(header)) + align(header.len);
    ctx.state.data += sizeof(header);

    const auto property = ctx.strings.data() + header.nameoff;

    token = {
        .name = std::string_view(property, std::strlen(property)),
        .data = std::string_view(ctx.state.data, header.len),
    };

    return {std::move(token)};
}

auto parse(token_types::nop &&token, context &) -> fdt::parser::token {
    return {std::move(token)};
}

auto parse(token_types::end &&token, context &) -> fdt::parser::token {
    return {std::move(token)};
}
} // namespace fdt::parser

template <fdt::parser::Tokenizable... Ts>
auto foreach_token_type(std::variant<Ts...>, const u32 token_id, fdt::parser::context &ctx) {
    auto conditional_parse = [&](auto &&token) {
        if (fdt::parser::token_types::id_of(token) == token_id) {
            ctx.tokens.emplace_back(fdt::parser::parse(std::move(token), ctx));
            return true;
        }
        return false;
    };
    return (conditional_parse(Ts{}) || ...);
}

auto fdt::parser::parse(std::string_view view, std::string_view root_name) -> std::expected<fdt::parser::tokens, fdt::parser::error> {
    using error = fdt::parser::error;

    if (view.size() < sizeof(decode::header))
        return std::unexpected(error::invalid_header);

    const auto header = read_data_32be<decode::header>(view.data());

    if (decode::is_magic_invalid(header))
        return std::unexpected(error::invalid_magic);

    if (view.size() < header.totalsize)
        return std::unexpected(error::data_truncated);

    if (decode::is_version_unsupported(header))
        return std::unexpected(error::unsupported_version);

    const auto dt_struct = view.data() + header.off_dt_struct;
    const auto dt_strings = view.data() + header.off_dt_strings;

    fdt::parser::tokens tokens;
    fdt::parser::context ctx{
        .structs = {dt_struct, header.size_dt_struct},   //
        .strings = {dt_strings, header.size_dt_strings}, //
        .tokens = tokens,
    };

    tokens.reserve(50000);

    const auto begin = reinterpret_cast<const u32 *>(dt_struct);
    const auto end = reinterpret_cast<const u32 *>(dt_struct) + header.size_dt_struct / sizeof(u32);

    if (header.size_dt_struct % sizeof(u32) != 0)
        return std::unexpected(error::data_unaligned);

    for (auto iter = begin; iter != end;) {
        const auto id = static_cast<u32>(convert(*iter));
        ctx.state.data = reinterpret_cast<const char *>(++iter);
        ctx.state.skip = 0;

        if (!foreach_token_type(token{}, id, ctx))
            return std::unexpected(error::invalid_token);

        iter += ctx.state.skip;

        if (std::holds_alternative<token_types::property>(tokens.back())) {
            auto &prop = std::get<token_types::property>(tokens.back());

            if (auto dtb = fdt::parser::parse(prop.data, prop.name); dtb.has_value()) {
                auto &embedded_tokens = dtb.value();
                std::move(std::begin(embedded_tokens), std::end(embedded_tokens), std::back_inserter(tokens));
            }
        }
    }

    // change property name for first node
    for (auto &&token : tokens)
        if (std::holds_alternative<token_types::node_begin>(token)) {
            std::get<token_types::node_begin>(token).name = root_name;
            break;
        }

    return tokens;
}

template <class... Ts>
struct overloaded : Ts... {
    using Ts::operator()...;
};

auto fdt::parser::validate(const fdt::parser::tokens &tokens) -> bool {
    using namespace fdt::parser;

    bool valid_depth_test{true};
    std::int32_t node_scope_depth{};
    std::int32_t node_begin_count{};
    std::int32_t node_end_count{};
    std::int32_t property_count{};
    std::int32_t nop_count{};
    std::int32_t end_count{};

    for (auto &&token : tokens) {
        std::visit(overloaded{
                       [&](const token_types::node_begin &) {
                           node_begin_count++;
                           node_scope_depth++;
                       },
                       [&](const token_types::node_end &) {
                           node_end_count++;
                           node_scope_depth--;
                       },
                       [&](const token_types::property &) {
                           property_count++;
                       },
                       [&](const token_types::nop &) { nop_count++; },
                       [&](const token_types::end &) { end_count++; },
                   },
            token);

        // check that we never go below 0
        valid_depth_test &= (node_scope_depth >= 0);
    }

    std::cout << "node depth validation: " << valid_depth_test << std::endl;
    std::cout << "node begin: " << node_begin_count << std::endl;
    std::cout << "node end  : " << node_end_count << std::endl;
    std::cout << "property  : " << property_count << std::endl;
    std::cout << "nop       : " << nop_count << std::endl;
    std::cout << "end       : " << end_count << std::endl;

    return valid_depth_test && node_begin_count == node_end_count;
}
