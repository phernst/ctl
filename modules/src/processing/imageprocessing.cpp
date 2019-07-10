#include "imageprocessing.h"
#include "img/chunk2d.h"
#include "mat/matrix.h"
#include "img/voxelvolume.h"
#include <array>

namespace CTL {
namespace imgproc {

// Implementation details
// ----------------------

void cosWeighting(Chunk2D<float>& proj, const mat::Matrix<3,3>& K)
{
    const uint xSize  = proj.width();
    const uint ySize  = proj.height();
    const float cx = xSize / 2.0f;
    const float cy = ySize / 2.0f;

    //COS-WEIGHTING
    const float D   = 0.5f * (K(0,0) + K(1,1));
    const float Dsq = std::pow(D, 2.0);

    uint x, y;
    for(x = 0; x < xSize; ++x)
        for(uint y = 0; y < ySize; ++y)
            proj(x,y) *= D/std::sqrt(std::pow(x-K(0,2), 2.0) + std::pow(y-K(1,2), 2.0) + Dsq);
}


} // namespace imgproc
} // namespace CTL
