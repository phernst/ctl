#include "basetypeio.h"
#include "metainfokeys.h"
#include <QDebug>

namespace CTL {
namespace io {

// ####### implementation for generalized version ########

template <class FileIOImplementer>
template <typename T>
VoxelVolume<T> BaseTypeIO<FileIOImplementer>::readVolume(const QString& fileName) const
{
    QVariantMap metaInfo = _implementer.metaInfo(fileName);

    typename VoxelVolume<T>::Dimensions dim{ metaInfo.value(meta_info::dimX, QVariant(0u)).toUInt(),
                                             metaInfo.value(meta_info::dimY, QVariant(0u)).toUInt(),
                                             metaInfo.value(meta_info::dimZ, QVariant(0u)).toUInt() };

    typename VoxelVolume<T>::VoxelSize size{ metaInfo.value(meta_info::voxSizeX, QVariant(.0f)).toFloat(),
                                             metaInfo.value(meta_info::voxSizeY, QVariant(.0f)).toFloat(),
                                             metaInfo.value(meta_info::voxSizeZ, QVariant(.0f)).toFloat() };

    typename VoxelVolume<T>::Offset offset{ metaInfo.value(meta_info::volOffX, QVariant(.0f)).toFloat(),
                                            metaInfo.value(meta_info::volOffY, QVariant(.0f)).toFloat(),
                                            metaInfo.value(meta_info::volOffZ, QVariant(.0f)).toFloat() };

    VoxelVolume<T> ret(dim, size);

    ret.setData(_implementer.template readAll<T>(fileName));
    ret.setVolumeOffset(offset);

    return ret;
}

template <class FileIOImplementer>
template <typename T>
Chunk2D<T> BaseTypeIO<FileIOImplementer>::readSlice(const QString& fileName, uint sliceNb) const
{
    QVariantMap metaInfo = _implementer.metaInfo(fileName);
    uint dim[3] { metaInfo.value(meta_info::dimX, QVariant(0u)).toUInt(),
                  metaInfo.value(meta_info::dimY, QVariant(0u)).toUInt(),
                  metaInfo.value(meta_info::dimZ, QVariant(0u)).toUInt() };

    Chunk2D<T> ret(dim[0], dim[1]);

    ret.setData(_implementer.template readChunk<T>(fileName, sliceNb));
    return ret;
}

template <class FileIOImplementer>
QVariantMap BaseTypeIO<FileIOImplementer>::metaInfo(const QString &fileName) const
{
    return _implementer.metaInfo(fileName);
}

template <class FileIOImplementer>
ProjectionData BaseTypeIO<FileIOImplementer>::readProjections(const QString& fileName,
                                                              uint nbModules) const
{
    QVariantMap metaInfo = _implementer.metaInfo(fileName);
    auto dim = dimensionsFromMetaInfo(metaInfo, nbModules);

    ProjectionData ret(dim.nbChannels, dim.nbRows, dim.nbModules);
    ret.setDataFromVector(_implementer.template readAll<float>(fileName));

    return ret;
}

template <class FileIOImplementer>
SingleViewData BaseTypeIO<FileIOImplementer>::readSingleView(const QString& fileName,
                                                             uint viewNb,
                                                             uint nbModules) const
{
    QVariantMap metaInfo = _implementer.metaInfo(fileName);
    const auto dim = dimensionsFromMetaInfo(metaInfo, nbModules);

    SingleViewData ret(dim.nbChannels, dim.nbRows);
    ret.allocateMemory(dim.nbModules);

    uint indexLookup = viewNb * dim.nbModules;
    for(uint mod = 0; mod < dim.nbModules; ++mod)
        ret.module(mod).setData(_implementer.template readChunk<float>(fileName, indexLookup++));

    return ret;
}

template <class FileIOImplementer>
FullGeometry BaseTypeIO<FileIOImplementer>::readFullGeometry(const QString& fileName,
                                                             uint nbModules) const
{
    QVariantMap metaInfo = _implementer.metaInfo(fileName);
    const auto dim = dimensionsFromMetaInfo(metaInfo, nbModules);

    if((dim.nbChannels != 4) || (dim.nbRows != 3))
        throw std::domain_error(
            "Loaded chunks do not have correct dimensions of projection matrices.");

    const auto rawData = _implementer.template readAll<double>(fileName);

    if(rawData.empty())
        throw std::runtime_error("No data has been read.");

    if(rawData.size() < 12u * dim.nbModules * dim.nbViews)
        throw std::runtime_error("Not enough data has been read.");

    FullGeometry ret;

    int matNb = 0;
    for(uint view = 0; view < dim.nbViews; ++view)
    {
        SingleViewGeometry sViewPmats;
        for(uint module = 0; module < dim.nbModules; ++module)
            sViewPmats.append(mat::ProjectionMatrix::fromContainer(rawData, matNb++));

        ret.append(sViewPmats);
    }

    return ret;
}

template <class FileIOImplementer>
SingleViewGeometry BaseTypeIO<FileIOImplementer>::readSingleViewGeometry(const QString& fileName,
                                                                         uint viewNb,
                                                                         uint nbModules) const
{
    QVariantMap metaInfo = _implementer.metaInfo(fileName);
    const auto dim = dimensionsFromMetaInfo(metaInfo, nbModules);
    SingleViewGeometry ret(dim.nbModules);

    uint indexLookup = viewNb * dim.nbModules;
    for(uint mod = 0; mod < dim.nbModules; ++mod)
        ret[mod] = mat::ProjectionMatrix::fromContainer(
                    _implementer.template readChunk<double>(fileName, indexLookup++), 0);

    return ret;
}


template <class FileIOImplementer>
template <typename T>
bool BaseTypeIO<FileIOImplementer>::write(const Chunk2D<T> &data,
                                          const QString &fileName,
                                          QVariantMap supplementaryMetaInfo) const
{
    QVariantMap metaInfo;
    metaInfo.insert(meta_info::dimX, QVariant(data.dimensions().width));
    metaInfo.insert(meta_info::dimY, QVariant(data.dimensions().height));
    metaInfo.insert(meta_info::dimZ, QVariant(1u));

    metaInfo = fusedMetaInfo(metaInfo, std::move(supplementaryMetaInfo));

    return _implementer.write(data.constData(), metaInfo, fileName);
}

template <class FileIOImplementer>
template <typename T>
bool BaseTypeIO<FileIOImplementer>::write(const VoxelVolume<T>& data,
                                          const QString& fileName,
                                          QVariantMap supplementaryMetaInfo) const
{
    QVariantMap metaInfo;
    metaInfo.insert(meta_info::dimX, QVariant(data.nbVoxels().x));
    metaInfo.insert(meta_info::dimY, QVariant(data.nbVoxels().y));
    metaInfo.insert(meta_info::dimZ, QVariant(data.nbVoxels().z));
    metaInfo.insert(meta_info::voxSizeX, QVariant(data.voxelSize().x));
    metaInfo.insert(meta_info::voxSizeY, QVariant(data.voxelSize().y));
    metaInfo.insert(meta_info::voxSizeZ, QVariant(data.voxelSize().z));
    metaInfo.insert(meta_info::volOffX, QVariant(data.offset().x));
    metaInfo.insert(meta_info::volOffY, QVariant(data.offset().y));
    metaInfo.insert(meta_info::volOffZ, QVariant(data.offset().z));
    metaInfo.insert(meta_info::typeHint, meta_info::type_hint::volume);

    metaInfo = fusedMetaInfo(metaInfo, std::move(supplementaryMetaInfo));

    return _implementer.write(data.constData(), metaInfo, fileName);
}

template <class FileIOImplementer>
bool BaseTypeIO<FileIOImplementer>::write(const ProjectionData& data,
                                          const QString& fileName,
                                          QVariantMap supplementaryMetaInfo) const
{
    QVariantMap metaInfo;
    metaInfo.insert(meta_info::dimChans, QVariant(data.dimensions().nbChannels));
    metaInfo.insert(meta_info::dimRows, QVariant(data.dimensions().nbRows));
    metaInfo.insert(meta_info::dimMods, QVariant(data.dimensions().nbModules));
    metaInfo.insert(meta_info::dimViews, QVariant(data.nbViews()));
    metaInfo.insert(meta_info::typeHint, meta_info::type_hint::projection);

    metaInfo = fusedMetaInfo(metaInfo, std::move(supplementaryMetaInfo));

    return _implementer.write(data.toVector(), metaInfo, fileName);
}

template <class FileIOImplementer>
bool BaseTypeIO<FileIOImplementer>::write(const FullGeometry& data,
                                          const QString& fileName,
                                          QVariantMap supplementaryMetaInfo) const
{
    const auto nbViews = uint(data.size());
    const auto nbModules = uint(data.at(0).size());

    QVariantMap metaInfo;
    metaInfo.insert(meta_info::dimChans, QVariant(4u));
    metaInfo.insert(meta_info::dimRows, QVariant(3u));
    metaInfo.insert(meta_info::dimMods, QVariant(nbModules));
    metaInfo.insert(meta_info::dimViews, QVariant(nbViews));
    metaInfo.insert(meta_info::typeHint, meta_info::type_hint::projMatrix);

    metaInfo = fusedMetaInfo(metaInfo, std::move(supplementaryMetaInfo));

    std::vector<double> dataVec;
    dataVec.reserve(12u * nbViews * nbModules);

    for(uint view = 0; view < nbViews; ++view)
        for(uint module = 0; module < nbModules; ++module)
            dataVec.insert(dataVec.end(), data[view][module].constBegin(),
                           data[view][module].constEnd());

    return _implementer.write(dataVec, metaInfo, fileName);
}

template<class FileIOImplementer>
ProjectionData::Dimensions BaseTypeIO<FileIOImplementer>::dimensionsFromMetaInfo(const QVariantMap& info, uint nbModules) const
{
    if(nbModules == 0) // try to extract nbModules from metaInfo
    {
        if(info.contains(meta_info::dimMods))
        {
            nbModules = info.value(meta_info::dimMods).toUInt();
        }
        else
        {
#ifdef SINGLE_MODULE_FALLBACK
            nbModules = 1;
            qDebug() << "missing file meta information about the number of modules: "
                        "assuming nbModules = 1.";
#else
            throw std::runtime_error("Aborted loading: missing file meta information!");
#endif
        }



        if(!nbModules)
            throw std::domain_error("Aborted loading: number of modules is zero!");
    }

    uint nbViews = info.value(meta_info::dimViews, QVariant(0u)).toUInt();
    if(nbViews == 0)
    {
        // look for entry 'meta_info::dimZ'
        if(!info.contains(meta_info::dimZ))
            throw std::runtime_error("Aborted loading: missing file meta information!");

        uint totalBlocks = info.value(meta_info::dimZ).toUInt();

        // check if nb. projs is multiple of nbModules
        if(totalBlocks % nbModules)
            throw std::runtime_error("Aborted loading: Number of projections in file is not a"
                                     "multiple of the specified number of modules!");

        nbViews = totalBlocks / nbModules;

        if(!nbViews)
            throw std::domain_error("Aborted loading: number of views is zero!");
    }

    ProjectionData::Dimensions ret;

    ret.nbChannels = info.value(meta_info::dimChans, QVariant(0u)).toUInt();
    ret.nbRows     = info.value(meta_info::dimRows, QVariant(0u)).toUInt();
    ret.nbModules  = static_cast<uint>(nbModules);
    ret.nbViews    = static_cast<uint>(nbViews);

    return ret;
}

template<class FileIOImplementer>
QVariantMap BaseTypeIO<FileIOImplementer>::fusedMetaInfo(const QVariantMap &baseInfo,
                                                         QVariantMap supplementary) const
{
    if(supplementary.isEmpty())
        return baseInfo;

    auto i = baseInfo.begin();
    while(i != baseInfo.constEnd())
    {
        supplementary.insert(i.key(), i.value());
        ++i;
    }

    return supplementary;
}


// ### IMPLEMENTATION OF ABSTRACT TYPES ###

// MetaInfoReader
template<class FileIOImplementer>
QVariantMap BaseTypeIO<FileIOImplementer>::MetaInfoReader::metaInfo(const QString &fileName) const
{
    BaseTypeIO<FileIOImplementer> io;
    return io.metaInfo(fileName);
}

// VolumeIO
template<typename FileIOImplementer>
template<typename T>
QVariantMap BaseTypeIO<FileIOImplementer>::VolumeIO<T>::metaInfo(const QString &fileName) const
{
    BaseTypeIO<FileIOImplementer> io;
    QVariantMap metaInfo = io.metaInfo(fileName);
    // remove meta info that makes no sense in this context but has been possibly set due to a lack
    // of information provided by the file format
    metaInfo.remove(meta_info::dimChans);
    metaInfo.remove(meta_info::dimMods);
    metaInfo.remove(meta_info::dimRows);

    if(!metaInfo.contains(meta_info::typeHint))
        metaInfo.insert(meta_info::typeHint, meta_info::type_hint::volume);

    return metaInfo;
}

template<typename FileIOImplementer>
template<typename T>
VoxelVolume<T> BaseTypeIO<FileIOImplementer>::VolumeIO<T>::readVolume(const QString &fileName) const
{
    BaseTypeIO<FileIOImplementer> io;
    return io.readVolume<T>(fileName);
}

template<typename FileIOImplementer>
template<typename T>
Chunk2D<T> BaseTypeIO<FileIOImplementer>::VolumeIO<T>::readSlice(const QString &fileName,
                                                                 uint sliceNb) const
{
    BaseTypeIO<FileIOImplementer> io;
    return io.readSlice<T>(fileName, sliceNb);
}

template<typename FileIOImplementer>
template<typename T>
bool BaseTypeIO<FileIOImplementer>::VolumeIO<T>::write(const VoxelVolume<T> &data,
                                                       const QString &fileName,
                                                       QVariantMap supplementaryMetaInfo) const
{
    BaseTypeIO<FileIOImplementer> io;
    return io.write(data, fileName, std::move(supplementaryMetaInfo));
}

template<typename FileIOImplementer>
template<typename T>
bool BaseTypeIO<FileIOImplementer>::VolumeIO<T>::write(const Chunk2D<T> &data,
                                                       const QString &fileName,
                                                       QVariantMap supplementaryMetaInfo) const
{
    BaseTypeIO<FileIOImplementer> io;
    return io.write(data, fileName, std::move(supplementaryMetaInfo));
}

// ProjectionDataIO
template<class FileIOImplementer>
QVariantMap BaseTypeIO<FileIOImplementer>::ProjectionDataIO::metaInfo(const QString &fileName) const
{
    BaseTypeIO<FileIOImplementer> io;
    QVariantMap metaInfo = io.metaInfo(fileName);
    // remove meta info that makes no sense in this context but has been possibly set due to a lack
    // of information provided by the file format
    metaInfo.remove(meta_info::voxSizeX);
    metaInfo.remove(meta_info::voxSizeY);
    metaInfo.remove(meta_info::voxSizeZ);
    metaInfo.remove(meta_info::volOffX);
    metaInfo.remove(meta_info::volOffY);
    metaInfo.remove(meta_info::volOffZ);

    if(!metaInfo.contains(meta_info::typeHint))
        metaInfo.insert(meta_info::typeHint, meta_info::type_hint::projection);

    return metaInfo;
}

template<class FileIOImplementer>
ProjectionData
BaseTypeIO<FileIOImplementer>::ProjectionDataIO::readProjections(const QString &fileName,
                                                                 uint nbModules) const
{
    BaseTypeIO<FileIOImplementer> io;
    return io.readProjections(fileName, nbModules);
}

template<class FileIOImplementer>
SingleViewData
BaseTypeIO<FileIOImplementer>::ProjectionDataIO::readSingleView(const QString &fileName,
                                                                uint viewNb, uint nbModules) const
{
    BaseTypeIO<FileIOImplementer> io;
    return io.readSingleView(fileName, viewNb, nbModules);
}

template<class FileIOImplementer>
bool
BaseTypeIO<FileIOImplementer>::ProjectionDataIO::write(const ProjectionData &data,
                                                       const QString &fileName,
                                                       QVariantMap supplementaryMetaInfo) const
{
    BaseTypeIO<FileIOImplementer> io;
    return io.write(data, fileName, std::move(supplementaryMetaInfo));
}

// ProjectionMatrixIO
template<class FileIOImplementer>
QVariantMap
BaseTypeIO<FileIOImplementer>::ProjectionMatrixIO::metaInfo(const QString &fileName) const
{
    BaseTypeIO<FileIOImplementer> io;
    QVariantMap metaInfo = io.metaInfo(fileName);
    // remove meta info that makes no sense in this context but has been possibly set due to a lack
    // of information provided by the file format
    metaInfo.remove(meta_info::dimChans);
    metaInfo.remove(meta_info::dimMods);
    metaInfo.remove(meta_info::dimRows);
    metaInfo.remove(meta_info::voxSizeX);
    metaInfo.remove(meta_info::voxSizeY);
    metaInfo.remove(meta_info::voxSizeZ);
    metaInfo.remove(meta_info::volOffX);
    metaInfo.remove(meta_info::volOffY);
    metaInfo.remove(meta_info::volOffZ);

    if(!metaInfo.contains(meta_info::typeHint))
        metaInfo.insert(meta_info::typeHint, meta_info::type_hint::projMatrix);

    return metaInfo;
}

template<class FileIOImplementer>
FullGeometry
BaseTypeIO<FileIOImplementer>::ProjectionMatrixIO::readFullGeometry(const QString &fileName,
                                                                    uint nbModules) const
{
    BaseTypeIO<FileIOImplementer> io;
    return io.readFullGeometry(fileName, nbModules);
}

template<class FileIOImplementer>
SingleViewGeometry
BaseTypeIO<FileIOImplementer>::ProjectionMatrixIO::readSingleViewGeometry(const QString &fileName,
                                                                          uint viewNb,
                                                                          uint nbModules) const
{
    BaseTypeIO<FileIOImplementer> io;
    return io.readSingleViewGeometry(fileName, viewNb, nbModules);
}

template<class FileIOImplementer>
bool
BaseTypeIO<FileIOImplementer>::ProjectionMatrixIO::write(const FullGeometry &data,
                                                         const QString &fileName,
                                                         QVariantMap supplementaryMetaInfo) const
{
    BaseTypeIO<FileIOImplementer> io;
    return io.write(data, fileName, std::move(supplementaryMetaInfo));
}

// ### MAKE FUNCTION FOR INSTANTIATION OF A CONCRETE IO OBJECT ###

template<class FileIOImplementer>
auto BaseTypeIO<FileIOImplementer>::makeMetaInfoReader() -> std::unique_ptr<MetaInfoReader>
{
    return std::unique_ptr<MetaInfoReader>(new MetaInfoReader);
}

template<class FileIOImplementer>
template<typename T>
auto BaseTypeIO<FileIOImplementer>::makeVolumeIO() -> std::unique_ptr<VolumeIO<T>>
{
    return std::unique_ptr<VolumeIO<T>>(new VolumeIO<T>);
}

template<class FileIOImplementer>
auto BaseTypeIO<FileIOImplementer>::makeProjectionDataIO() -> std::unique_ptr<ProjectionDataIO>
{
   return std::unique_ptr<ProjectionDataIO>(new ProjectionDataIO);
}

template<class FileIOImplementer>
auto BaseTypeIO<FileIOImplementer>::makeProjectionMatrixIO() -> std::unique_ptr<ProjectionMatrixIO>
{
  return std::unique_ptr<ProjectionMatrixIO>(new ProjectionMatrixIO);
}

} // namespace io
} // namespace CTL
