#include "fdt-parser.hpp"
#include "fdt/fdt-header.hpp"
#include "fdt-parser-v2.hpp"

#include <algorithm>
#include <cstring>
#include <iostream>
#include <ostream>
#include <string_view>
#include <variant>

#include <endian-conversions.hpp>

fdt_parser::fdt_parser(const char *data, u64 size, iface_fdt_generator &generator, const QString &default_root_node, const std::vector<fdt_handle_special_property> &handle_special_properties)
        : m_data(data)
        , m_size(size)
        , m_default_root_node(default_root_node)
        , m_handle_special_properties(handle_special_properties) {
    if (size >= sizeof(fdt::header)) {
        auto header = read_data_32be<fdt::header>(data);
        if (FDT_MAGIC_VALUE != header.magic)
            return;

        if (size < header.totalsize)
            return;

        if (FDT_SUPPORT_ABOVE > header.version)
            return;

        m_header = std::move(header);
        parse(m_header.value(), generator);
    }
}

/*
void fdt_parser::parse(const fdt::header header, iface_fdt_generator &generator) {
    const auto dt_struct = m_data + header.off_dt_struct;
    const auto dt_strings = m_data + header.off_dt_strings;

    auto get_property_name = [&](auto offset) {
        const auto ptr = dt_strings + offset;
        return QString::fromUtf8(ptr, std::strlen(ptr));
    };

    for (auto iter = dt_struct; iter < dt_struct + header.size_dt_struct;) {
        auto seek_and_align = [&iter](const std::size_t size) {
            const auto value = size % sizeof(fdt::token);
            if (value)
                return iter += size + sizeof(fdt::token) - value;

            return iter += size;
        };

        const auto token = static_cast<fdt::token>(convert(*reinterpret_cast<const u32 *>(iter)));
        seek_and_align(sizeof(token));

        if (fdt::token::begin_node == token) {
            const auto size = std::strlen(iter);
            auto name = QString::fromUtf8(iter, size);
            seek_and_align(size);
            generator.begin_node(size ? name : m_default_root_node);
        }

        if (fdt::token::end_node == token)
            generator.end_node();

        if (fdt::token::property == token) {
            const auto header = read_data_32be<fdt::property>(iter);
            seek_and_align(sizeof(header));

            fdt_property property;
            property.data = QByteArray(iter, header.len);
            seek_and_align(header.len);

            property.name = get_property_name(header.nameoff);
            generator.insert_property(property);

            for (auto &&handle : m_handle_special_properties)
                if (handle.name == property.name)
                    handle.callback(property, generator);
        }

        if (fdt::token::end == token)
            break;
    }
}
*/

auto align(const std::size_t size) {
    const auto q = size % sizeof(u32);
    const auto w = size / sizeof(u32);
    return std::max<u32>(1u, w + (q ? 1u : 0u));
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

template <class... Ts>
struct overloaded : Ts... {
    using Ts::operator()...;
};

void fdt_parser::parse(const fdt::header header, iface_fdt_generator &generator) {
    const auto dt_struct = m_data + header.off_dt_struct;
    const auto dt_strings = m_data + header.off_dt_strings;

    auto get_property_name = [&](auto offset) {
        const auto ptr = dt_strings + offset;
        return QString::fromUtf8(ptr, std::strlen(ptr));
    };

    using namespace fdt::tokenizer;
    using namespace fdt::tokenizer::types;

    context ctx{
        .structs = {dt_struct, header.size_dt_struct}, //
        .strings = {dt_strings, header.size_dt_strings}, //
    };

    const auto begin = reinterpret_cast<const u32 *>(dt_struct);
    const auto end = reinterpret_cast<const u32 *>(dt_struct) + header.size_dt_struct / sizeof(u32);

    const auto is_aligned = (header.size_dt_struct % sizeof(u32)) == 0;

    for (auto iter = begin; iter != end;) {
        const auto id = static_cast<u32>(convert(*iter));
        ctx.state.data = reinterpret_cast<const char *>(++iter);
        ctx.state.skip = 0;

        if (!foreach_token_type(token{}, id, ctx))
            break;

        // if (!ctx.tokens.empty() && std::holds_alternative<types::end>(ctx.tokens.back()))
        //     break;

        iter += ctx.state.skip;
    }

    const auto node_begin_count = std::ranges::count_if(ctx.tokens, [](auto &&v) {
        return std::holds_alternative<node_begin>(v);
    });

    const auto node_end_count = std::ranges::count_if(ctx.tokens, [](auto &&v) {
        return std::holds_alternative<node_end>(v);
    });

    const auto property_count = std::ranges::count_if(ctx.tokens, [](auto &&v) {
        return std::holds_alternative<property>(v);
    });

    const auto nop_count = std::ranges::count_if(ctx.tokens, [](auto &&v) {
        return std::holds_alternative<nop>(v);
    });

    const auto end_count = std::ranges::count_if(ctx.tokens, [](auto &&v) {
        return std::holds_alternative<types::end>(v);
    });

    std::cout << "is_aligned : " << is_aligned << std::endl;
    std::cout << "node begin : " << node_begin_count << std::endl;
    std::cout << "node end   : " << node_end_count << std::endl;
    std::cout << "property   : " << property_count << std::endl;
    std::cout << "nop        : " << nop_count << std::endl;
    std::cout << "end        : " << end_count << std::endl;

    for (auto &&token : ctx.tokens)
        std::visit(overloaded{
                       [&](node_begin &arg) { generator.begin_node(QString::fromUtf8(arg.name.data(), arg.name.size())); },
                       [&](node_end &arg) { generator.end_node(); },
                       [&](property &arg) {
                           auto property = fdt_property{
                               .name = QString::fromUtf8(arg.name.data(), arg.name.size()),
                               .data = QByteArray(arg.data.data(), arg.data.size()),
                           };

                           generator.insert_property(property);

                           // for (auto &&handle : m_handle_special_properties)
                           //     if (handle.name.toStdString() == arg.name)
                           //         handle.callback(property, generator);
                       },
                       [&](nop &) {},
                       [&](types::end &) {},
                   },
            token);
}
