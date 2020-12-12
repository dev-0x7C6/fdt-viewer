#include "fdt-view.hpp"

#include <fdt-parser.hpp>

#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QFileInfo>

#include <stack>

bool fdt::fdt_view_prepare(tree_widget *target, const byte_array &datamap, const file_info &info) {
    fdt_generator generator;

    auto root = new tree_widget_item(target);
    root->setText(0, info.fileName());
    root->setData(0, QT_ROLE_FILEPATH, info.absoluteFilePath());

    std::stack<tree_widget_item *> tree_stack;

    generator.begin_node = [&](std::string_view &&name) {
        auto child = [&]() {
            if (tree_stack.empty())
                return root;
            else
                return new tree_widget_item(tree_stack.top());
        }();

        if (child->text(0).isEmpty())
            child->setText(0, QString::fromStdString(name.data()));

        tree_stack.emplace(child);
    };

    generator.end_node = [&tree_stack]() {
        tree_stack.pop();
    };

    generator.insert_property = [&tree_stack](std::string_view &&name, std::string_view &&data) {
        auto current = tree_stack.top();
        QVariant values = current->data(0, QT_ROLE_PROPERTY);
        auto properties = values.value<qt_fdt_properties>();
        qt_fdt_property property;
        property.name = QString::fromStdString(name.data());
        property.data = QByteArray(data.data(), data.size());
        properties << property;
        current->setData(0, QT_ROLE_PROPERTY, QVariant::fromValue(properties));
    };

    fdt_parser parser(datamap.data(), datamap.size(), generator);

    if (!parser.is_valid()) {
        delete root;
        return false;
    }

    return true;
}
