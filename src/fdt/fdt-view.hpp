#pragma once

#include <fdt/fdt-generator-qt.hpp>

namespace fdt {

class viewer {
public:
    viewer(tree_widget *target);

    auto is_loaded(string &&id) const noexcept -> bool;
    auto is_loaded(const string &id) const noexcept -> bool;

    auto load(QByteArray &&data, string &&name, string &&id) -> bool;
    auto drop(string &&id) -> void;

private:
    tree_map m_tree;
    tree_widget *m_target;
};

bool fdt_view_prepare(tree_widget *target, const byte_array &datamap, const file_info &info);
bool fdt_view_dts(QTreeWidgetItem *item, string &ret, int depth = 0);
bool fdt_content_filter(QTreeWidgetItem *item, const std::function<bool(const string &)> &match);

} // namespace fdt
