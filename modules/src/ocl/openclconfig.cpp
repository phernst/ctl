#include "openclconfig.h"
#include <algorithm>

namespace CTL {
namespace OCL {
// static function declaration
// ---------------------------
static bool areSuitableDevices(const std::vector<cl::Device>& devices);
static std::vector<cl::Device> findDevices(cl_device_type type, const cl::Platform& platform);
static std::vector<cl::Platform> findPlatforms();

// OpenCLConfig member function definition
// ---------------------------------------
// Constructor
OpenCLConfig::OpenCLConfig(bool initialize)
    : _isValid(false)
{
    if(initialize)
        if(!setDevices(CL_DEVICE_TYPE_GPU))
            setDevices(CL_DEVICE_TYPE_CPU); // fallback to CPU
}

/*!
 * Accessor of the OpenCLConfig instance.
 * If it is called for the first time and \a autoSetDevicesForFirstCall is true, the instance will
 * be constructed using the default constructor, which sets a standard device configuration: first,
 * it tries to use GPUs and second (if not successful) it searchs for OpenCL compatible CPUs.
 * It is intended to call instance(false) only when using the setDevices() function to prevent
 * double initialization, e.g.
 * \code
 *    OpenCLConfig::instance(false).setDevices(CL_DEVICE_TYPE_CPU);
 * \endcode
 */
OpenCLConfig& OpenCLConfig::instance(bool autoSetDevicesForFirstCall)
{
    static OpenCLConfig theInstance(autoSetDevicesForFirstCall);
    return theInstance;
}

/*!
 * Same as setDevices(std::vector<cl::Device> devices) with an addition search functionality for
 * devices fitting to device \a type and a \a platform that provides this \a type.
 *
 * It sets the OpenCL devices corresponding to the device \a type. If multiple OpenCL platforms
 * are installed you can specify the desired platform by a pointer to that \a platform. The default
 * \a platform pointer is a `nullptr`, which means that a platform will be chosen automatically be
 * searching for the first platform that has at least one device with the specified \a type.
 * An argument for \a type can be one or combinations of the following values
 * \li `CL_DEVICE_TYPE_DEFAULT`
 * \li `CL_DEVICE_TYPE_CPU`
 * \li `CL_DEVICE_TYPE_GPU`
 * \li `CL_DEVICE_TYPE_ACCELERATOR`
 * \li `CL_DEVICE_TYPE_CUSTOM`
 * \li `CL_DEVICE_TYPE_ALL`
 *
 * \sa setDevices(const std::vector<std::string>&, const std::vector<std::string>&, cl_device_type),
 * setDevices(std::vector<cl::Device> devices), instance(bool)
 */
bool OpenCLConfig::setDevices(cl_device_type type, const cl::Platform* platform)
{
    // reset
    removeDevices();

    // search for proper platform
    if(platform == nullptr)
    {
        // get list of OpenCL platforms
        auto platforms = findPlatforms();
        // check each platform
        for(const auto& p : platforms)
            if(setDevices(findDevices(type, p)))
                return true;
    }
    // use specified platform
    else
    {
        return setDevices(findDevices(type, *platform));
    }

    return false;
}

/*!
 * Sets only devices whose names fit to a certain whitelist of strings, i.e. a device name must
 * contain at least one of the strings in the \a whiteListedStrings. Additionally, you can specify a
 * blacklist, i.e. a device name must not contain any of the strings in the \a blackListedStrings.
 * Eventually, you can specify whether the devices must have a certain device \a type.
 *
 * Notes:
 * The internal string comparison is case-sensitive.
 * A whitelist that contains an empty string ("") allows all device names. This fact can be used if
 * you want to specify only a blacklist. Then you can specify the whitelist as
 * `std::vector<std::string> whiteListedStrings = {""}`.
 *
 * \sa setDevices(cl_device_type, const cl::Platform*), setDevices(std::vector<cl::Device> devices),
 * instance(bool)
 */
bool OpenCLConfig::setDevices(const std::vector<std::string>& whiteListedStrings,
                              const std::vector<std::string>& blackListedStrings,
                              cl_device_type type)
{
    // get list of OpenCL platforms
    auto platforms = findPlatforms();
    std::vector<cl::Device> devList;

    // check each platform
    for(const auto& p : platforms)
    {
        devList.clear();
        auto tmpDevices = findDevices(type, p);

        for(const auto& dev : tmpDevices)
        {
            std::string devName = dev.getInfo<CL_DEVICE_NAME>();
            bool matched = false;

            // check white list
            for(const auto& whiteName : whiteListedStrings)
                matched |= devName.find(whiteName) != std::string::npos;
            if(!matched)
                continue;

            // check black list
            for(const auto& blackName : blackListedStrings)
                matched &= devName.find(blackName) == std::string::npos;

            if(matched) // matches white- and blacklist
                devList.push_back(dev);
        }

        if(setDevices(std::move(devList)))
            return true;
    }

    return false;
}

/*!
 * Sets a specific list of \a devices and returns true if successful, otherwise false. Moreover,
 * if the setting of devices succeeds, the OpenCLConfig instance is valid, which can be check using
 * isValid().
 *
 * If the found devices are suitable, an OpenCL context based on these devices is created
 * internally. Devices are only suitable if they have 64 Bit floating point support.
 *
 * If the function is used at the begin of the program to initially set a device list, it is more
 * efficient to call the `instance(false)`, otherwise (if `instance()` is called) the device list
 * will be automatically initialize by the `instance(true)` call and then reset by the call of this
 * function.
 *
 * \sa setDevices(cl_device_type type, const cl::Platform* platform),
 * setDevices(const std::vector<std::string>&, const std::vector<std::string>&, cl_device_type),
 * instance(bool)
 */
bool OpenCLConfig::setDevices(std::vector<cl::Device> devices)
{
    // reset
    removeDevices();

    // check devices
    if(!areSuitableDevices(devices))
        return false;
    // set devices
    _devices = std::move(devices);

    // create context
    if(!createContext())
        return false;

    return _isValid = true;
}

/*!
 * Remove all devices in OpenCLConfig instance. Makes OpenCLConfig invalid and all programs are
 * are marked as not ready.
 */
void OpenCLConfig::removeDevices()
{
    // set OpenCLConfig invalid
    _isValid = false;
    for(auto& program : _programs)
        program.second.isReady = false;
    _devices.clear();
}

bool OpenCLConfig::prebuild()
{
    if(!_isValid)
        return false;
    return buildPrograms();
}

bool OpenCLConfig::programExists(const std::string &programName) const
{
    return _programs.find(programName) != _programs.end();
}

bool OpenCLConfig::kernelExists(const std::string &kernelName, const std::string &programName) const
{
    return programExists(programName) &&
           _programs.at(programName).kernelExists(kernelName);
}

cl::Kernel* OpenCLConfig::kernel(const std::string& kernelName, const std::string& programName)
{
    if(!_isValid)
        return nullptr;
    // check if program and kernel do not exists
    if(!programExists(programName))
        return nullptr;
    if(!_programs[programName].kernelExists(kernelName))
        return nullptr;

    // lazy compilation
    if(!_programs[programName].isReady)
        if(!_programs[programName].build(*this))
            return nullptr;

    return &_programs[programName].kernels[kernelName].clKernel;
}

bool OpenCLConfig::isReady(const std::string& programName) const
{
    if(!_isValid)
        return false;
    if(!programExists(programName))
        return false;
    return _programs.at(programName).isReady;
}

bool OpenCLConfig::allProgramsReady() const
{
    if(!_isValid)
        return false;
    for(const auto& program : _programs)
        if(!program.second.isReady)
            return false;
    return true;
}

bool OpenCLConfig::addKernel(const std::string& kernelName,
                             const std::string& source,
                             const std::string& programName)
{
    // check if same kernel exists (then return false) and allocate a new program if necessary
    if(!prepareNewKernel(kernelName, programName))
        return false;

    _programs[programName].kernels[kernelName] = KernelRessource();
    _programs[programName].kernels[kernelName].sources.push_back(source);
    _programs[programName].isReady = false;

    return true;
}

bool OpenCLConfig::addKernel(const std::string& kernelName,
                             const std::vector<std::string>& sources,
                             const std::string& programName)
{
    // check if same kernel exists (then return false) and allocate a new program if necessary
    if(!prepareNewKernel(kernelName, programName))
        return false;

    _programs[programName].kernels[kernelName] = KernelRessource();
    _programs[programName].kernels[kernelName].sources = sources;
    _programs[programName].isReady = false;

    return true;
}

bool OpenCLConfig::replaceKernel(const std::string& kernelName,
                                 const std::string& source,
                                 const std::string& programName)
{
    // check if kernel not exists
    if(!programExists(programName))
        return false;
    if(!_programs[programName].kernelExists(kernelName))
        return false;

    // replace sources
    _programs[programName].kernels[kernelName].sources.clear();
    _programs[programName].kernels[kernelName].sources.push_back(source);

    // rebuild program if program was ready (tries to keep state)
    if(_programs[programName].isReady)
        if(!_programs[programName].build(*this))
            return false;

    return true;
}

bool OpenCLConfig::replaceKernel(const std::string& kernelName,
                                 const std::vector<std::string>& source,
                                 const std::string& programName)
{
    // check if kernel not exists
    if(!programExists(programName))
        return false;
    if(!_programs[programName].kernelExists(kernelName))
        return false;

    // replace sources
    _programs[programName].kernels[kernelName].sources = source;

    // rebuild program if program was ready (tries to keep state)
    if(_programs[programName].isReady)
        if(!_programs[programName].build(*this))
            return false;

    return true;
}

void OpenCLConfig::removeKernel(const std::string& kernelName, const std::string& programName)
{
    // check if kernel not exists
    if(!programExists(programName))
        return;
    if(!_programs[programName].kernelExists(kernelName))
        return;
    _programs[programName].kernels.erase(kernelName);
}

void OpenCLConfig::removeAllKernels() { _programs.clear(); }

bool OpenCLConfig::createContext()
{
    cl_int err;
    _context = cl::Context(_devices, nullptr, nullptr, nullptr, &err);
    return err == CL_SUCCESS;
}

bool OpenCLConfig::buildPrograms()
{
    for(auto& program : _programs)
        if(!program.second.build(*this))
            return false;
    return true;
}

bool OpenCLConfig::prepareNewKernel(const std::string& kernelName, const std::string& programName)
{
    // check if same kernel exists
    if(programExists(programName))
    {
        if(_programs[programName].kernelExists(kernelName))
            return false;
    }
    // no program with this `programName` -> create
    else
    {
        _programs[programName] = Program();
    }
    return true;
}

// Program struct implementation
// -----------------------------
// build or rebuild a program
bool OpenCLConfig::Program::build(const OpenCLConfig& parent)
{
    isReady = false;
    if(!parent.isValid())
        return false;

    // create cl-source object
    auto uSources = extractUniqueSources();
    const auto N = uSources.size();
    cl::Program::Sources clSources(N);
    for(size_t i = 0; i < N; ++i)
        clSources[i] = std::make_pair(uSources[i].c_str(), uSources[i].size());

    // create program
    cl_int err;
    _clProgram = cl::Program(parent.context(), clSources, &err);
    if(err != CL_SUCCESS)
        return false;

    // build program
    err = _clProgram.build(parent.devices());
    if(err != CL_SUCCESS)
        return false;

    // make kernels
    for(auto& kernel : kernels)
    {
        const auto& kernelName = kernel.first;
        kernels[kernelName].clKernel = cl::Kernel(_clProgram, kernelName.c_str(), &err);
        if(err != CL_SUCCESS)
            return false;
    }

    return isReady = true;
}

// filter out recurrent sources
std::vector<std::string> OpenCLConfig::Program::extractUniqueSources() const
{
    std::vector<std::string> ret;
    for(const auto& kernel : kernels)
        for(const auto& source : kernel.second.sources)
            ret.push_back(source);

    std::sort(ret.begin(), ret.end());
    auto last = std::unique(ret.begin(), ret.end());
    ret.erase(last, ret.end());

    return ret;
}

// check for existence of a specific kernel name
bool OpenCLConfig::Program::kernelExists(const std::string& kernelName) const
{
    return kernels.find(kernelName) != kernels.end();
}

// static function definition
// --------------------------
bool areSuitableDevices(const std::vector<cl::Device>& devices)
{
    if(devices.empty())
        return false;
    for(const auto& device : devices)
    {
        // check for 64 bit extension
        std::string ext = device.getInfo<CL_DEVICE_EXTENSIONS>();
        if(ext.find("cl_khr_fp64") == std::string::npos
           && ext.find("cl_amd_fp64") == std::string::npos)
            return false;
        if(!device.getInfo<CL_DEVICE_AVAILABLE>())
            return false;
    }
    return true;
}

std::vector<cl::Device> findDevices(cl_device_type type, const cl::Platform& platform)
{
    std::vector<cl::Device> ret, tmp;
    auto err = platform.getDevices(CL_DEVICE_TYPE_ALL, &tmp);
    if(err != CL_SUCCESS)
        return ret;

    for(const auto& d : tmp)
        if(d.getInfo<CL_DEVICE_TYPE>() & type)
            ret.push_back(d);

    return ret;
}

static std::vector<cl::Platform> findPlatforms()
{
    std::vector<cl::Platform> ret;
    auto err = cl::Platform::get(&ret);
    if(err != CL_SUCCESS)
        return { };
    return ret;
}

} // namespace OCL
} // namespace CTL
