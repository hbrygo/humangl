#ifndef FILESYSTEM_HPP
#define FILESYSTEM_HPP

#include <string>

class FileSystem {
public:
    // In the tutorial this resolves path; here return the input unchanged.
    static std::string getPath(const std::string &relativePath) {
        return relativePath;
    }
};

#endif
