#include "fdt-view.hpp"

#include <endian-conversions.hpp>
#include <fdt/fdt-generator-qt.hpp>
#include <fdt/fdt-parser.hpp>

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

string present(const fdt_property &property) {
    auto &&name = property.name;
    auto &&data = property.data;

    auto result = [&](string &&value) {
        return name + " = <" + value + ">;";
    };

    auto result_str = [&](string &&value) {
        return name + " = \"" + value + "\";";
    };

    auto result_multi = [&](auto &value) {
        auto lines = value.split(0);
        lines.removeLast();

        string ret;
        for (auto i = 0; i < lines.count(); ++i) {
            if (i == lines.count() - 1)
                ret += lines[i];
            else
                ret += lines[i] + "\", \"";
        }

        return result_str(std::move(ret));
    };

    if (property_map.contains(name)) {
        const property_info info = property_map.value(name);
        if (property_type::string == info.type)
            return result_str({data});

	if (property_type::multiline == info.type)
	    return result_multi(data);

        if (property_type::number == info.type)
            return result(string::number(convert(*reinterpret_cast<const u32 *>(data.data()))));
    }

    const static QRegularExpression cells_regexp("^#.*-cells$");
    const static QRegularExpression names_regexp("^.*-names");

    if (cells_regexp.match(name).hasMatch())
        return result(string::number(convert(*reinterpret_cast<const u32 *>(data.data()))));

    if (names_regexp.match(name).hasMatch()) {
        return result_multi(data);
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
    qt_tree_fdt_generator generator(m_tree[id], m_target, std::move(name), std::move(id));

    std::vector<fdt_handle_special_property> handle_special_properties;

    fdt_handle_special_property handle_inner_dt;
    handle_inner_dt.name = "data";
    handle_inner_dt.callback = [&handle_special_properties](const fdt_property &property, iface_fdt_generator &generator) {
        fdt_parser(property.data.data(), property.data.size(), generator, property.name, handle_special_properties);
    };

    handle_special_properties.emplace_back(std::move(handle_inner_dt));

    fdt_parser parser(datamap.data(), datamap.size(), generator, {}, handle_special_properties);

    if (!parser.is_valid()) {
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

        const auto property = item->data(0, QT_ROLE_PROPERTY).value<fdt_property>();
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
        const auto property = item->data(0, QT_ROLE_PROPERTY).value<fdt_property>();
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
