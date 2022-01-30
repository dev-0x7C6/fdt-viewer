#include "fdt-view.hpp"

#include <endian-conversions.hpp>
#include <fdt-parser.hpp>

#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QFileInfo>

#include <stack>

namespace {
constexpr auto BINARY_PREVIEW_LIMIT = 256;

string present_u32be(const QByteArray &data) {
    string ret;

    auto array = reinterpret_cast<u8 *>(const_cast<char *>(data.data()));
    for (auto i = 0; i < data.size(); ++i) {
        ret += "0x" + QString::number(array[i], 16).rightJustified(2, '0').toUpper() + " ";
        if (i == BINARY_PREVIEW_LIMIT) {
            ret += "... ";
            break;
        }
    }
    ret.remove(ret.size() - 1, 1);

    return ret;
}

string present(const qt_fdt_property &property) {
    auto &&name = property.name;
    auto &&data = property.data;

    auto result = [&](string &&value) {
        return name + " = <" + value + ">;";
    };

    auto result_str = [&](string &&value) {
        return name + " = \"" + value + "\";";
    };

    if (property_map.contains(name)) {
        const property_info info = property_map.value(name);
        if (property_type::string == info.type)
            return result_str({data});

        if (property_type::number == info.type)
            return result(string::number(u32_be(data.data())));
    }

    const static regexp cells_regexp("^#.*-cells$");
    const static regexp names_regexp("^.*-names");

    if (cells_regexp.exactMatch(name))
        return result(string::number(u32_be(data.data())));

    if (names_regexp.exactMatch(name)) {
        auto lines = data.split(0);
        lines.removeLast();

        string ret;
        for (auto i = 0; i < lines.count(); ++i) {
            if (i == lines.count() - 1)
                ret += lines[i];
            else
                ret += lines[i] + ", ";
        }

        return result_str(std::move(ret));
    }

    if (std::count_if(data.begin(), data.end(), [](auto &&value) { return value == 0x00; }) == 1 &&
        data.at(data.size() - 1) == 0x00) return result_str({property.data});

    return result(present_u32be(property.data));
}
} // namespace

fdt::viewer::viewer(tree_widget *target)
        : m_target(target) {
}

auto fdt::viewer::is_loaded(string &&id) const noexcept -> bool {
    return m_tree.contains(id);
}

auto fdt::viewer::is_loaded(const string &id) const noexcept -> bool {
    return m_tree.contains(id);
}

bool fdt::viewer::load(const byte_array &datamap, string &&name, string &&id) {
    fdt_generator generator;

    auto &reference = m_tree[id];

    auto root = [&]() {
        if (reference.root)
            return reference.root;

        auto ret = new tree_widget_item(m_target);
        reference.root = ret;
        return ret;
    }();

    root->setText(0, name);
    root->setData(0, QT_ROLE_FILEPATH, id);
    root->setIcon(0, QIcon::fromTheme("folder-open"));
    root->setData(0, QT_ROLE_NODETYPE, QVariant::fromValue(NodeType::Node));
    root->setExpanded(true);
    root->setSelected(true);

    std::stack<tree_widget_item *> tree_stack;

    generator.begin_node = [&](std::string_view &&name) {
        auto child = [&]() {
            if (tree_stack.empty())
                return root;

            tree_widget_item *item = nullptr;
            tree_widget_item *root = tree_stack.top();

            for (auto i = 0; i < root->childCount(); ++i)
                if (root->child(i)->text(0) == QString::fromStdString(name.data())) {
                    auto ret = root->child(i);
                    ret->setIcon(0, QIcon::fromTheme("folder-new"));
                    ret->setData(0, QT_ROLE_NODETYPE, QVariant::fromValue(NodeType::Node));
                    return root->child(i);
                }

            return new tree_widget_item(root);
        }();

        if (child->text(0).isEmpty()) {
            child->setText(0, QString::fromStdString(name.data()));
            child->setIcon(0, QIcon::fromTheme("folder-open"));
            child->setData(0, QT_ROLE_NODETYPE, QVariant::fromValue(NodeType::Node));
        }

        tree_stack.emplace(child);
    };

    generator.end_node = [&tree_stack]() {
        tree_stack.pop();
    };

    generator.insert_property = [&tree_stack](const fdt_property &property) {
        auto item = new tree_widget_item(tree_stack.top());

        item->setText(0, QString::fromStdString(property.name));
        item->setIcon(0, QIcon::fromTheme("flag-green"));
        item->setData(0, QT_ROLE_NODETYPE, QVariant::fromValue(NodeType::Property));

        qt_fdt_property p2;
        p2.name = QString::fromStdString(property.name);
        p2.data = std::move(property.data);
        item->setData(0, QT_ROLE_PROPERTY, QVariant::fromValue(p2));
    };

    std::vector<fdt_handle_special_property> handle_special_properties;

    fdt_handle_special_property handle_inner_dt;
    handle_inner_dt.name = "data";
    handle_inner_dt.callback = [&handle_special_properties](const fdt_property &property, fdt_generator &generator) {
        fdt_parser(property.data.data(), property.data.size(), generator, property.name, handle_special_properties);
    };

    handle_special_properties.emplace_back(std::move(handle_inner_dt));

    fdt_parser parser(datamap.data(), datamap.size(), generator, {}, handle_special_properties);

    if (!parser.is_valid()) {
        delete root;
        return false;
    }

    return true;
}

void fdt::viewer::drop(string &&id) {
    m_tree.remove(id);
}

bool fdt::fdt_content_filter(tree_widget_item *node, const std::function<bool(const string &)> &match) {
    QList<tree_widget_item *> nodes;
    QList<tree_widget_item *> properties;

    for (auto i = 0; i < node->childCount(); ++i) {
        const auto child = node->child(i);
        switch (child->data(0, QT_ROLE_NODETYPE).value<NodeType>()) {
            case NodeType::Node:
                nodes.append(child);
                break;
            case NodeType::Property:
                properties.append(child);
                break;
        }
    }

    const auto name = node->data(0, Qt::DisplayRole).toString();

    bool isFound = match(name);

    for (auto item : properties) {
        if (isFound)
            break;

        const auto property = item->data(0, QT_ROLE_PROPERTY).value<qt_fdt_property>();
        isFound |= match(property.name) || match(present(property));
    }

    for (auto i = 0; i < nodes.count(); ++i) {
        isFound |= fdt::fdt_content_filter(nodes.at(i), match);
    }

    node->setHidden(!isFound);

    return isFound;
}

bool fdt::fdt_view_dts(tree_widget_item *item, string &ret, int depth) {
    string depth_str;
    depth_str.fill(' ', depth * 4);

    QList<tree_widget_item *> nodes;
    QList<tree_widget_item *> properties;

    for (auto i = 0; i < item->childCount(); ++i) {
        const auto child = item->child(i);
        switch (child->data(0, QT_ROLE_NODETYPE).value<NodeType>()) {
            case NodeType::Node:
                nodes.append(child);
                break;
            case NodeType::Property:
                properties.append(child);
                break;
        }
    }

    if (item->isHidden())
        return false;

    ret += depth_str + item->data(0, Qt::DisplayRole).toString() + " {\n";

    for (auto item : properties) {
        const auto property = item->data(0, QT_ROLE_PROPERTY).value<qt_fdt_property>();
        ret += depth_str + "    " + present(property) + "\n";
    }

    if (!properties.isEmpty() && !nodes.isEmpty())
        ret += "\n";

    for (auto i = 0; i < nodes.count(); ++i) {
        if (!fdt_view_dts(nodes.at(i), ret, depth + 1))
            continue;

        if (nodes.count() - 1 != i)
            ret += "\n";
    }

    ret += depth_str + "};\n";

    return true;
}
