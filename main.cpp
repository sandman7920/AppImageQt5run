#include <QCoreApplication>
#include <QStringList>
#include <iostream>
#include <string>
#include <unistd.h>

int main(int /*argc*/, char *argv[], char **envp) {
    char *real_path;
    char *appDir = getenv("APPDIR");

    if (appDir != nullptr) {
        real_path = realpath(std::string(appDir).append("/AppRun").c_str(), nullptr);
    } else {
        real_path = realpath(argv[0], nullptr);
    }

    if (real_path == nullptr) return 1;

    const std::string self(real_path);

    size_t pos = self.find(".qt5run");
    if (pos == std::string::npos) return 1;

    real_path[pos] = '\0';
    argv[0] = real_path;

    pos = self.find_last_of('/');
    const std::string appPath = (pos != std::string::npos) ? self.substr(0, pos) : "";

    std::string qt_plugins = std::string(appPath).append("/../plugins");
    for (auto const &qstring: QCoreApplication::libraryPaths()) {
        qt_plugins.push_back(':');
        qt_plugins.append(qstring.toStdString());
    }

    setenv("QT_PLUGIN_PATH", qt_plugins.c_str(), 1);

    if (getenv("QT5_DEBUG_RUN") != nullptr) {
        std::cerr << "QT_PLUGIN_PATH: " << qt_plugins << '\n'
                  << "EXECUTABLE: " << argv[0] << std::endl;
    }

    return execve(argv[0], argv, envp);
}
