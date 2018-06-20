#include <QCoreApplication>
#include <QStringList>
#include <iostream>
#include <string>
#include <dlfcn.h>
#include <unistd.h>
#include <locale>
#include <codecvt>

#define DBG(a) if (debug) std::cerr << a

int main(int /*argc*/, char *argv[]) {
    char *real_path;
    char *appDir = getenv("APPDIR");

    if (appDir != nullptr) {
        real_path = realpath(std::string(appDir).append("/AppRun").c_str(), nullptr);
    } else {
        real_path = realpath(argv[0], nullptr);
    }

    const std::string self(real_path);

    size_t pos = self.find(".qt5run");
    if (pos == std::string::npos) return 1;

    real_path[pos] = '\0';
    argv[0] = real_path;

    pos = self.find_last_of('/');
    const std::string appPath = (pos != std::string::npos) ? self.substr(0, pos) : "";

    std::string qt_plugins = std::string(appPath).append("/../plugins");
    void *handle = dlopen("libQt5Core.so.5", RTLD_LAZY);
    if (handle != nullptr) {
        QStringList (*libraryPaths)();
        *(void **) (&libraryPaths) = dlsym(handle, "_ZN16QCoreApplication12libraryPathsEv");
        std::wstring str;
        for (auto &s: (*libraryPaths)().toStdList()) {
            qt_plugins.push_back(':');
            auto p = s.data_ptr();
            str.clear();
            str.append(p->begin(), p->end());
            qt_plugins.append(std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(str));
        }
        dlclose(handle);
    }

    setenv("QT_PLUGIN_PATH", qt_plugins.c_str(), 1);

    if (getenv("QT5_DEBUG_RUN")) {
        std::cerr << "QT_PLUGIN_PATH: " << qt_plugins << '\n'
                  << "EXECUTABLE: " << argv[0] << std::endl;
    }

    return execve(argv[0], argv, environ);
}
