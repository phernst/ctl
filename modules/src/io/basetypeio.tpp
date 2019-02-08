#include "basetypeio.h"
#include "metainfokeys.h"
#include <QDebug>

namespace CTL {
namespace io {

// ####### implementation for generalized version ########

/*!
 * Reads a VoxelVolume<T> from the file \a fileName.
 *
 * If metaInfo(\a fileName) contains information about the size of the voxels (keys:
 * meta_info::voxSizeX, meta_info::voxSizeY, meta_info::voxSizeZ) and/or the volume offset (keys:
 * meta_info::volOffX, meta_info::volOffY, meta_info::volOffZ), these values will be set
 * accordingly.
 */
template <class FileIOImplementer>
template <typename T>
VoxelVolume<T> BaseTypeIO<FileIOImplementer>::readVolume(const QString& fileName) const
{
    QVariantMap metaInfo = _implementer.metaInfo(fileName);

    auto dimList = metaInfo.value(meta_info::dimensions).toList();

    typename VoxelVolume<T>::Dimensions dim{ dimList.value(0, 0u).toUInt(),
                                             dimList.value(1, 0u).toUInt(),
                                             dimList.value(2, 0u).toUInt() };

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

/*!
 * Reads the slice \a sliceNb of data from the file \a fileName.
 *
 * The slice index \a sliceNb refers to the *z*-slice (volumes) or to the index in the
 * one-dimensional mapping *modules* -> *views* (projections / projection matrices).
 */
template <class FileIOImplementer>
template <typename T>
Chunk2D<T> BaseTypeIO<FileIOImplementer>::readSlice(const QString& fileName, uint sliceNb) const
{
    QVariantMap metaInfo = _implementer.metaInfo(fileName);
    auto dimList = metaInfo.value(meta_info::dimensions).toList();

    Chunk2D<T> ret(dimList.value(0, 0u).toUInt(), dimList.value(1, 0u).toUInt());

    ret.setData(_implementer.template readChunk<T>(fileName, sliceNb));
    return ret;
}

template <class FileIOImplementer>
QVariantMap BaseTypeIO<FileIOImplementer>::metaInfo(const QString &fileName) const
{
    return _implementer.metaInfo(fileName);
}

/*!
 * Reads all projection data from the file \a fileName.
 *
 * Specify the number of detector modules with \a nbModules in case this information is not
 * contained in the meta information of the file. For \a nbModules = 0 (default case), the
 * method tries to extract the information from the meta information.
 *
 * If no suitable meta information can be found, it is assumed that the number of modules is one.
 * This behavior can be changed by defining NO_SINGLE_MODULE_FALLBACK before including basetypio.h.
 * In that case, an exception is thrown when the number of modules cannot be extracted from meta
 * information.
 */
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
    QVariantList dimensions{ data.dimensions().width,
                             data.dimensions().height };
    metaInfo.insert(meta_info::dimensions, dimensions);

    metaInfo.insert(meta_info::dim1Type, meta_info::nbVoxelsX);
    metaInfo.insert(meta_info::dim2Type, meta_info::nbVoxelsY);
    metaInfo.insert(meta_info::typeHint, meta_info::type_hint::slice);

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
    QVariantList dimensions{ data.nbVoxels().x,
                             data.nbVoxels().y,
                             data.nbVoxels().z };
    metaInfo.insert(meta_info::dimensions, dimensions);
    metaInfo.insert(meta_info::dim1Type, meta_info::nbVoxelsX);
    metaInfo.insert(meta_info::dim2Type, meta_info::nbVoxelsY);
    metaInfo.insert(meta_info::dim3Type, meta_info::nbVoxelsZ);
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
bool BaseTypeIO<FileIOImplementer>::write(const SingleViewData& data,
                                          const QString& fileName,
                                          QVariantMap supplementaryMetaInfo) const
{
    QVariantMap metaInfo;
    QVariantList dimensions{ data.dimensions().nbChannels,
                             data.dimensions().nbRows,
                             data.dimensions().nbModules };
    metaInfo.insert(meta_info::dimensions, dimensions);
    metaInfo.insert(meta_info::dim1Type, meta_info::nbChans);
    metaInfo.insert(meta_info::dim2Type, meta_info::nbRows);
    metaInfo.insert(meta_info::dim3Type, meta_info::nbMods);
    metaInfo.insert(meta_info::typeHint, meta_info::type_hint::projection);

    metaInfo = fusedMetaInfo(metaInfo, std::move(supplementaryMetaInfo));

    return _implementer.write(data.toVector(), metaInfo, fileName);
}

template <class FileIOImplementer>
bool BaseTypeIO<FileIOImplementer>::write(const ProjectionData& data,
                                          const QString& fileName,
                                          QVariantMap supplementaryMetaInfo) const
{
    QVariantMap metaInfo;
    QVariantList dimensions{ data.dimensions().nbChannels,
                             data.dimensions().nbRows,
                             data.dimensions().nbModules,
                             data.dimensions().nbViews };
    metaInfo.insert(meta_info::dimensions, dimensions);
    metaInfo.insert(meta_info::dim1Type, meta_info::nbChans);
    metaInfo.insert(meta_info::dim2Type, meta_info::nbRows);
    metaInfo.insert(meta_info::dim3Type, meta_info::nbMods);
    metaInfo.insert(meta_info::dim4Type, meta_info::nbViews);
    metaInfo.insert(meta_info::typeHint, meta_info::type_hint::projection);

    metaInfo = fusedMetaInfo(metaInfo, std::move(supplementaryMetaInfo));

    return _implementer.write(data.toVector(), metaInfo, fileName);
}

template <class FileIOImplementer>
bool BaseTypeIO<FileIOImplementer>::write(const SingleViewGeometry& data,
                                          const QString& fileName,
                                          QVariantMap supplementaryMetaInfo) const
{
    const auto nbModules = uint(data.size());

    QVariantMap metaInfo;
    QVariantList dimensions{ 4u,
                             3u,
                             nbModules };
    metaInfo.insert(meta_info::dimensions, dimensions);
    metaInfo.insert(meta_info::dim1Type, meta_info::nbCols);
    metaInfo.insert(meta_info::dim2Type, meta_info::nbRows);
    metaInfo.insert(meta_info::dim3Type, meta_info::nbMods);
    metaInfo.insert(meta_info::typeHint, meta_info::type_hint::projMatrix);

    metaInfo = fusedMetaInfo(metaInfo, std::move(supplementaryMetaInfo));

    std::vector<double> dataVec;
    dataVec.reserve(12u * nbModules);

    for(uint module = 0; module < nbModules; ++module)
        dataVec.insert(dataVec.end(), data[module].constBegin(),
                       data[module].constEnd());

    return _implementer.write(dataVec, metaInfo, fileName);
}

template <class FileIOImplementer>
bool BaseTypeIO<FileIOImplementer>::write(const FullGeometry& data,
                                          const QString& fileName,
                                          QVariantMap supplementaryMetaInfo) const
{
    const auto nbViews = uint(data.size());
    const auto nbModules = uint(data.at(0).size());

    QVariantMap metaInfo;
    QVariantList dimensions{ 4u,
                             3u,
                             nbModules,
                             nbViews };
    metaInfo.insert(meta_info::dimensions, dimensions);
    metaInfo.insert(meta_info::dim1Type, meta_info::nbCols);
    metaInfo.insert(meta_info::dim2Type, meta_info::nbRows);
    metaInfo.insert(meta_info::dim3Type, meta_info::nbMods);
    metaInfo.insert(meta_info::dim4Type, meta_info::nbViews);
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
    auto dimensionList = info.value(meta_info::dimensions).toList();

    if(nbModules == 0) // try to extract nbModules from metaInfo
    {
        if(info.value(meta_info::dim3Type).toString() == meta_info::nbMods)
        {
            nbModules = dimensionList.value(2, 0u).toUInt();
        }
        else
        {
#ifndef NO_SINGLE_MODULE_FALLBACK
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

    uint nbViews = dimensionList.value(3, 0u).toUInt();
    if(nbViews == 0)
    {
        // look for third dimension entry
        if(dimensionList.size() < 3)
            throw std::runtime_error("Aborted loading: missing file meta information!");

        uint totalBlocks = dimensionList.at(2).toUInt();

        // check if nb. projs is multiple of nbModules
        if(totalBlocks % nbModules)
            throw std::runtime_error("Aborted loading: Number of projections in file is not a"
                                     "multiple of the specified number of modules!");

        nbViews = totalBlocks / nbModules;

        if(!nbViews)
            throw std::domain_error("Aborted loading: number of views is zero!");
    }

    ProjectionData::Dimensions ret;

    ret.nbChannels = dimensionList.value(0, 0u).toUInt();
    ret.nbRows     = dimensionList.value(1, 0u).toUInt();
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

template<class FileIOImplementer>
bool
BaseTypeIO<FileIOImplementer>::ProjectionDataIO::write(const SingleViewData &data,
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

template<class FileIOImplementer>
bool
BaseTypeIO<FileIOImplementer>::ProjectionMatrixIO::write(const SingleViewGeometry &data,
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
