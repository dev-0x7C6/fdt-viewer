#include "fdt-parser.hpp"
#include <ostream>
#include <string_view>
#include <variant>

#include <cstring>
#include <concepts>
#include <algorithm>
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

#include "fdt-parser-v2.hpp"
#include <iostream>
#include <algorithm>

auto align(const std::size_t size) {
    const auto q = size % sizeof(u32);
    const auto w = size / sizeof(u32);
    return std::max<u32>(1u, w + (q ? 1u : 0u));
};

namespace fdt::parser {

using namespace fdt::tokenizer;
using namespace fdt::tokenizer::types;

auto parse(node_begin &&token, const char *data, int &skip, context &ctx) -> fdt::tokenizer::token {
    const auto size = std::strlen(data);
    token.name = std::string_view(data, size);
    skip += align(size);
    return {std::move(token)};
}

auto parse(node_end &&token, const char *data, int &skip, context &ctx) -> fdt::tokenizer::token {
    return {std::move(token)};
}

auto parse(types::property &&token, const char *data, int &skip, context &ctx) -> fdt::tokenizer::token {
    const auto header = read_data_32be<fdt::property>(data);
    skip += align(sizeof(header)) + align(header.len);
    data += sizeof(header);

    token.name = std::string_view();
    token.data = std::string_view(data, header.len);

    return {std::move(token)};
}

auto parse(nop &&token, const char *data, int &skip, context &ctx) -> fdt::tokenizer::token {
    return {std::move(token)};
}

auto parse(end &&token, const char *data, int &skip, context &ctx) -> fdt::tokenizer::token {
    return {std::move(token)};
}
} // namespace fdt::parser

template <fdt::tokenizer::Tokenizable... Ts>
auto foreach_token_type(std::variant<Ts...>, const u32 token_id, const char *data, int &skip, fdt::tokenizer::context &ctx) {
    auto conditional_parse = [&](auto &&token) {
        if (fdt::tokenizer::types::id_of(token) == token_id) {
            ctx.tokens.emplace_back(fdt::parser::parse(std::move(token), data, skip, ctx));
            return true;
        }
        return false;
    };
    return (conditional_parse(Ts{}) || ...);
}

void fdt_parser::parse(const fdt::header header, iface_fdt_generator &generator) {
    const auto dt_struct = m_data + header.off_dt_struct;
    const auto dt_strings = m_data + header.off_dt_strings;

    auto get_property_name = [&](auto offset) {
        const auto ptr = dt_strings + offset;
        return QString::fromUtf8(ptr, std::strlen(ptr));
    };

    using namespace fdt::tokenizer;
    using namespace fdt::tokenizer::types;
    context ctx;

    auto begin = reinterpret_cast<const u32 *>(dt_struct);
    auto end = reinterpret_cast<const u32 *>(dt_struct) + header.size_dt_struct / sizeof(u32);

    for (auto iter = begin; iter != end;) {
        const auto id = static_cast<u32>(convert(*iter));
        iter++;
        std::cout << id << std::endl;

        auto skip = 0;
        foreach_token_type(token{}, id, reinterpret_cast<const char *>(iter), skip, ctx);
        iter += skip;
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

    std::cout << "node begin  count: " << node_begin_count << std::endl;
    std::cout << "node end    count: " << node_end_count << std::endl;
    std::cout << "property    count: " << property_count << std::endl;
    std::cout << "nop         count: " << nop_count << std::endl;
    std::cout << "end         count: " << end_count << std::endl;
}
