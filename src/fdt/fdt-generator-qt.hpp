#pragma once

#include <fdt/fdt-generator.hpp>
#include <fdt/fdt-header.hpp>
#include <fdt/fdt-property-types.hpp>
#include <types.hpp>

#include <QMetaType>
#include <QTreeWidgetItem>
#include <QHash>
#include "fdt/fdt-parser-v2.hpp"
#include <stack>
#include <string_view>

Q_DECLARE_METATYPE(fdt::qt_wrappers::property)

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

using node_map = hash_map<string, QTreeWidgetItem *>;

struct tree_info {
    string id;
    QTreeWidgetItem *root{nullptr};
    node_map nodes;
};

using tree_map = hash_map<string, tree_info>;

struct qt_tree_fdt_generator : public iface_fdt_generator {
    qt_tree_fdt_generator(tree_info &reference, tree_widget *target, string &&name, string &&id);

    void begin_node(std::string_view) noexcept final;
    void end_node() noexcept final;
    void insert_property(const fdt::tokenizer::types::property &) noexcept final;

    auto root() { return m_root; }

private:
    QTreeWidgetItem *m_root{nullptr};
    std::stack<QTreeWidgetItem *> m_tree_stack;
};
