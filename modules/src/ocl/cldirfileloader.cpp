#include "cldirfileloader.h"
#include "openclconfig.h"

#include <fstream>
#include <iostream>

namespace CTL {
namespace OCL {

// constructors
/*!
 * Constructs a ClDirFileLoader object using \a fileName as the path of the .cl file.
 * \sa setFileName()
 */
ClDirFileLoader::ClDirFileLoader(const char* fileName, const char* dir)
    : _fn(fileName), _dir(dir)
{
}

ClDirFileLoader::ClDirFileLoader(const char* fileName)
    : _fn(fileName), _dir(OpenCLConfig::instance().getKernelFileDir())
{
}

/*!
 * Constructs a ClDirFileLoader object using \a fileName as the path of the .cl file.
 * \sa setFileName()
 */
ClDirFileLoader::ClDirFileLoader(const std::string& fileName, const std::string& dir)
    : _fn(fileName), _dir(dir)
{
}

ClDirFileLoader::ClDirFileLoader(const std::string& fileName)
    : _fn(fileName), _dir(OpenCLConfig::instance().getKernelFileDir())
{
}

// setter
/*!
 * \sa setFileName(std::string fileName)
 */
void ClDirFileLoader::setFileName(const char* fileName) { _fn = fileName; }

/*!
 * \sa setFileName(std::string fileName)
 */
void ClDirFileLoader::setDir(const char* dir) { _dir = dir; }

/*!
 * (Re)sets the path of the .cl file. The \a fileName shall be passed to this method by the relative
 * path wrt the initialized \a path.
 * \sa fileName()
 */
void ClDirFileLoader::setFileName(const std::string& fileName) { _fn = fileName; }

/*!
 * (Re)sets the \a path.
 * \sa path()
 */
void ClDirFileLoader::setDir(const std::string& dir) { _dir = dir; }

// getter
/*!
 * Returns the relative path within the \a path directory to the .cl file set by setFileName()
 * or a constructor.
 */
const std::string& ClDirFileLoader::fileName() const { return _fn; }

const std::string& ClDirFileLoader::dir() const { return _dir; }

/*!
 * Returns true if the .cl file is readable, otherwise false.
 */
bool ClDirFileLoader::isValid() const
{
    if(_fn.empty())
        return false;

    auto fi { std::ifstream(_dir + "/" + _fn) };
    return fi.good();
}

/*!
 * Returns the content of the .cl file as a std::string.
 */
std::string ClDirFileLoader::loadSourceCode() const
{
    auto file {std::ifstream(_dir + "/" + _fn)};
    if(file.bad())
        return {};

    return {std::istreambuf_iterator<char>(file),
            std::istreambuf_iterator<char>()};
}

} // namespace OCL
} // namespace CTL
