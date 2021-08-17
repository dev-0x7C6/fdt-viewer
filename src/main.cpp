#include "main-window.hpp"

#include <QApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QFileInfo>
#include <QSettings>

#include <config.h>

int main(int argc, char *argv[]) {
    const auto major = QString::number(PROJECT_VERSION_MAJOR);
    const auto minor = QString::number(PROJECT_VERSION_MINOR);
    const auto patch = QString::number(PROJECT_VERSION_PATCH);
    const auto version_string = major + "." + minor + "." + patch;

    QApplication application(argc, argv);

    application.setOrganizationName(PROJECT_NAME);
    application.setApplicationName(PROJECT_NAME);
    application.setApplicationVersion(version_string);
    application.setApplicationDisplayName(QString("Flattened Device Tree Viewer %1").arg(version_string));

    QSettings settings;

    QCommandLineParser parser;
    QCommandLineOption file_option{{"f", "file"}, QCoreApplication::translate("main", "open file."), "file"};
    QCommandLineOption dir_option{{"d", "directory"}, QCoreApplication::translate("main", "open directory."), "directory"};
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addOptions({file_option, dir_option});

    parser.process(application);

    Window::MainWindow window;
    window.show();

    bool no_parameters = !parser.isSet(dir_option) && !parser.isSet(file_option);

    if (parser.isSet(dir_option))
        window.open_directory(parser.value(dir_option));

    if (parser.isSet(file_option))
        window.open_file(parser.value(file_option));

    if (no_parameters) {
        auto args = application.arguments();
        args.removeFirst();
        for (auto &&path : args) {
            auto info = QFileInfo{path};
            if (info.isDir())
                window.open_directory(path);

            if (info.isFile())
                window.open_file(path);
        }
    }

    return application.exec();
}
