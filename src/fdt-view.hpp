#pragma once

#include <QMetaType>

#include <fdt-property-types.hpp>
#include <types.hpp>

struct qt_fdt_property {
    QString name;
    QByteArray data;
};

using qt_fdt_properties = QList<qt_fdt_property>;

Q_DECLARE_METATYPE(qt_fdt_property)
Q_DECLARE_METATYPE(qt_fdt_properties)

constexpr auto QT_ROLE_PROPERTY = Qt::UserRole;
constexpr auto QT_ROLE_FILEPATH = Qt::UserRole + 1;

namespace fdt {

bool fdt_view_prepare(tree_widget *target, const byte_array &datamap, const file_info &info);

}
