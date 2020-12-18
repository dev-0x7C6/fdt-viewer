#pragma once

#include <types.hpp>

#include <QMainWindow>
#include <memory>

class QHexView;
class QTreeWidgetItem;

namespace Window {

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void open_file_dialog();
    void open_directory_dialog();

    void open_directory(const string &path);
    void open_file(const string &path);

    bool open(const string &path);

private:
    void update_fdt_path(QTreeWidgetItem *item = nullptr);
    void update_view();

private:
    std::unique_ptr<Ui::MainWindow> m_ui;
    tree_widget_item *m_fdt{nullptr};
    QHexView *m_hexview{nullptr};

private:
    action *m_file_close_action{nullptr};
    action *m_file_close_all_action{nullptr};
};

} // namespace Window
