#ifndef COORDINATES_H
#define COORDINATES_H

#include <algorithm>
#include <vector>

typedef unsigned int uint;

namespace CTL {

struct Generic2DCoord
{
    Generic2DCoord() = default;
    Generic2DCoord(float coord1, float coord2) : data{ coord1, coord2 } {}

    float& coord1() { return data[0]; }
    float& coord2() { return data[1]; }
    const float& coord1() const { return data[0]; }
    const float& coord2() const { return data[1]; }

    float data[2];
};

struct Generic3DCoord
{
    Generic3DCoord() = default;
    Generic3DCoord(float coord1, float coord2, float coord3) : data{ coord1, coord2, coord3 } {}

    float& coord1() { return data[0]; }
    float& coord2() { return data[1]; }
    float& coord3() { return data[2]; }
    const float& coord1() const { return data[0]; }
    const float& coord2() const { return data[1]; }
    const float& coord3() const { return data[2]; }

    float data[3];
};


// Range
template<typename T>
class Range
{
public:
    // ctor
    template<typename T1, typename T2,
             typename = typename std::enable_if<std::is_convertible<T1, T>::value &&
                                                std::is_convertible<T2, T>::value>::type>
    Range(T1 start, T2 end) : data{ static_cast<T>(start), static_cast<T>(end) }
    { }

    // start and end of the range
    T& start() { return data[0]; }
    T& end() { return data[1]; }
    const T& start() const { return data[0]; }
    const T& end() const { return data[1]; }

    // width of the range
    T width() const { return data[1] - data[0]; }

    // spacing of the resulting linspace vector for given number of samples
    T spacing(uint nbSamples) const
    {
        return (nbSamples > 1) ? (data[1] - data[0]) / T(nbSamples - 1)
                               : T(0);
    }
    // center
    T center() const { return (data[0] + data[1]) / T(2); }

    // linspace
    std::vector<T> linspace(uint nbSamples) const { return linspace(data[0], data[1], nbSamples); }
    // static linspace
    static std::vector<T> linspace(T from, T to, uint nbSamples)
    {
        std::vector<T> ret(nbSamples);
        const T increment = (nbSamples > 1) ? (to - from) / T(nbSamples - 1)
                                            : T(0);
        uint idx = 0;
        std::generate(ret.begin(), ret.end(), [&idx, from, increment]{ return from + T(idx++) * increment; });
        return ret;
    }

private:
    // data member (start and end)
    T data[2];
};

typedef Range<float> SamplingRange;

} // namespace CTL

#endif // COORDINATES_H
