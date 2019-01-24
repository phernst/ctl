#include "raycaster.h"
#include "mat/matrix_algorithm.h"
#include "ocl/clfileloader.h"
#include <iostream>
#include <string>
#include <vector>

const std::string CL_FILE_NAME = "projectors/external_raycaster.cl";

namespace CTL {
namespace OCL {

static cl_double16 decomposeM(const Matrix3x3& M);
static cl_float3 determineSource(const ProjectionMatrix& P);

RayCaster::RayCaster()
{
    initOpenCL();
}

std::vector<float> RayCaster::project(const std::vector<ProjectionMatrix>& Pmats,
                                      const std::vector<float>& volume)
{
    const size_t nbProjs = Pmats.size();
    const size_t sizeOfProj = detectorColumns * detectorRows;
    std::vector<float> ret(nbProjs * sizeOfProj);
    try // OpenCL exception handling
    {
        // Create command queue.
        cl::CommandQueue queue(context, device[0]);

        // Create kernel
        cl::Kernel kernel(program, "ray_caster");

        // Prepare input data.
        cl_float3 volCorner = volumeCorner();
        cl_float3 source;
        cl_double16 QR;

        // Allocate device buffers and transfer input data to device.
        cl_mem_flags readCopyFlag = CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR;
        cl::Buffer volCornerBuf(context, readCopyFlag, sizeof(volCorner), &volCorner);
        cl::Buffer voxelSizeBuf(context, readCopyFlag, sizeof(voxelSize), &voxelSize);
        cl::Buffer sourceBuf(context, CL_MEM_READ_ONLY, sizeof(source));
        cl::Buffer QRBuf(context, CL_MEM_READ_ONLY, sizeof(QR));

        std::cout << volDim[0] << " " << volDim[1] << " " << volDim[2] << std::endl;

        cl::Image3D volumeImg(context, CL_MEM_READ_ONLY, cl::ImageFormat(CL_INTENSITY, CL_FLOAT),
                              volDim[0], volDim[1], volDim[2]);
        cl::size_t<3> zeroVecOrigin;
        zeroVecOrigin[0] = zeroVecOrigin[1] = zeroVecOrigin[2] = 0;
        queue.enqueueWriteImage(volumeImg, CL_FALSE, zeroVecOrigin, volDim, 0, 0,
                                (void*)volume.data());

        cl_mem_flags writeFlag = CL_MEM_WRITE_ONLY;
        cl::Buffer projectionBuf(context, writeFlag,
                                 sizeof(float) * detectorColumns * detectorRows);

        // Set kernel parameters.
        kernel.setArg(0, detectorColumns);
        kernel.setArg(1, increment_mm);
        kernel.setArg(2, sourceBuf);
        kernel.setArg(3, volCornerBuf);
        kernel.setArg(4, voxelSizeBuf);
        kernel.setArg(5, QRBuf);
        kernel.setArg(6, projectionBuf);
        kernel.setArg(7, volumeImg);

        std::vector<float> singleProjection(detectorColumns * detectorRows);

        // loop over all projections in 'Pmats'
        for(uint proj = 0; proj < nbProjs; ++proj)
        {
            std::cout << "projection " << proj << std::endl;
            source = determineSource(Pmats.at(proj));
            queue.enqueueWriteBuffer(sourceBuf, CL_FALSE, 0, sizeof(source), &source);
            QR = decomposeM(Pmats.at(proj).M());
            queue.enqueueWriteBuffer(QRBuf, CL_FALSE, 0, sizeof(QR), &QR);

            // Launch kernel on the compute device.
            queue.enqueueNDRangeKernel(kernel, cl::NullRange,
                                       cl::NDRange(detectorColumns, detectorRows));

            // Get result back to host.
            queue.enqueueReadBuffer(projectionBuf, CL_TRUE, 0,
                                    singleProjection.size() * sizeof(float),
                                    singleProjection.data());

            // append data to full result vector
            std::copy(singleProjection.cbegin(), singleProjection.cend(),
                      ret.data() + proj * sizeOfProj);
        }
    } catch(const cl::Error& err)
    {
        std::cerr << "OpenCL error: " << err.what() << "(" << err.err() << ")" << std::endl;
    }
    return ret;
}

void RayCaster::initOpenCL()
{
    try
    {
        // Get list of OpenCL platforms.
        std::vector<cl::Platform> platform;
        cl::Platform::get(&platform);
        if(platform.empty())
        {
            std::cerr << "OpenCL platforms not found." << std::endl;
            return;
        }

        // Get first available GPU device which supports double precision.
        for(auto p = platform.begin(); device.empty() && p != platform.end(); p++)
        {
            std::vector<cl::Device> pldev;
            try
            {
                p->getDevices(CL_DEVICE_TYPE_GPU, &pldev);
                for(auto d = pldev.begin(); device.empty() && d != pldev.end(); d++)
                {
                    if(!d->getInfo<CL_DEVICE_AVAILABLE>())
                        continue;
                    std::string ext = d->getInfo<CL_DEVICE_EXTENSIONS>();
                    if(ext.find("cl_khr_fp64") == std::string::npos
                       && ext.find("cl_amd_fp64") == std::string::npos)
                        continue;
                    device.push_back(*d);
                    context = cl::Context(device);
                }
            } catch(...)
            {
                device.clear();
            }
        }

        if(device.empty())
        {
            std::cerr << "GPUs with double precision not found." << std::endl;
            return;
        }
        std::cout << device[0].getInfo<CL_DEVICE_NAME>() << std::endl;

        // Load .cl file and compile OpenCL program for found device.
        ClFileLoader clFile(CL_FILE_NAME);
        if(!clFile.isValid())
            throw std::runtime_error(CL_FILE_NAME + "\nis not readable");
        auto sourceString = clFile.loadSourceCode();
        program = cl::Program(
            context,
            cl::Program::Sources(1, std::make_pair(sourceString.c_str(), sourceString.size())));
        try
        {
            program.build(device);
        } catch(const cl::Error&)
        {
            std::cerr << "OpenCL compilation error" << std::endl
                      << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device[0]) << std::endl;
            return;
        }

    } catch(const cl::Error& err)
    {
        std::cerr << "OpenCL error: " << err.what() << "(" << err.err() << ")" << std::endl;
        return;
    }
}

cl_float3 RayCaster::volumeCorner() const
{
    return { { volOffset.s[0] - 0.5f * volDim[0] * voxelSize.s[0],
               volOffset.s[1] - 0.5f * volDim[1] * voxelSize.s[1],
               volOffset.s[2] - 0.5f * volDim[2] * voxelSize.s[2] } };
}

cl_double16 decomposeM(const Matrix3x3& M)
{
    auto QR = mat::QRdecomposition(M);
    auto& Q = QR.Q;
    auto& R = QR.R;
    if(std::signbit(R(0, 0) * R(1, 1) * R(2, 2)))
        R = -R;
    cl_double16 ret = { {Q(0,0),Q(0,1),Q(0,2),
                         Q(1,0),Q(1,1),Q(1,2),
                         Q(2,0),Q(2,1),Q(2,2),
                         R(0,0),R(0,1),R(0,2),
                                R(1,1),R(1,2),
                                       R(2,2)} };
    return ret;
}

cl_float3 determineSource(const ProjectionMatrix& P)
{
    auto ret = P.sourcePosition();
    return { { static_cast<float>(ret.get<0>()), static_cast<float>(ret.get<1>()),
               static_cast<float>(ret.get<2>()) } };
}

void RayCaster::setDetectorSize(uint nbRows, uint nbColumns)
{
    detectorRows = cl_uint(nbRows);
    detectorColumns = cl_uint(nbColumns);
}

void RayCaster::setIncrement(float incrementMM) { increment_mm = cl_float(incrementMM); }

void RayCaster::setVolumeOffset(const float (&offset)[3])
{
    volOffset = { { offset[0], offset[1], offset[2] } };
}

void RayCaster::setVolumeInfo(const uint (&nbVoxel)[3], const float (&vSize)[3])
{
    volDim[0] = nbVoxel[0];
    volDim[1] = nbVoxel[1];
    volDim[2] = nbVoxel[2];
    voxelSize = { { vSize[0], vSize[1], vSize[2] } };
}

} // namespace OCL
} // namespace CTL
