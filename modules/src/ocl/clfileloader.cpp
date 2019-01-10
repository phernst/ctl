#include "clfileloader.h"

#include <QCoreApplication>
#include <QFileInfo>
#include <QString>

static QString determineOpenCLSourceDir();

namespace CTL {
namespace OCL {

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

/*!
 * Returns the absolute path to the "ocl_src" directory in a platform-independent way.
 */
const QString& ClFileLoader::absoluteOpenCLSourceDir() const
{
    static const QString ret = determineOpenCLSourceDir();
    return ret;
}

} // namespace OCL
} // namespace CTL

QString determineOpenCLSourceDir()
{
    QString ret = QCoreApplication::applicationDirPath();
    if(!ret.isEmpty() &&
       !ret.endsWith(QStringLiteral("/")) &&
       !ret.endsWith(QStringLiteral("\\")))
        ret += QStringLiteral("/");
    ret += QStringLiteral("cl_src/");
    return ret;
}
