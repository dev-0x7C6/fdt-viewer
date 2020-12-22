#include "fdt-view.hpp"

#include <endian-conversions.hpp>
#include <fdt-parser.hpp>

#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QFileInfo>

#include <stack>

namespace {
constexpr auto BINARY_PREVIEW_LIMIT = 2048;

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
        data.back() == 0x00) return result_str({property.data});

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
    root->setData(0, QT_ROLE_NODETYPE, NodeType::Node);

    std::stack<tree_widget_item *> tree_stack;

    generator.begin_node = [&](std::string_view &&name) {
        auto child = [&]() {
            if (tree_stack.empty())
                return root;
            else
                return new tree_widget_item(tree_stack.top());
        }();

        if (child->text(0).isEmpty()) {
            child->setText(0, QString::fromStdString(name.data()));
            child->setIcon(0, QIcon::fromTheme("folder-open"));
            child->setData(0, QT_ROLE_NODETYPE, NodeType::Node);
        }

        tree_stack.emplace(child);
    };

    generator.end_node = [&tree_stack]() {
        tree_stack.pop();
    };

    generator.insert_property = [&tree_stack](std::string_view &&name, std::string_view &&data) {
        auto item = new tree_widget_item(tree_stack.top());

        item->setText(0, QString::fromStdString(name.data()));
        item->setIcon(0, QIcon::fromTheme("flag-green"));
        item->setData(0, QT_ROLE_NODETYPE, NodeType::Property);

        qt_fdt_property property;
        property.name = QString::fromStdString(name.data());
        property.data = QByteArray(data.data(), data.size());
        item->setData(0, QT_ROLE_PROPERTY, QVariant::fromValue(property));
    };

    fdt_parser parser(datamap.data(), datamap.size(), generator);

    if (!parser.is_valid()) {
        delete root;
        return false;
    }

    return true;
}

void fdt::viewer::drop(string &&id) {
    m_tree.remove(id);
}

void fdt::fdt_view_dts(tree_widget_item *item, string &ret, int depth) {
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

    ret += depth_str + item->data(0, Qt::DisplayRole).toString() + " {\n";

    for (auto item : properties) {
        const auto property = item->data(0, QT_ROLE_PROPERTY).value<qt_fdt_property>();
        ret += depth_str + "    " + present(property) + "\n";
    }

    if (!properties.isEmpty() && !nodes.isEmpty())
        ret += "\n";

    for (auto i = 0; i < nodes.count(); ++i) {
        fdt_view_dts(nodes.at(i), ret, depth + 1);

        if (nodes.count() - 1 != i)
            ret += "\n";
    }

    ret += depth_str + "};\n";
}
