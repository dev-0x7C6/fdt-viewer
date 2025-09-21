#include "fdt-parser.hpp"
#include "fdt/fdt-header.hpp"
#include "fdt/fdt-parser-tokens.hpp"
#include "fdt-parser-context.hpp"
#include "endian-conversions.hpp"

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <expected>
#include <string_view>
#include <variant>

auto align(const std::size_t size) {
    const auto q = size % sizeof(u32);
    const auto w = size / sizeof(u32);
    return w + (q ? 1u : 0u);
};

template <class... Ts>
struct overloaded : Ts... {
    using Ts::operator()...;
};

auto validate(const fdt::parser::tokens &tokens) -> std::expected<fdt::parser::status, fdt::parser::error> {
    fdt::parser::status ret{};
    bool valid_depth_test = true;

    using namespace fdt::parser;

    for (auto &&token : tokens) {
        std::visit(overloaded{
                       [&](const token_types::node_begin &) {
                           ret.node_begin_count++;
                           ret.node_scope_depth++;
                       },
                       [&](const token_types::node_end &) {
                           ret.node_end_count++;
                           ret.node_scope_depth--;
                       },
                       [&](const token_types::property &) {
                           ret.property_count++;
                       },
                       [&](const token_types::nop &) { ret.nop_count++; },
                       [&](const token_types::end &) { ret.end_count++; },
                   },
            token);

        // check that we never go below 0
        valid_depth_test &= (ret.node_scope_depth >= 0);
    }

    if (!valid_depth_test)
        return std::unexpected(fdt::parser::error::invalid_structure);

    if (ret.node_begin_count != ret.node_end_count)
        return std::unexpected(fdt::parser::error::invalid_structure);

    return ret;
}

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
        const auto token_match = (fdt::parser::token_types::id_of(token) == token_id);

        if (token_match)
            ctx.tokens.emplace_back(fdt::parser::parse(std::move(token), ctx));

        return token_match;
    };
    return (conditional_parse(Ts{}) || ...);
}

auto fdt::parser::parse(std::string_view view) -> std::expected<fdt::parser::result, fdt::parser::error> {
    using error = fdt::parser::error;

    if (view.size() < sizeof(decode::header))
        return std::unexpected(error::invalid_header);

    const auto header = read_data_32be<decode::header>(view.data());

    if (decode::is_magic_invalid(header))
        return std::unexpected(error::invalid_magic);

    if (decode::is_data_truncated(header, view.size()))
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
        const auto id = static_cast<u32>(byteorder(*iter));
        ctx.state.data = reinterpret_cast<const char *>(++iter);
        ctx.state.skip = 0;

        if (!foreach_token_type(token{}, id, ctx))
            return std::unexpected(error::invalid_token);

        iter += ctx.state.skip;

        if (std::holds_alternative<token_types::property>(tokens.back())) {
            auto &prop = std::get<token_types::property>(tokens.back());

            if (auto inner_dtb = fdt::parser::parse(prop.data); inner_dtb.has_value()) {
                auto &&inner_tokens = inner_dtb.value().tokens;

                rename_root(tokens, prop.name);
                std::move(std::begin(inner_tokens), std::end(inner_tokens), std::back_inserter(tokens));
            }
        }
    }

    const auto status = validate(tokens);

    if (!status)
        return std::unexpected(status.error());

    return fdt::parser::result{
        .header = std::move(header),
        .tokens = std::move(tokens),
        .status = status.value(),
    };
}

auto fdt::parser::parse_multiple_offsets(std::string_view view) -> std::vector<std::expected<fdt::parser::result, fdt::parser::error>> {
    std::vector<std::expected<fdt::parser::result, fdt::parser::error>> ret;

    using namespace fdt::decode;

    auto reference = view;

    while (view.size() >= sizeof(decode::header)) {
        const auto magic = byteorder(*reinterpret_cast<const std::uint32_t *>(view.data()));

        if (is_magic_valid(magic))
            if (auto dtb = fdt::parser::parse(view); dtb.has_value()) {
                dtb->offset = std::distance(reference.begin(), view.begin());
                view = std::string_view(view.begin() + dtb.value().header.totalsize, view.end());
                ret.emplace_back(std::move(dtb));
                continue;
            }

        view = std::string_view(view.begin() + 1, view.end());
    }

    return ret;
}

auto fdt::parser::rename_root(fdt::parser::tokens &tokens, std::string_view name) -> bool {
    for (auto &&token : tokens)
        if (std::holds_alternative<token_types::node_begin>(token)) {
            std::get<token_types::node_begin>(token).name = name;
            return true;
        }

    return false;
}
