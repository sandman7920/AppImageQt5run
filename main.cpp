#include <QCoreApplication>
#include <QStringList>
#include <iostream>
#include <string>
#include <dlfcn.h>
#include <unistd.h>

#define DBG(a) if (debug) std::cerr << a

int main(int /*argc*/, char *argv[]) {
    bool debug = getenv("QT5_DEBUG_RUN") != nullptr;
    char *real_path;
    char *appDir = getenv("APPDIR");

    if (appDir != nullptr) {
        real_path = realpath(std::string(appDir).append("/AppRun").c_str(), nullptr);
    } else {
        real_path = realpath(argv[0], nullptr);
    }

    if (real_path == nullptr) return 1;

    if (getenv("QT_EXISTS") == nullptr) {
        DBG("First run load system libQt5Core.so.5: ");
        void *handle = dlopen("libQt5Core.so.5", RTLD_LAZY);
        if (handle != nullptr) {
            dlclose(handle);
            setenv("QT_EXISTS", "yes", 1);
            setenv("LD_PRELOAD", "libQt5Core.so.5", 1);
            DBG("loaded\nSet LD_PRELOAD=libQt5Core.so.5\n");
        } else {
            DBG("not loaded\n");
            setenv("QT_EXISTS", "no", 1);
        }
        DBG("restart") << std::endl;
        return execve(real_path, argv, environ);
    }
    unsetenv("LD_PRELOAD");

    const std::string self(real_path);

    size_t pos = self.find(".qt5run");
    if (pos == std::string::npos) return 1;

    real_path[pos] = '\0';
    argv[0] = real_path;

    pos = self.find_last_of('/');
    const std::string appPath = (pos != std::string::npos) ? self.substr(0, pos) : "";

    std::string qt_plugins = std::string(appPath).append("/../plugins");
    char *QT_EXISTS = getenv("QT_EXISTS");
    if (QT_EXISTS && strncmp(QT_EXISTS, "yes", 3) == 0) {
        for (auto const &qstring: QCoreApplication::libraryPaths()) {
            qt_plugins.push_back(':');
            qt_plugins.append(qstring.toStdString());
        }
    }
    unsetenv("QT_EXISTS");

    setenv("QT_PLUGIN_PATH", qt_plugins.c_str(), 1);

    if (debug) {
        std::cerr << "QT_PLUGIN_PATH: " << qt_plugins << '\n'
                  << "EXECUTABLE: " << argv[0] << std::endl;
    }

    return execve(argv[0], argv, environ);
}
