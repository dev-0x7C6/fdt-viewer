#pragma once

#include <QMetaType>

#include <fdt-property-types.hpp>
#include <types.hpp>

struct qt_fdt_property {
    QString name;
    QByteArray data;
};

Q_DECLARE_METATYPE(qt_fdt_property)

constexpr auto QT_ROLE_PROPERTY = Qt::UserRole;
constexpr auto QT_ROLE_FILEPATH = Qt::UserRole + 1;
constexpr auto QT_ROLE_NODETYPE = Qt::UserRole + 2;

enum NodeType {
    Node,
    Property
};

Q_DECLARE_METATYPE(NodeType)

namespace fdt {

bool fdt_view_prepare(tree_widget *target, const byte_array &datamap, const file_info &info);
void fdt_view_dts(tree_widget_item *item, string &ret, int depth = 0);

} // namespace fdt
