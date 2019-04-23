#ifndef OPENCLCONFIG_H
#define OPENCLCONFIG_H

#define __CL_ENABLE_EXCEPTIONS
#define CL_USE_DEPRECATED_OPENCL_1_1_APIS
#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#if defined(__APPLE__) || defined(__MACOSX)
#include "OpenCL/cl.hpp"
#else
#include <CL/cl.hpp>
#endif

// OpenCL version < 1.2 compatibility
 #ifndef CL_VERSION_1_2
 #define CL_MEM_HOST_WRITE_ONLY 0
 #define CL_MEM_HOST_READ_ONLY 0
 #endif

#include <unordered_map>

namespace CTL {
namespace OCL {

const std::string OCL_CORE_PROGRAM = "ctl_core";

/*!
 * \class OpenCLConfig
 *
 * \brief Singleton for unified OpenCL configuration
 *
 * This OpenCLConfig class should take off the work to setup a OpenCL context and device list each
 * time a function uses OpenCL. Therefore it allows for a coherent OpenCL configuration within the
 * whole program. Moreover, it manages OpenCL programs and kernels. Thus, there is no need to
 * recompile an OpenCL programs twice.
 * It is designed as a Singleton (only one instance of that class possible). The instance can be
 * accessed by the static function instance().
 */
class OpenCLConfig
{
public:
    // access to the singleton instance
    static OpenCLConfig& instance(bool autoSetDevicesForFirstCall = true);

    // set devices to be used. invoke cl-program compilation
    bool setDevices(cl_device_type type, const cl::Platform* platform = nullptr);
    bool setDevices(const std::vector<std::string>& whiteListedStrings,
                    const std::vector<std::string>& blackListedStrings = { },
                    cl_device_type type = CL_DEVICE_TYPE_ALL);
    bool setDevices(std::vector<cl::Device> devices);
    // remove all devices from OpenCLConfig
    void removeDevices();
    // recompile using exisiting device list
    bool prebuild();

    // getter
    bool isValid() const { return _isValid; }
    const cl::Context& context() const { return _context; }
    const std::vector<cl::Device>& devices() const { return _devices; }
    bool isReady(const std::string& programName) const;
    bool allProgramsReady() const;
    bool programExists(const std::string& programName) const;
    bool kernelExists(const std::string& kernelName,
                      const std::string& programName = OCL_CORE_PROGRAM) const;

    // kernel getter with lazy compilation on demand
    cl::Kernel* kernel(const std::string& kernelName,
                       const std::string& programName = OCL_CORE_PROGRAM);

    // add kernel: if kernel already exists, nothing happens and false will be returned
    bool addKernel(const std::string& kernelName,
                   const std::string& source,
                   const std::string& programName = OCL_CORE_PROGRAM);
    bool addKernel(const std::string& kernelName,
                   const std::vector<std::string>& sources,
                   const std::string& programName = OCL_CORE_PROGRAM);

    // replace single kernel and its source(s), references or pointers are still valid
    bool replaceKernel(const std::string& kernelName,
                       const std::string& source,
                       const std::string& programName = OCL_CORE_PROGRAM);
    bool replaceKernel(const std::string& kernelName,
                       const std::vector<std::string>& source,
                       const std::string& programName = OCL_CORE_PROGRAM);

    // remove kernels -> caution: references or pointers to removed kernels are not valid anymore
    void removeKernel(const std::string& kernelName,
                      const std::string& programName = OCL_CORE_PROGRAM);
    void removeAllKernels();

private:
    // private constructor. can initialize with default devices (tries first GPU, then CPU)
    OpenCLConfig(bool initialize);
    // non-copyable
    OpenCLConfig(const OpenCLConfig&) = delete;
    OpenCLConfig& operator=(const OpenCLConfig&) = delete;

    // kernel sources and cl-kernel object
    struct KernelRessource
    {
        std::vector<std::string> sources;
        cl::Kernel clKernel;
    };

    // program struct that can store several kernels
    struct Program
    {
        bool build(const OpenCLConfig& parent);
        bool kernelExists(const std::string& kernelName) const;
        std::unordered_map<std::string, KernelRessource> kernels;
        bool isReady = false;

    private:
        std::vector<std::string> extractUniqueSources() const;
        cl::Program _clProgram;
    };

    // members
    cl::Context _context;
    std::vector<cl::Device> _devices;
    std::unordered_map<std::string, Program> _programs;
    bool _isValid;

    // help functions
    bool createContext();
    bool buildPrograms();
    bool prepareNewKernel(const std::string& kernelName, const std::string& programName);
};

} // namespace OCL
} // namespace CTL

#endif // OPENCLCONFIG_H
