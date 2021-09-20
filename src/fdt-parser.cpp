#include "fdt-parser.hpp"

#include <cstring>
#include <endian-conversions.hpp>

fdt_parser::fdt_parser(const char *data, u64 size, fdt_generator &generator, const std::string &default_root_node, const std::vector<fdt_handle_special_property> &handle_special_properties)
        : m_data(data)
        , m_size(size)
        , m_default_root_node(default_root_node)
        , m_handle_special_properties(handle_special_properties) {
    if (size >= sizeof(fdt_header)) {
        auto header = read_data_32be<fdt_header>(data);
        if (FDT_MAGIC_VALUE != header.magic || size != header.totalsize)
            return;

        if (FDT_SUPPORT_ABOVE > header.version)
            return;

        m_header = std::move(header);
        parse(m_header.value(), generator);
    }
}

void fdt_parser::parse(const fdt_header header, fdt_generator &generator) {
    const auto dt_struct = m_data + header.off_dt_struct;
    const auto dt_strings = m_data + header.off_dt_strings;

    fdt_property property;
    std::string node_name;

    property.name.reserve(4096);
    property.data.reserve(4096);
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
            node_name.resize(node_name.size() - 1);
            generator.begin_node(node_name.size() == 0 ? m_default_root_node : node_name);
        }

        if (FDT_TOKEN::END_NODE == token) {
            generator.end_node();
        }

        if (FDT_TOKEN::PROPERTY == token) {
            const auto header = read_data_32be<fdt_property_header>(iter);
            iter += sizeof(header);
            for (auto i = 0u; i < header.len; ++i)
                property.data += *iter++;
            align(property.data.size());

            for (auto str = dt_strings + header.nameoff; *str != 0x00; ++str)
                property.name += *str;

            generator.insert_property(property.name, property.data);

            for (auto &&handle : m_handle_special_properties)
                if (handle.name == property.name)
                    handle.callback(property, generator);
        }

        if (FDT_TOKEN::END == token)
            break;

        property.clear();
        node_name.clear();
    }
}
