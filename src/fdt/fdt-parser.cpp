#include "fdt-parser.hpp"

#include <cstring>
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
