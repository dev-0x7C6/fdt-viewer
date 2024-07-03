#include "fdt-generator-qt.hpp"
#include "fdt/fdt-parser-tokens.hpp"
#include "fdt/fdt-property-types.hpp"
#include <memory>
#include <string_view>

using namespace fdt::qt_wrappers;

tree_generator::tree_generator(tree_info &reference, QTreeWidget *target, QString &&name, QString &&id) {
    m_root = [&]() {
        if (reference.root)
            return reference.root;

        auto ret = new QTreeWidgetItem(target);
        reference.root = ret;
        return ret;
    }();

    m_root->setText(0, name);
    m_root->setIcon(0, QIcon::fromTheme("folder-open"));
    m_root->setData(0, QT_ROLE_FILEPATH, id);
    m_root->setData(0, QT_ROLE_NODETYPE, QVariant::fromValue(NodeType::Node));
    m_root->setExpanded(true);
    m_root->setSelected(true);
}

void tree_generator::begin_node(std::string_view vname) noexcept {
    const auto name = QString::fromUtf8(vname.data(), vname.size());

    auto child = [&]() {
        if (m_tree_stack.empty())
            return m_root;

        QTreeWidgetItem *item = nullptr;
        QTreeWidgetItem *root = m_tree_stack.top();

        for (auto i = 0; i < root->childCount(); ++i)
            if (root->child(i)->text(0) == name) {
                auto ret = root->child(i);
                ret->setIcon(0, QIcon::fromTheme("folder-new"));
                ret->setData(0, QT_ROLE_NODETYPE, QVariant::fromValue(NodeType::Node));
                return root->child(i);
            }

        return new QTreeWidgetItem(root);
    }();

    if (child->text(0).isEmpty()) {
        child->setText(0, name);
        child->setIcon(0, QIcon::fromTheme("folder-open"));
        child->setData(0, QT_ROLE_NODETYPE, QVariant::fromValue(NodeType::Node));
    }

    m_tree_stack.emplace(child);
}

void tree_generator::end_node() noexcept {
    m_tree_stack.pop();
}

void tree_generator::insert_property(const fdt::parser::token_types::property &prop) noexcept {
    auto item = new QTreeWidgetItem(m_tree_stack.top());

    auto property = fdt::qt_wrappers::property{
        .name = QString::fromUtf8(prop.name.data(), prop.name.size()),
        .data = QByteArray::fromRawData(prop.data.data(), prop.data.size()),
    };

    item->setText(0, QString::fromUtf8(prop.name.data(), prop.name.size()));
    item->setIcon(0, QIcon::fromTheme("flag-green"));
    item->setData(0, QT_ROLE_NODETYPE, QVariant::fromValue(NodeType::Property));
    item->setData(0, QT_ROLE_PROPERTY, QVariant::fromValue(std::move(property)));
}
