/******************************************************************************
** # DEN - Delightful Easy Norm
** VERSION 1.5.2
**
** <<< Fast version of DFileFormat specialized in memory block containers >>>
**
** DFileFormat template class (a DEN - file format Handler)
** for standard Container<typename> which provide the resource in one memory
** block that can be accessed by a pointer from "data()" member function
** (such as common vector types).
**
** Definition of DEN format only allows following types:
**  unsigned char    (8 Bit)
**  unsigned short  (16 Bit)
**  float           (32 Bit)
**  double          (64 Bit)
**
** Predefined specialisation in namespace den::
**  typedef DFileFormat<std_vector> DFile; // for std::vector
**  typedef DFileFormat<QVector> QDFile;   // for QVector
**
** example/usage:
** --- SAVE ---
**  std::vector<float> vec(1337,42.0f);
**  den::DFile file("myFile.den");
**  file.save(vec);
** or for another header
**  file.save(vec,7,191); // or file.save(vec,7,191,1);
** creates one 7x191 matrix.
** or
**  file.save(vec,7,1);
** creates 191 7x1 vectors.
**
** --- LOAD ---
**  den::QDFile file("myFile.den");
**  auto s = file.loadUShort(); // creates a QVector<ushort>
**  std::cout << file.rows() << file.columns() << file.count() << std::endl;
** or as a free function
**  auto v1 = den::loadFloat<QVector>("float.den"); // returns a QVector<float>
**
** The den:: namespace provides also a save function with auto type deduction
** of the used container. So you can simply write e.g.
**  QVector<double> v = {1.0,3.0,3.0,7.0};
**  den::save(v, "autoType.den");
**
** --- OUTPUT ---
** You can disable outputs (except errors) by calling the verbose mode setter:
**  myDFile.setVerbose(false);
** In order to customize the output, you can reimplement the virtual method
**  virtual void callBack(const char *output, int code = 0, int numSteps = 0) const
** by inheriting your custom DFileFormat class (including the behavior in
** (non-)verbose mode). This is an exemplary snippet how it could look like:
**
**  template<template<typename> class C>
**  class MyDFileFormat : public DFileFormat<C> {
**  public:
**      MyDFileFormat() { }
**      template<class string>
**      MyDFileFormat(string&& s) : DFileFormat(std::forward<string>(s)) { }
**  protected:
**      void callBack(const char *output, int code = 0, int numSteps = 0) const override {
**          if(verbose()) // check verbose mode
**              std::cout << "my dennerlein IO | " << output << std::endl;
**      }
**  };
**
** Further information about a given file can be obtained by using the following functions
** namespace den {
** Header loadHeader(S&& fileName, size_t* bytesData = nullptr)
** Type getDataType(S&& fileName)
** }
** "Type" is defined by the enumeration
** enum Type { unDef=0, uChar=1, uShort=2, Float=4, Double=8 };
**
**
** by Robert Frysch | Oct 08, 2018
** Otto von Guericke University Magdeburg
** Institute for Medical Engineering - IMT (Head: Georg Rose)
** Email: robert.frysch@ovgu.de
**
******************************************************************************/

#ifndef DFILEFORMAT_H
#define DFILEFORMAT_H

#include "den_header.h"
#include <vector>

/// @cond QT_INTRINSIC
template <typename T>
class QVector;
/// @endcond

typedef uint16_t ushort;
typedef unsigned char uchar;
typedef unsigned int uint;

namespace CTL {
namespace io {
namespace den {

template <template <typename> class Container>
class DFileFormat
{
public:
    DFileFormat();
    template <class S>
    DFileFormat(S&& fileName);

    // setter
    template <class S>
    void setFileName(S&& toSet);
    void setVerbose(bool toSet);

    // # constant functions #
    // getter
    const std::string& fileName() const { return fn; }
    const den::Header& header() const { return lastReadIn; }
    int count() const { return lastReadIn.count; }
    int rows() const { return lastReadIn.rows; }
    int columns() const { return lastReadIn.cols; }
    bool verbose() const { return v; }

    // save
    bool save(const Container<uchar>& toWrite, int rows = 0, int cols = 0, int count = 0) const;
    bool save(const Container<ushort>& toWrite, int rows = 0, int cols = 0, int count = 0) const;
    bool save(const Container<float>& toWrite, int rows = 0, int cols = 0, int count = 0) const;
    bool save(const Container<double>& toWrite, int rows = 0, int cols = 0, int count = 0) const;

    bool save(const Container<uchar>& toWrite, const den::Header& header) const;
    bool save(const Container<ushort>& toWrite, const den::Header& header) const;
    bool save(const Container<float>& toWrite, const den::Header& header) const;
    bool save(const Container<double>& toWrite, const den::Header& header) const;

    // # functions that update the internal stored header #
    // append
    bool appendMatrices(const Container<uchar>& toAppend, bool padding = false, uchar value = 0);
    bool appendMatrices(const Container<ushort>& toAppend, bool padding = false, ushort value = 0);
    bool appendMatrices(const Container<float>& toAppend, bool padding = false, float value = 0.0f);
    bool appendMatrices(const Container<double>& toAppend, bool padding = false, double value = 0.0);

    // load
    Container<uchar> loadUChar(uint startMatrix = 0, int numMatrices = -1);
    Container<ushort> loadUShort(uint startMatrix = 0, int numMatrices = -1);
    Container<float> loadFloat(uint startMatrix = 0, int numMatrices = -1);
    Container<double> loadDouble(uint startMatrix = 0, int numMatrices = -1);

    const den::Header& loadHeader();

protected:
    virtual void callBack(const char* output, int code = 0, int numSteps = 0) const;

private: // methods
    template <typename T>
    bool gen_save(const Container<T>& toWrite, int rows, int cols, int count) const;
    template <typename T>
    bool gen_append(const Container<T>& toAppend, bool padding, T value);
    template <typename T>
    Container<T> gen_load(uint startMatrix, int numMatrices);

    // auxiliary function
    template <typename T>
    const T& minimum(const T& a, const T& b) const { return (a < b) ? a : b; }

private: // members
    std::string fn; // path to DEN file
    den::Header lastReadIn; // header information of the last loaded/appended file
    bool v{ true }; // verbose mode
};

// type declarations for convenience:
template <class T>
using std_vector = std::vector<T, std::allocator<T>>;

typedef DFileFormat<std_vector> DFile;
typedef DFileFormat<QVector> QDFile;

} // namespace den
} // namespace io
} // namespace CTL

#include "dfileformat.tpp"

#endif // DFILEFORMAT_H
