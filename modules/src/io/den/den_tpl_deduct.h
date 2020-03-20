/* Template details / switches for DFileFormat template class */

#ifndef CTL_DEN_TPL_DEDUCT_H
#define CTL_DEN_TPL_DEDUCT_H

#include <string>

// forward declarations
class QString;
class QByteArray;

namespace CTL {
namespace io {
namespace den {

template <template<typename> class Container>
class DFileFormat;

/// @cond TEMPLATE_DETAILS
namespace _tpl_deduct {

    // wrapper for reducing std::vector<type,allocator> type to a container<type> type
    template<template<typename,class> class C, class Alloc>
    struct Wrapper {
        template<typename T>
        using red_type = C<T,Alloc>;
    };

    // save C<type>
    template<template<typename> class C, typename T, class S>
    bool save(const C<T> &toWrite, S&& fileName, int rows, int cols, int count) {
        DFileFormat<C> tmp(std::forward<S>(fileName));
        return tmp.save(toWrite,rows,cols,count);
    }
    // save C<type,allocator>
    template<template<typename,class> class C, typename T, class A, class S>
    bool save(const C<T,A> &toWrite, S&& fileName, int rows, int cols, int count) {
        DFileFormat< Wrapper<C,A>::template red_type > tmp(std::forward<S>(fileName));
        return tmp.save(toWrite,rows,cols,count);
    }
    // append C<type>
    template<template<typename> class C, typename T, class S>
    bool appendMatrices(S&& fileName, const C<T> &toAppend, bool padding, T value) {
        DFileFormat<C> tmp(std::forward<S>(fileName));
        return tmp.appendMatrices(toAppend, padding, value);
    }
    // append C<type,allocator>
    template<template<typename,class> class C, typename T, class A, class S>
    bool appendMatrices(S&& fileName, const C<T,A> &toAppend, bool padding, T value) {
        DFileFormat< Wrapper<C,A>::template red_type > tmp(std::forward<S>(fileName));
        return tmp.appendMatrices(toAppend, padding, value);
    }

    // generic string conversion
    // 1. STL compatible type: perfect forwarding of a reference
    template<class S>
    auto toStdString(S &&string)
    -> typename std::enable_if< std::is_convertible<S,std::string>::value
                              , S&& >::type
    {
        return std::forward<S>(string);
    }
    // 2. QString: return std::string using Qt method
    template<class S>
    auto toStdString(const S &string)
    -> typename std::enable_if< std::is_same<S,QString>::value
                              , std::string >::type
    {
        return string.toStdString();
    }
    // 3. QByteArray: return std::string using Qt method
    template<class S>
    auto toStdString(const S &string)
    -> typename std::enable_if< std::is_same<S,QByteArray>::value
                              , std::string >::type
    {
        return string.toStdString();
    }

} // namespace _tpl_deduct
/// @endcond
} // namespace den
} // namespace io
} // namespace CTL


#endif // CTL_DEN_TPL_DEDUCT_H
