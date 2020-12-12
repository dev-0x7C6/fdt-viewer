#include "main-window.hpp"

#include <QApplication>

#include <config.h>

int main(int argc, char *argv[]) {
    const auto major = QString::number(PROJECT_VERSION_MAJOR);
    const auto minor = QString::number(PROJECT_VERSION_MINOR);
    const auto patch = QString::number(PROJECT_VERSION_PATCH);
    const auto version_string = major + "." + minor + "." + patch;

    QApplication application(argc, argv);
    application.setApplicationName(PROJECT_NAME);
    application.setApplicationVersion(version_string);
    application.setApplicationDisplayName(QString("Flattened Device Tree Viewer %1").arg(version_string));
    Window::MainWindow w;
    w.show();
    return application.exec();
}
