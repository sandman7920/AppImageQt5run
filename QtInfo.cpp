#include "QtInfo.h"
#include <dlfcn.h>
#include <QCoreApplication>
#include <QLibraryInfo>
#include <iostream>

#define DBG(a) if (debug) std::cerr << a

Info QtInfo::getInfo(const std::string &appPath, const char *exe, bool debug) {
    Info info;
    std::string system_lib;
    std::string system_plugins;
    int system_version = 0;

    dlerror(); // reset errors
    int local_version = getLocalQtVersion(appPath, debug);

    DBG("load libQt5Core.so.5 system -> ");
    void *handle = dlopen("libQt5Core.so.5", RTLD_NOW | RTLD_GLOBAL); // LD_PRELOAD
    if (handle != nullptr) {
        char origin[2048];
        if (dlinfo(handle, RTLD_DI_ORIGIN, origin) == 0) {
            const char *version = qVersion();
            DBG("OK ") << origin << "/libQt5Core.so.5 (" << version << ')' << std::endl;
            system_version = getIntQtVersion(version);
            system_lib = origin;
            system_plugins = QLibraryInfo::location(QLibraryInfo::PluginsPath).toStdString();
        }
        dlclose(handle);
    } else {
        char *err = dlerror();
        if (debug) {
            std::cerr << "FAILED";
            if (err) std::cerr << ' ' << err;
            std::cerr << std::endl;
        }
    }



    bool use_system = false;
    FILE *always_local = fopen(std::string(appPath).append("/qt5.always_local").c_str(), "r");
    if (always_local == nullptr) {
        if (system_version > local_version) {
            DBG("local Qt5 version is lower than system\nuse system (check dependencies)") << std::endl;
            use_system = checkDeps(system_lib, exe, debug);
            if (debug) {
                if (use_system) {
                    std::cerr << "yes" << std::endl;
                } else {
                    std::cerr << "no (missing Qt libraries in system path)" << std::endl;
                }
            }
        }
    } else {
        DBG("qt5.always_local exists local Qt version will be used") << std::endl;
        fclose(always_local);
    }

    if (use_system) {
        info.ld_path.append(system_lib).append(":").append(std::string(appPath).append("/../lib"));
        info.qt_plugins.append(system_plugins).append(":").append(std::string(appPath).append("/../plugins"));
    } else {
        info.ld_path.append(std::string(appPath).append("/../lib"));
        info.qt_plugins.append(std::string(appPath).append("/../plugins"));
        if (!system_plugins.empty()) info.qt_plugins.append(":").append(system_plugins);
    }

    return info;
}

bool QtInfo::checkDeps(const std::string &system_lib, const char *exe, bool debug) {
    int state = 1;
    std::string cmd("LD_TRACE_LOADED_OBJECTS=1");
    cmd.append(" LD_LIBRARY_PATH=").append(system_lib)
       .append(" LD_BIND_NOW=1 ")
       .append(exe).append(" 2>&1");

    FILE *ldd = popen(cmd.c_str(), "r");
    if (ldd) {
        state = 0;
        char *path;
        char line[1024];
        while (!feof(ldd) && fgets(line, 1024, ldd)) {
            if (strstr(line, "Qt5") == nullptr) continue;
            DBG(line);
            if (state == 0 && (path = strstr(line, "/"))) {
                state = strncmp(system_lib.c_str(), path, system_lib.length());
            } else state = 1;
        }
        state += pclose(ldd);
    }
    return state == 0;
}

int QtInfo::getLocalQtVersion(const std::string &appPath, bool debug) {
    int result = 0;
    DBG("load libQt5Core.so.5  local -> ");
    dlerror(); // reset errors
    const std::string lib = appPath + "/../lib/libQt5Core.so.5";
    void *handle = dlopen(lib.c_str(), RTLD_LAZY | RTLD_LOCAL);
    if (handle != nullptr) {
        void *p = dlsym(handle, "qVersion");
        char origin[2048];
        if (p != nullptr && dlinfo(handle, RTLD_DI_ORIGIN, origin) == 0) {
            auto qVersion = reinterpret_cast<const char*(*)()>(p);
            const char *version = qVersion();
            DBG("OK ") << origin << "/libQt5Core.so.5 (" << version << ')' << std::endl;
            result = getIntQtVersion(version);
        }
        dlclose(handle);
    } else {
        char *err = dlerror();
        if (debug) {
            std::cerr << "FAILED";
            if (err) std::cerr << ' ' << err;
            std::cerr << std::endl;
        }
    }
    return result;
}


#define STATE_MAJOR 0
#define STATE_MINOR 1
#define STATE_PATCH 2
int QtInfo::getIntQtVersion(const char *qtVersion) {
    if (qtVersion == nullptr) return 0;

    int state = STATE_MAJOR;
    int major = 0, minor = 0, patch = 0;

    const char *v = qtVersion;
    while (*v) {
        switch (state) {
        case STATE_MAJOR:
            if (*v != '.') {
                major = major * 10 + (*v - '0');
            } else {
                state = STATE_MINOR;
            }
            break;
        case STATE_MINOR:
            if (*v != '.') {
                minor = minor * 10 + (*v - '0');
            } else {
                state = STATE_PATCH;
            }
            break;
        case STATE_PATCH:
            if (*v != '.') {
                patch = patch * 10 + (*v - '0');
            }
            break;
        }
        v++;
    }

    return  major << 8 | minor; // exclude patch
//    return  major << 16 | minor << 8 | patch;
}
