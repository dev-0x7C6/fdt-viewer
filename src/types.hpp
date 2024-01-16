#pragma once

#include <cstdint>
#include <QStringList>
#include <QRegularExpression>

using i16 = std::int16_t;
using i32 = std::int32_t;
using i64 = std::int64_t;
using i8 = std::int8_t;

using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;
using u8 = std::uint8_t;

class QAction;
class QActionGroup;
class QMenuBar;
class QByteArray;
class QDirIterator;
class QFile;
class QFileInfo;
class QRegExp;
class QString;
class QTreeWidget;
class QTreeWidgetItem;
class QWidget;

using action = QAction;
using menu_bar = QMenuBar;
using action_group = QActionGroup;
using byte_array = QByteArray;
using dir_iterator = QDirIterator;
using file = QFile;
using file_info = QFileInfo;
using string = QString;
using string_list = QStringList;
using tree_widget = QTreeWidget;
using tree_widget_item = QTreeWidgetItem;
using widget = QWidget;
