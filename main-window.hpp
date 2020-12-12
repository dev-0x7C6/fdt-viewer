#pragma once

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

    void open_dialog();
    bool open(const QString &path);

private:
    void update_fdt_path(QTreeWidgetItem *item = nullptr);

private:
    std::unique_ptr<Ui::MainWindow> m_ui;
};

} // namespace Window
