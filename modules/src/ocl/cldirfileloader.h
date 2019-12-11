#ifndef CLDIRFILELOADER_H
#define CLDIRFILELOADER_H

#include <string>

namespace CTL {
namespace OCL {

class ClDirFileLoader
{
public:
    ClDirFileLoader() = default;
    explicit ClDirFileLoader(const char* fileName);
    explicit ClDirFileLoader(const std::string& fileName);
    ClDirFileLoader(const char* fileName, const char* dir);
    ClDirFileLoader(const std::string& fileName, const std::string& dir);

    void setFileName(const char* fileName);
    void setFileName(const std::string& fileName);
    void setDir(const char* dir);
    void setDir(const std::string& dir);

    const std::string& fileName() const;
    const std::string& dir() const;
    bool isValid() const;
    std::string loadSourceCode() const;

private:
    std::string _fn;
    std::string _dir;
};

} // namespace OCL
} // namespace CTL

#endif // CLDIRFILELOADER_H