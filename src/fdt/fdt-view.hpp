#pragma once

#include <fdt/fdt-generator-qt.hpp>

class QString;
class QByteArray;
class QFileInfo;

namespace fdt {

class viewer {
public:
    viewer(QTreeWidget *target);

    auto is_loaded(QString &&id) const noexcept -> bool;
    auto is_loaded(const QString &id) const noexcept -> bool;

    auto load(QByteArray &&data, QString &&name, QString &&id) -> bool;
    auto drop(QString &&id) -> void;

private:
    tree_map m_tree;
    QTreeWidget *m_target;
};

bool fdt_view_prepare(QTreeWidget *target, const QByteArray &datamap, const QFileInfo &info);
bool fdt_view_dts(QTreeWidgetItem *item, QString &ret, int depth = 0);
bool fdt_content_filter(QTreeWidgetItem *item, const std::function<bool(const QString &)> &match);

} // namespace fdt
