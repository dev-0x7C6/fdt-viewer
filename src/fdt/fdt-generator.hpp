#pragma once

#include <string_view>
#include <QByteArray>
#include <QString>

struct fdt_property {
    QString name;
    std::string_view data;

    auto clear() noexcept {
        name.clear();
    }
};

struct iface_fdt_generator {
    virtual void begin_node(const QString &name) noexcept = 0;
    virtual void end_node() noexcept = 0;
    virtual void insert_property(const fdt_property &property) noexcept = 0;
};
