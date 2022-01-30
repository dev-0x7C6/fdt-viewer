#pragma once

#include <fdt-header.hpp>

#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <vector>
#include <QByteArray>

struct fdt_property {
    std::string name;
    QByteArray data;

    auto clear() noexcept {
        name.clear();
        data.clear();
    }
};

struct fdt_generator {
    std::function<void(std::string_view &&view)> begin_node{[](auto...) {}};
    std::function<void()> end_node{[](auto...) {}};
    std::function<void(const fdt_property &property)> insert_property{[](auto...) {}};
};

using fdt_property_callback = std::function<void(const fdt_property &property, fdt_generator &generator)>;

struct fdt_handle_special_property {
    std::string name;
    fdt_property_callback callback;
};

class fdt_parser {
public:
    fdt_parser(const char *data, u64 size, fdt_generator &generator,
        const std::string &default_root_node = {},
        const std::vector<fdt_handle_special_property> &handle_special_properties = {});
    constexpr bool is_valid() noexcept { return m_header.has_value(); }

private:
    void parse(const fdt_header header, fdt_generator &generator);

private:
    std::optional<fdt_header> m_header;
    std::string_view m_dt_struct;
    std::string_view m_dt_strings;
    const std::string m_default_root_node;
    const std::vector<fdt_handle_special_property> &m_handle_special_properties;

    const char *const m_data;
    const u64 m_size;
};
