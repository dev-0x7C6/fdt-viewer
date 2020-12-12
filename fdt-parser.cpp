#include "fdt-parser.hpp"

#include <bit>
#include <cstring>

namespace {

u32 u32_be(const char *data) {
    if constexpr (std::endian::native == std::endian::big) {
        return *reinterpret_cast<const u32 *>(data);
    } else {
        return __builtin_bswap32(*reinterpret_cast<const u32 *>(data));
    }

    return 0;
}

u32 u32_be(u32 data) {
    if constexpr (std::endian::native == std::endian::big) {
        return data;
    } else {
        return __builtin_bswap32(data);
    }

    return 0;
}

template <typename type, typename input_data>
type read_data_32be(input_data *input) {
    type container{};
    std::memcpy(reinterpret_cast<void *>(&container), reinterpret_cast<const void *>(input), sizeof(type));
    auto *data = reinterpret_cast<u32 *>(&container);
    for (auto i = 0; i < (sizeof(type) / sizeof(u32)); ++i)
        data[i] = u32_be(data[i]);
    return container;
}
} // namespace

fdt_parser::fdt_parser(const char *data, u64 size, fdt_generator &generator)
        : m_data(data)
        , m_size(size) {
    if (size >= sizeof(fdt_header)) {
        auto header = read_data_32be<fdt_header>(data);
        if (FDT_MAGIC_VALUE == header.magic && size == header.totalsize)
            m_header = std::move(header);

        parse(m_header.value(), generator);
    }
}

void fdt_parser::parse(const fdt_header header, fdt_generator &generator) {
    const auto dt_struct = m_data + header.off_dt_struct;
    const auto dt_strings = m_data + header.off_dt_strings;

    std::string property_name;
    std::string property_data;
    std::string node_name;

    property_data.reserve(4096);
    property_name.reserve(4096);
    node_name.reserve(4096);

    for (auto iter = dt_struct; iter < dt_struct + header.size_dt_struct;) {
        const auto token = static_cast<FDT_TOKEN>(u32_be(iter));
        iter += sizeof(FDT_TOKEN);

        auto align = [&iter](const std::size_t readed) {
            const auto value = readed % sizeof(FDT_TOKEN);
            if (value)
                iter += sizeof(FDT_TOKEN) - value;
        };

        if (FDT_TOKEN::BEGIN_NODE == token) {
            while (*iter != 0x00)
                node_name += *iter++;
            node_name += *++iter;
            align(node_name.size());
            generator.begin_node(node_name);
        }

        if (FDT_TOKEN::END_NODE == token) {
            generator.end_node();
        }

        if (FDT_TOKEN::PROPERTY == token) {
            const auto property = read_data_32be<fdt_property_header>(iter);
            iter += sizeof(property);
            for (auto i = 0; i < property.len; ++i)
                property_data += *iter++;
            align(property_data.size());

            for (auto str = dt_strings + property.nameoff; *str != 0x00; ++str)
                property_name += *str;

            generator.insert_property(property_name, property_data);
        }

        if (FDT_TOKEN::END == token)
            break;

        property_data.clear();
        property_name.clear();
        node_name.clear();
    }
}
