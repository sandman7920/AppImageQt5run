#ifndef QTINFO_H
#define QTINFO_H

#include <string>

struct Info {
    std::string ld_path{""};
    std::string qt_plugins{""};
};

class QtInfo {
public:
    static Info getInfo(const std::string &appPath, const char *exe, bool debug);

private:
    static int getLocalQtVersion(const std::string &appPath, bool debug);
    static int getIntQtVersion(const char *qtVersion);
    static bool checkDeps(const std::string &system_lib, const char *exe, bool debug);
};

#endif // QTINFO_H
