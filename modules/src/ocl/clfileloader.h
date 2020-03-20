#ifndef CTL_CLFILELOADER_H
#define CTL_CLFILELOADER_H

#include <string>

class QString;

namespace CTL {
namespace OCL {

/*!
 * \class ClFileLoader
 *
 * \brief The ClFileLoader class provides the ability to load files with OpenCL C source code
 *
 * OpenCL C source code needs to be available at runtime. Therefore, it will be collected in
 * .cl files within the "cl_src" directory, which is located in the path of the executable
 * (copied from the "ocl" folder of the repository by the qmake build system; see "ocl_config.pri").
 * The ClFileLoader provides the method loadSourceCode() that loads a file from the "cl_src" folder
 * and return the content as a std::string. The file name of a .cl file is passed by the constructor
 * or the setFileName() method as a relative path wrt the "cl_src" folder, e.g.
 * "projectors/external_raycaster.cl".
 * Moreover, the CLFileLoader allows to check beforehand if a certain .cl file is readable
 * by using isValid().
 *
 * NOTE: The internal determination of the absolute path of the "cl_src" folder can only work
 * correctly if the main function of the program instantiate a QCoreApplication (or derived classes)
 * as in the following snippet:
 * \code
 *   #include <QCoreApplication>
 *
 *   int main(int argc, char *argv[])
 *   {
 *       QCoreApplication a(argc, argv);
 *
 *       // your code...
 *   }
 * \endcode
 * If no QCoreApplication has been instantiated: \n
 * The CLFileLoader can only use the relative path to the OpenCL sources wrt to the executable.
 * This requires that the *current directory* (from where the program was started) is equal to the
 * directory of the executable. Should this not be the case, the CLFileLoader is not able to locate
 * the .cl files, i.e. isValid() will return `false` and loadSourceCode() will return an empty
 * string. Alternatively, the path to the "cl_src" folder can be set during run time using the
 * static method setOpenCLSourceDir().
 */
class ClFileLoader
{
    static QString _oclSourceDir;

public:
    explicit ClFileLoader() = default;
    explicit ClFileLoader(const char* fileName);
    explicit ClFileLoader(std::string fileName);
    explicit ClFileLoader(const QString& fileName);

    void setFileName(const char* fileName);
    void setFileName(std::string fileName);
    void setFileName(const QString& fileName);

    const std::string& fileName() const;
    bool isValid() const;
    std::string loadSourceCode() const;

    // static methods for adjusting the path of the OpenCL source files
    static void setOpenCLSourceDir(const char* path);
    static void setOpenCLSourceDir(const QString& path);
    static void setOpenCLSourceDir(QString&& path);
    static void setOpenCLSourceDir(const std::string& path);

    static const QString& openCLSourceDir();

private:
    std::string _fn;

    static const QString& absoluteOpenCLSourceDir();
};

} // namespace OCL
} // namespace CTL

#endif // CTL_CLFILELOADER_H
