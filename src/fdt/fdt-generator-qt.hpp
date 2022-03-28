#pragma once

#include <fdt/fdt-generator.hpp>
#include <fdt/fdt-header.hpp>
#include <fdt/fdt-property-types.hpp>
#include <types.hpp>

#include <QMetaType>
#include <QTreeWidgetItem>
#include <QHash>
#include <stack>

Q_DECLARE_METATYPE(fdt_property)

constexpr auto QT_ROLE_PROPERTY = Qt::UserRole;
constexpr auto QT_ROLE_FILEPATH = Qt::UserRole + 1;
constexpr auto QT_ROLE_NODETYPE = Qt::UserRole + 2;

enum class NodeType {
    Node,
    Property
};

Q_DECLARE_METATYPE(NodeType)

template <typename... types>
using hash_map = QHash<types...>;

using node_map = hash_map<string, tree_widget_item *>;

struct tree_info {
    string id;
    tree_widget_item *root{nullptr};
    node_map nodes;
};

using tree_map = hash_map<string, tree_info>;

struct qt_tree_fdt_generator : public iface_fdt_generator {
    qt_tree_fdt_generator(tree_info &reference, tree_widget *target, string &&name, string &&id);

    void begin_node(const QString &name) noexcept final;
    void end_node() noexcept final;
    void insert_property(const fdt_property &property) noexcept final;

private:
    tree_widget_item *m_root{nullptr};
    std::stack<tree_widget_item *> m_tree_stack;
};
