#ifndef LSBRELEASE_H
#define LSBRELEASE_H

#include <string>

struct LSBRelease {
    LSBRelease();
    std::string id{};
    std::string release{};
    std::string code_name{};
    std::string description{};
};

#endif // LSBRELEASE_H
