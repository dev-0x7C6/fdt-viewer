#pragma once

#include <string_view>
#include <QByteArray>
#include <QString>

#include "fdt/fdt-parser-v2.hpp"

struct iface_fdt_generator {
    virtual void begin_node(std::string_view) noexcept = 0;
    virtual void end_node() noexcept = 0;
    virtual void insert_property(const fdt::tokenizer::types::property &) noexcept = 0;
};
