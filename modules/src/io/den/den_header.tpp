#include "den_header.h"
#include "den_tpl_deduct.h"
#include <fstream>

typedef unsigned short ushort;

namespace CTL {
namespace io {

// Header factory function
inline den::Header den::createHeader(size_t totSize, int rows, int cols, int count)
{
    den::Header ret(rows, cols, count);
    if(!ret.isZero())
    {
        if(ret != totSize)
            return den::Header();
    } else
    { /* any size is 0 */
        if(ret.rows == 0)
        {
            if(totSize <= INT_MAX)
                ret.rows = static_cast<int>(totSize), ret.cols = 1, ret.count = 1;
        } else
        { /* rows!=0 */
            if(ret.cols == 0)
            {
                if(totSize % ret.rows == 0)
                {
                    auto c = totSize / ret.rows;
                    if(c <= INT_MAX)
                        ret.cols = static_cast<int>(c), ret.count = 1;
                }
            } else
            { /* ret.cols!=0 && count==0*/
                auto matSize = static_cast<size_t>(ret.rows) * static_cast<size_t>(ret.cols);
                if(totSize % matSize == 0)
                {
                    auto c = totSize / matSize;
                    if(c <= INT_MAX)
                        ret.count = static_cast<int>(c);
                }
            }
        }
    }
    return ret;
}

// load only header information
template <class S>
den::Header den::loadHeader(S&& fileName, size_t* bytesData)
{
    Header ret;
    if(bytesData)
        *bytesData = 0; // init bytesData
    std::ifstream file(_tpl_deduct::toStdString(std::forward<S>(fileName)), std::ios::binary);
    if(!file.is_open()) // cannot open file
        return ret;
    ushort buf[3];
    file.read(reinterpret_cast<char*>(buf), 6);
    auto num = file.gcount();
    if(num >= 2)
        ret.rows = buf[0];
    if(num >= 4)
        ret.cols = buf[1];
    if(num == 6)
        ret.count = buf[2];

    if(bytesData && !file.fail())
    {
        auto fsize = file.tellg();
        file.seekg(0, file.end);
        *bytesData = file.tellg() - fsize;
    }
    file.close();
    return ret;
}

template <class S>
den::Header den::Header::fromFile(S&& fileName, size_t* bytesData)
{
    return loadHeader(std::forward<S>(fileName), bytesData);
}

} // namespace io
} // namespace CTL
