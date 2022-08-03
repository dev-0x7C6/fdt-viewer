#pragma once

#include <fdt/fdt-header.hpp>

#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include <fdt/fdt-generator.hpp>

using fdt_property_callback = std::function<void(const fdt_property &property, iface_fdt_generator &generator)>;

struct fdt_handle_special_property {
    QString name;
    fdt_property_callback callback;
};

class fdt_parser {
public:
    fdt_parser(const char *data, u64 size, iface_fdt_generator &generator,
        const QString &default_root_node = {},
        const std::vector<fdt_handle_special_property> &handle_special_properties = {});
    constexpr bool is_valid() noexcept { return m_header.has_value(); }

private:
    void parse(const fdt::header header, iface_fdt_generator &generator);

private:
    std::optional<fdt::header> m_header;
    const QString m_default_root_node;
    const std::vector<fdt_handle_special_property> &m_handle_special_properties;

    const char *const m_data;
    const u64 m_size;
};
