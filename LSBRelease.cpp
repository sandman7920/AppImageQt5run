#include "LSBRelease.h"

#include <cstdio>
#include <cstring>

LSBRelease::LSBRelease() {
    FILE *fp = fopen("/etc/lsb-release", "rb");
    if (fp == nullptr)
        return;

    char key[128];
    char value[128];

    key[127] = value[127] = 0;

    while (fscanf(fp, "%127[^=]=%127[^\n]\n", key, value) == 2) {
        if (strncmp(key, "DISTRIB_ID", 128) == 0) {
            id = value;
        } else if (strncmp(key, "DISTRIB_RELEASE", 128) == 0) {
            release = value;
        } else if (strncmp(key, "DISTRIB_CODENAME", 128) == 0) {
            code_name = value;
        } else if (strncmp(key, "DISTRIB_DESCRIPTION", 128) == 0) {
            description = value;
            if (description.front() == '"' && description.back() == '"') {
                description.erase(0, 1);
                description.pop_back();
            }
        }
    }

    fclose(fp);
}
