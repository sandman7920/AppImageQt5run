#include "LSBRelease.h"
#include "QtInfo.h"

#include <cstring>
#include <iostream>
#include <string>
#include <unistd.h>

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

    auto info = QtInfo::getInfo(appPath, real_path, debug);

    char *OLD_LD_PATH = getenv("OLD_LD_LIBRARY_PATH");
    if (OLD_LD_PATH != nullptr) {
        if (!info.ld_path.empty()) info.ld_path.push_back(':');
        info.ld_path.append(OLD_LD_PATH);
    }
    unsetenv("OLD_LD_LIBRARY_PATH");

    LSBRelease lsb_release;
    if (!lsb_release.code_name.empty()) {
        info.ld_path.push_back(':');
        info.ld_path.append(appPath).append("/../lib_").append(lsb_release.code_name);
    }

    if (debug) {
        std::cerr << "LD_LIBRARY_PATH=" << info.ld_path << '\n'
                  << "QT_PLUGIN_PATH=" << info.qt_plugins << '\n'
                  << "EXECUTABLE " << argv[0] << std::endl;
    }

    if (auto s = getenv("XDG_CURRENT_DESKTOP")) {
        std::string current_desktop(s);
        for (auto &c : current_desktop) {
            c = std::toupper(c);
        }
        if (current_desktop.find("KDE") == std::string::npos && current_desktop.find("LXQT") == std::string::npos) {
            char *qpa_theme = getenv("QT_QPA_PLATFORMTHEME");
            setenv("QT_QPA_PLATFORMTHEME", "gtk2", qpa_theme != nullptr && strncmp(qpa_theme, "appmenu-qt5", 11) == 0);
            if (debug) {
                std::cerr << "QT_QPA_PLATFORMTHEME=" << getenv("QT_QPA_PLATFORMTHEME") << std::endl;
            }
        }
    }

    setenv("LD_LIBRARY_PATH", info.ld_path.c_str(), 1);
    setenv("QT_PLUGIN_PATH", info.qt_plugins.c_str(), 1);

    return execve(argv[0], argv, environ);
}
