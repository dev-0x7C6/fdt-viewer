#pragma once

#include "fdt/fdt-property-types.hpp"
#include "fdt/fdt-parser-tokens.hpp"

#include <stack>
#include <string_view>

#include <QMetaType>
#include <QTreeWidgetItem>
#include <QHash>

Q_DECLARE_METATYPE(fdt::qt_wrappers::property)

enum class NodeType {
    Node,
    Property
};

Q_DECLARE_METATYPE(NodeType)

template <typename... types>
using hash_map = QHash<types...>;

using node_map = hash_map<QString, QTreeWidgetItem *>;

struct tree_info {
    QString id;
    QTreeWidgetItem *root{nullptr};
    node_map nodes;
};

using tree_map = hash_map<QString, tree_info>;

namespace fdt::qt_wrappers {

constexpr auto ROLE_PROPERTY = Qt::UserRole;
constexpr auto ROLE_FILEPATH = Qt::UserRole + 1;
constexpr auto ROLE_NODETYPE = Qt::UserRole + 2;
constexpr auto ROLE_DATA_HOLDER = Qt::UserRole + 3;

struct tree_generator {
    tree_generator(tree_info &reference, QTreeWidget *target, QString &&name, QString &&id);

    void begin_node(std::string_view) noexcept;
    void end_node() noexcept;
    void insert_property(const fdt::parser::token_types::property &) noexcept;

    auto root() { return m_root; }

private:
    QTreeWidgetItem *m_root{nullptr};
    std::stack<QTreeWidgetItem *> m_tree_stack;
};

} // namespace fdt::qt_wrappers
