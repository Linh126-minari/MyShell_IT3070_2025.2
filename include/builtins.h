#ifndef BUILTINS_H
#define BUILTINS_H

#include <string>
#include <vector>

namespace Builtins {
    bool handleBuiltin(const std::vector<std::string>& args, bool isBackground);
}

#endif

