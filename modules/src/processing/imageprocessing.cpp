#include "imageprocessing.h"
#include "img/chunk2d.h"
#include "mat/matrix.h"

namespace CTL {
namespace imgproc {

// Implementation details
// ----------------------

void cosWeighting(Chunk2D<float>& proj, const mat::Matrix<3,3>& K)
{
    auto cosineOfConeAngle = [&K] (double x, double y) {
        // back substitution to find 'd' in K*d = [x,y,1]^t
        mat::Matrix<3, 1> direction;
        direction.get<2>() = 1.0;
        direction.get<1>() = (y - K.get<1,2>()) / K.get<1,1>();
        direction.get<0>() = (x - direction.get<1>()*K.get<0,1>() - K.get<0,2>()) / K.get<0,0>();

        // cosine to z-axis = <unitDirection, [0 0 1]^t>
        return float(direction.get<2>() / direction.norm());
    };

    const uint xSize = proj.width();
    const uint ySize = proj.height();
    uint x, y;
    for(x = 0; x < xSize; ++x)
        for(y = 0; y < ySize; ++y)
            proj(x,y) *= cosineOfConeAngle(x, y);
}

} // namespace imgproc
} // namespace CTL
