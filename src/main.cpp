#include "main-window.hpp"

#include <QApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
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
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addOptions({//
        {{"f", "file"}, QCoreApplication::translate("main", "open file."), QCoreApplication::translate("main", "file")},
        {{"d", "directory"}, QCoreApplication::translate("main", "open directory."), QCoreApplication::translate("main", "directory")}});

    parser.process(application);

    Window::MainWindow window;
    window.show();

    if (parser.isSet("directory"))
        window.open_directory(parser.value("directory"));

    if (parser.isSet("file"))
        window.open_file(parser.value("file"));

    return application.exec();
}
