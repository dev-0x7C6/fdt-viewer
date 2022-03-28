#include "fdt-generator-qt.hpp"

qt_tree_fdt_generator::qt_tree_fdt_generator(tree_info &reference, tree_widget *target, string &&name, string &&id) {
    m_root = [&]() {
        if (reference.root)
            return reference.root;

        auto ret = new tree_widget_item(target);
        reference.root = ret;
        return ret;
    }();

    m_root->setText(0, name);
    m_root->setData(0, QT_ROLE_FILEPATH, id);
    m_root->setIcon(0, QIcon::fromTheme("folder-open"));
    m_root->setData(0, QT_ROLE_NODETYPE, QVariant::fromValue(NodeType::Node));
    m_root->setExpanded(true);
    m_root->setSelected(true);
}

void qt_tree_fdt_generator::begin_node(const QString &name) noexcept {
    auto child = [&]() {
        if (m_tree_stack.empty())
            return m_root;

        tree_widget_item *item = nullptr;
        tree_widget_item *root = m_tree_stack.top();

        for (auto i = 0; i < root->childCount(); ++i)
            if (root->child(i)->text(0) == name) {
                auto ret = root->child(i);
                ret->setIcon(0, QIcon::fromTheme("folder-new"));
                ret->setData(0, QT_ROLE_NODETYPE, QVariant::fromValue(NodeType::Node));
                return root->child(i);
            }

        return new tree_widget_item(root);
    }();

    if (child->text(0).isEmpty()) {
        child->setText(0, name);
        child->setIcon(0, QIcon::fromTheme("folder-open"));
        child->setData(0, QT_ROLE_NODETYPE, QVariant::fromValue(NodeType::Node));
    }

    m_tree_stack.emplace(child);
}

void qt_tree_fdt_generator::end_node() noexcept {
    m_tree_stack.pop();
}

void qt_tree_fdt_generator::insert_property(const fdt_property &property) noexcept {
    auto item = new tree_widget_item(m_tree_stack.top());

    item->setText(0, property.name);
    item->setIcon(0, QIcon::fromTheme("flag-green"));
    item->setData(0, QT_ROLE_NODETYPE, QVariant::fromValue(NodeType::Property));
    item->setData(0, QT_ROLE_PROPERTY, QVariant::fromValue(property));
}
