#pragma once

#include <QMetaType>
#include <QHash>

#include <fdt-property-types.hpp>
#include <types.hpp>

template <typename... types>
using hash_map = QHash<types...>;

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

using node_map = hash_map<string, tree_widget_item *>;

struct tree_info {
    string id;
    tree_widget_item *root{nullptr};
    node_map nodes;
};

using tree_map = hash_map<string, tree_info>;

class viewer {
public:
    viewer(tree_widget *target);

    auto is_loaded(string &&id) const noexcept -> bool;
    auto is_loaded(const string &id) const noexcept -> bool;

    auto load(const byte_array &datamap, string &&name, string &&id) -> bool;
    auto drop(string &&id) -> void;

private:
    tree_map m_tree;
    tree_widget *m_target;
};

bool fdt_view_prepare(tree_widget *target, const byte_array &datamap, const file_info &info);
void fdt_view_dts(tree_widget_item *item, string &ret, int depth = 0);

} // namespace fdt
