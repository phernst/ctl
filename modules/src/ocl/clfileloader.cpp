#include "clfileloader.h"

#include <QCoreApplication>
#include <QFileInfo>
#include <QString>

static QString determineOpenCLSourceDir();
static void ensureProperEnding(QString& path);

namespace CTL {
namespace OCL {

QString ClFileLoader::_oclSourceDir;

// constructors
/*!
 * Constructs a CLFileLoader object using \a fileName as the path of the .cl file.
 * \sa setFileName()
 */
ClFileLoader::ClFileLoader(const char* fileName)
    : _fn(fileName)
{
}

/*!
 * Constructs a CLFileLoader object using \a fileName as the path of the .cl file.
 * \sa setFileName()
 */
ClFileLoader::ClFileLoader(std::string fileName)
    : _fn(std::move(fileName))
{
}

/*!
 * Constructs a CLFileLoader object using \a fileName as the path of the .cl file.
 * \sa setFileName()
 */
ClFileLoader::ClFileLoader(const QString& fileName)
    : _fn(fileName.toStdString())
{
}

// setter
/*!
 * \sa setFileName(std::string fileName)
 */
void ClFileLoader::setFileName(const char* fileName) { _fn = fileName; }

/*!
 * (Re)sets the path of the .cl file. The \a fileName shall be passed to this method by the relative
 * path wrt the "cl_src" directory.
 * \sa fileName()
 */
void ClFileLoader::setFileName(std::string fileName) { _fn = std::move(fileName); }

/*!
 * \sa setFileName(std::string fileName)
 */
void ClFileLoader::setFileName(const QString& fileName) { _fn = fileName.toStdString(); }

// getter
/*!
 * Returns the relative path within the "cl_src" directory to the .cl file set by setFileName()
 * or a constructor.
 */
const std::string& ClFileLoader::fileName() const { return _fn; }

/*!
 * Returns true if the .cl file is readable, otherwise false.
 */
bool ClFileLoader::isValid() const
{
    if(_fn.empty())
        return false;

    QFileInfo fi(absoluteOpenCLSourceDir() + QString::fromStdString(_fn));
    return fi.isReadable();
}

/*!
 * Returns the content of the .cl file as a std::string.
 */
std::string ClFileLoader::loadSourceCode() const
{
    QFile file(absoluteOpenCLSourceDir() + QString::fromStdString(_fn));
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return {};

    return file.readAll().toStdString();
}

void ClFileLoader::setOpenCLSourceDir(const char* path)
{
    setOpenCLSourceDir(QString(path));
}

void ClFileLoader::setOpenCLSourceDir(const QString& path)
{
    _oclSourceDir = path;
    ensureProperEnding(_oclSourceDir);
}

void ClFileLoader::setOpenCLSourceDir(QString&& path)
{
    _oclSourceDir = std::move(path);
    ensureProperEnding(_oclSourceDir);
}

void ClFileLoader::setOpenCLSourceDir(const std::string& path)
{
    setOpenCLSourceDir(QString::fromStdString(path));
}

const QString& ClFileLoader::openCLSourceDir()
{
    return absoluteOpenCLSourceDir();
}

/*!
 * Returns the absolute path to the "ocl_src" directory in a platform-independent way.
 */
const QString& ClFileLoader::absoluteOpenCLSourceDir()
{
    if(_oclSourceDir.isEmpty())
        _oclSourceDir = determineOpenCLSourceDir();

    return _oclSourceDir;
}

} // namespace OCL
} // namespace CTL

QString determineOpenCLSourceDir()
{
    QString ret = QCoreApplication::applicationDirPath();
    ensureProperEnding(ret);
    ret += QStringLiteral("cl_src/");
    return ret;
}

static void ensureProperEnding(QString& path)
{
    if(!path.isEmpty() &&
       !path.endsWith(QStringLiteral("/")) &&
       !path.endsWith(QStringLiteral("\\")))
        path += QStringLiteral("/");
}
