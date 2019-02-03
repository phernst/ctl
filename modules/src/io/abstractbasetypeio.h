#ifndef ABSTRACTBASETYPEIO_H
#define ABSTRACTBASETYPEIO_H

#include "img/voxelvolume.h"
#include "img/projectiondata.h"
#include "acquisition/fullgeometry.h"
#include <QVariantMap>

namespace CTL {
namespace io {

class AbstractMetaInfoReader
{
    public:virtual QVariantMap metaInfo(const QString& fileName) const = 0;

public:
    virtual ~AbstractMetaInfoReader() = default;
};

template<typename T>
class AbstractVolumeIO : public AbstractMetaInfoReader
{
    public:virtual VoxelVolume<T> readVolume(const QString& fileName) const = 0;
    public:virtual Chunk2D<T> readSlice(const QString& fileName, uint sliceNb) const = 0;
    public:virtual bool write(const VoxelVolume<T>& data, const QString& fileName,
                              QVariantMap supplementaryMetaInfo = {}) const = 0;
    public:virtual bool write(const Chunk2D<T>& slice, const QString& fileName,
                              QVariantMap supplementaryMetaInfo = {}) const = 0;
};

class AbstractProjectionDataIO : public AbstractMetaInfoReader
{
    public:virtual ProjectionData readProjections(const QString& fileName,
                                                  uint nbModules = 0) const = 0;
    public:virtual SingleViewData readSingleView(const QString& fileName, uint viewNb,
                                                 uint nbModules = 0) const = 0;
    public:virtual bool write(const ProjectionData& data, const QString& fileName,
                              QVariantMap supplementaryMetaInfo = {}) const = 0;
    public:virtual bool write(const SingleViewData& data, const QString& fileName,
                              QVariantMap supplementaryMetaInfo = {}) const = 0;
};

class AbstractProjectionMatrixIO : public AbstractMetaInfoReader
{
    public:virtual FullGeometry readFullGeometry(const QString& fileName,
                                                 uint nbModules = 0) const = 0;
    public:virtual SingleViewGeometry readSingleViewGeometry(const QString& fileName, uint viewNb,
                                                             uint nbModules = 0) const = 0;
    public:virtual bool write(const FullGeometry& data, const QString& fileName,
                              QVariantMap supplementaryMetaInfo = {}) const = 0;
    public:virtual bool write(const SingleViewGeometry& data, const QString& fileName,
                              QVariantMap supplementaryMetaInfo = {}) const = 0;
};

} // namespace io
} // namespace CTL

#endif // ABSTRACTBASETYPEIO_H
