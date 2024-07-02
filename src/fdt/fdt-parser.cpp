#include "fdt-parser.hpp"
#include "fdt/fdt-header.hpp"
#include "fdt-parser-v2.hpp"
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

using namespace fdt::tokenizer;
using namespace fdt::tokenizer::types;

auto parse(node_begin &&token, context &ctx) -> fdt::tokenizer::token {
    const auto size = std::strlen(ctx.state.data);
    token.name = std::string_view(ctx.state.data, size);
    ctx.state.skip += align(size + 1);
    return {std::move(token)};
}

auto parse(node_end &&token, context &ctx) -> fdt::tokenizer::token {
    return {std::move(token)};
}

auto parse(types::property &&token, context &ctx) -> fdt::tokenizer::token {
    const auto header = read_data_32be<fdt::property>(ctx.state.data);
    ctx.state.skip += align(sizeof(header)) + align(header.len);
    ctx.state.data += sizeof(header);

    const auto property = ctx.strings.data() + header.nameoff;

    token = {
        .name = std::string_view(property, std::strlen(property)),
        .data = std::string_view(ctx.state.data, header.len),
    };

    return {std::move(token)};
}

auto parse(nop &&token, context &ctx) -> fdt::tokenizer::token {
    return {std::move(token)};
}

auto parse(end &&token, context &ctx) -> fdt::tokenizer::token {
    return {std::move(token)};
}
} // namespace fdt::parser

template <fdt::tokenizer::Tokenizable... Ts>
auto foreach_token_type(std::variant<Ts...>, const u32 token_id, fdt::tokenizer::context &ctx) {
    auto conditional_parse = [&](auto &&token) {
        if (fdt::tokenizer::types::id_of(token) == token_id) {
            ctx.tokens.emplace_back(fdt::parser::parse(std::move(token), ctx));
            return true;
        }
        return false;
    };
    return (conditional_parse(Ts{}) || ...);
}

auto fdt::tokenizer::generator(std::string_view view, std::string_view root_name) -> std::expected<fdt::tokenizer::token_list, error> {
    if (view.size() < sizeof(fdt::header))
        return std::unexpected(fdt::error::invalid_header);

    const auto header = read_data_32be<fdt::header>(view.data());

    if (fdt::is_magic_invalid(header))
        return std::unexpected(fdt::error::invalid_magic);

    if (view.size() < header.totalsize)
        return std::unexpected(fdt::error::data_truncated);

    if (fdt::is_version_unsupported(header))
        return std::unexpected(fdt::error::unsupported_version);

    const auto dt_struct = view.data() + header.off_dt_struct;
    const auto dt_strings = view.data() + header.off_dt_strings;

    fdt::tokenizer::token_list tokens;
    tokens.reserve(50000);

    using namespace fdt::tokenizer;
    using namespace fdt::tokenizer::types;

    context ctx{
        .structs = {dt_struct, header.size_dt_struct}, //
        .strings = {dt_strings, header.size_dt_strings}, //
        .tokens = tokens,
    };

    const auto begin = reinterpret_cast<const u32 *>(dt_struct);
    const auto end = reinterpret_cast<const u32 *>(dt_struct) + header.size_dt_struct / sizeof(u32);

    if (header.size_dt_struct % sizeof(u32) != 0)
        return std::unexpected(fdt::error::data_unaligned);

    for (auto iter = begin; iter != end;) {
        const auto id = static_cast<u32>(convert(*iter));
        ctx.state.data = reinterpret_cast<const char *>(++iter);
        ctx.state.skip = 0;

        if (!foreach_token_type(token{}, id, ctx))
            return std::unexpected(fdt::error::invalid_token);

        iter += ctx.state.skip;

        if (std::holds_alternative<types::property>(tokens.back())) {
            auto &prop = std::get<types::property>(tokens.back());

            if (auto dtb = fdt::tokenizer::generator(prop.data, prop.name); dtb.has_value()) {
                auto &embedded_tokens = dtb.value();
                std::move(std::begin(embedded_tokens), std::end(embedded_tokens), std::back_inserter(tokens));
            }
        }
    }

    // change property name for first node
    for (auto &&token : tokens)
        if (std::holds_alternative<types::node_begin>(token)) {
            std::get<types::node_begin>(token).name = root_name;
            break;
        }

    return tokens;
}

template <class... Ts>
struct overloaded : Ts... {
    using Ts::operator()...;
};

auto fdt::tokenizer::validate(const fdt::tokenizer::token_list &tokens) -> bool {
    using namespace fdt::tokenizer;

    bool valid_depth_test{true};
    std::int32_t node_scope_depth{};
    std::int32_t node_begin_count{};
    std::int32_t node_end_count{};
    std::int32_t property_count{};
    std::int32_t nop_count{};
    std::int32_t end_count{};

    for (auto &&token : tokens) {
        std::visit(overloaded{
                       [&](const types::node_begin &arg) {
                           node_begin_count++;
                           node_scope_depth++;
                       },
                       [&](const types::node_end &arg) {
                           node_end_count++;
                           node_scope_depth--;
                       },
                       [&](const types::property &arg) {
                           property_count++;
                       },
                       [&](const types::nop &) { nop_count++; },
                       [&](const types::end &) { end_count++; },
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
