#pragma once

#include <types.hpp>

#include <QMainWindow>
#include <memory>

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

private:
    std::unique_ptr<Ui::MainWindow> m_ui;
};

} // namespace Window
