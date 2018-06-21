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
    bool debug = getenv("QT5_DEBUG_RUN") != nullptr;
    char *LD_LIBRARY_PATH = getenv("LD_LIBRARY_PATH");
    if (LD_LIBRARY_PATH != nullptr && strlen(LD_LIBRARY_PATH) > 0) {
        DBG("LD_LIBRARY_PATH exists backup, clear and run again\n");
        setenv("OLD_LD_LIBRARY_PATH", LD_LIBRARY_PATH, 1);
        unsetenv("LD_LIBRARY_PATH");
        return execve(argv[0], argv, environ);
    }

    char *real_path;
    char *search_path;
    char *appDir = getenv("APPDIR");

    if (appDir != nullptr) {
        search_path = realpath(std::string(appDir).append("/AppRun").c_str(), nullptr);
    } else {
        search_path = realpath(argv[0], nullptr);
    }

    if (search_path == nullptr) return 1;
    real_path = search_path;

    const std::string self(real_path);

    size_t pos = self.find(".qt5run");
    if (pos == std::string::npos) return 1;

    real_path[pos] = '\0';
    argv[0] = real_path;

    pos = self.find_last_of('/');
    const std::string appPath = (pos != std::string::npos) ? self.substr(0, pos) : "";

    std::string qt_plugins("");
    search_path = realpath(std::string(appPath).append("/../plugins").c_str(), nullptr);
    if (search_path != nullptr) {
        qt_plugins.append(search_path);
        free(search_path);
    } else {
        std::cerr << "Warning " << std::string(appPath).append("/../plugins")
                  << " does not exists" << std::endl;
    }

    DBG("dlopen(libQt5Core.so.5) -> ");
    void *handle = dlopen("libQt5Core.so.5", RTLD_NOW | RTLD_GLOBAL);
    if (handle != nullptr) {
        Dl_info  DlInfo;
        void *p = dlsym(handle, "_ZN16QCoreApplication12libraryPathsEv");
        int nRet = dladdr(p, &DlInfo);
        if (nRet) {
            DBG("loaded ") << DlInfo.dli_fname << std::endl;

            QStringList (*libraryPaths)();
            *(void **) (&libraryPaths) = p;

            const QStringList &plugins = (*libraryPaths)();
            for (const auto &qstring: plugins) {
                if(!qt_plugins.empty()) qt_plugins.push_back(':');
                qt_plugins.append(qstring.toStdString());
            }
        } else {
            DBG("failed") << std::endl;
        }
        dlclose(handle);
    } else {
        DBG("does not exists in system") << std::endl;
    }
    setenv("QT_PLUGIN_PATH", qt_plugins.c_str(), 1);

    std::string ld_path("");;
    search_path = realpath(std::string(appPath).append("/../lib").c_str(), nullptr);
    if (search_path != nullptr) {
        ld_path.append(search_path);
        free(search_path);
    } else {
        std::cerr << "Warning " << std::string(appPath).append("/../lib")
                  << " does not exists" << std::endl;
    }

    char *OLD_LD_PATH = getenv("OLD_LD_LIBRARY_PATH");
    if (OLD_LD_PATH != nullptr) {
        if (!ld_path.empty()) ld_path.push_back(':');
        ld_path.append(OLD_LD_PATH);
    }
    unsetenv("OLD_LD_LIBRARY_PATH");
    setenv("LD_LIBRARY_PATH", ld_path.c_str(), 1);

    if (debug) {
        std::cerr << "LD_LIBRARY_PATH=" << ld_path << '\n'
                  << "QT_PLUGIN_PATH=" << qt_plugins << '\n'
                  << "EXECUTABLE " << argv[0] << std::endl;
    }

    return execve(argv[0], argv, environ);
}
