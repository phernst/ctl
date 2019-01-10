#ifndef BASETYPEIO_H
#define BASETYPEIO_H

#define SINGLE_MODULE_FALLBACK

#include "abstractbasetypeio.h"
#include <memory>

namespace CTL {
namespace io {

// generalized version (preferred)
template <class FileIOImplementer>
class BaseTypeIO
{
public:
    // ### READING ###
    // meta info
    QVariantMap metaInfo(const QString& fileName) const;

    // volume data
    template <typename T>
    VoxelVolume<T> readVolume(const QString& fileName) const;
    template <typename T>
    Chunk2D<T> readSlice(const QString& fileName, uint sliceNb) const;

    // projection data
    ProjectionData readProjections(const QString& fileName, uint nbModules = 0) const;
    SingleViewData readSingleView(const QString& fileName, uint viewNb, uint nbModules = 0) const;

    // projection matrices
    FullGeometry readFullGeometry(const QString& fileName, uint nbModules = 0) const;
    SingleViewGeometry readSingleViewGeometry(const QString& fileName, uint viewNb, uint nbModules = 0) const;

    // ### WRITING ###
    template <typename T>
    bool write(const Chunk2D<T>& data, const QString& fileName,
               QVariantMap supplementaryMetaInfo = {}) const;
    template <typename T>
    bool write(const VoxelVolume<T>& data, const QString& fileName,
               QVariantMap supplementaryMetaInfo = {}) const;
    bool write(const ProjectionData& data, const QString& fileName,
               QVariantMap supplementaryMetaInfo = {}) const;
    bool write(const FullGeometry& data, const QString& fileName,
               QVariantMap supplementaryMetaInfo = {}) const;
    // TBD: write SingleViewData and SingleViewGeometry ?

    // ### IMPLEMENTATION OF ABSTRACT TYPES ###
    class MetaInfoReader : public AbstractMetaInfoReader
    {
        public: QVariantMap metaInfo(const QString& fileName) const override;
    };

    template<typename T>
    class VolumeIO : public AbstractVolumeIO<T>
    {
        public: QVariantMap metaInfo(const QString& fileName) const override;
        public: VoxelVolume<T> readVolume(const QString& fileName) const override;
        public: Chunk2D<T> readSlice(const QString& fileName, uint sliceNb) const override;
        public: bool write(const VoxelVolume<T>& data, const QString& fileName,
                           QVariantMap supplementaryMetaInfo = {}) const override;
        public: bool write(const Chunk2D<T>& data, const QString& fileName,
                           QVariantMap supplementaryMetaInfo = {}) const override;
    };

    class ProjectionDataIO : public AbstractProjectionDataIO
    {
        public: QVariantMap metaInfo(const QString& fileName) const override;
        public: ProjectionData readProjections(const QString& fileName,
                                               uint nbModules = 0) const override;
        public: SingleViewData readSingleView(const QString& fileName, uint viewNb,
                                              uint nbModules = 0) const override;
        public: bool write(const ProjectionData& data, const QString& fileName,
                           QVariantMap supplementaryMetaInfo = {}) const override;
    };

    class ProjectionMatrixIO : public AbstractProjectionMatrixIO
    {
        public: QVariantMap metaInfo(const QString& fileName) const override;
        public: FullGeometry readFullGeometry(const QString& fileName,
                                              uint nbModules = 0) const override;
        public: SingleViewGeometry readSingleViewGeometry(const QString& fileName, uint viewNb,
                                                          uint nbModules = 0) const override;
        public: bool write(const FullGeometry& data, const QString& fileName,
                           QVariantMap supplementaryMetaInfo = {}) const override;
    };

    // ### MAKE FUNCTIONS FOR INSTANTIATION OF A CONCRETE IO OBJECT ###
    static std::unique_ptr<MetaInfoReader> makeMetaInfoReader();
    template<typename T>
    static std::unique_ptr<VolumeIO<T>> makeVolumeIO();
    static std::unique_ptr<ProjectionDataIO> makeProjectionDataIO();
    static std::unique_ptr<ProjectionMatrixIO> makeProjectionMatrixIO();

private:
    FileIOImplementer _implementer;

    ProjectionData::Dimensions dimensionsFromMetaInfo(const QVariantMap& info, uint nbModules = 0) const;
    QVariantMap fusedMetaInfo(const QVariantMap& baseInfo, QVariantMap supplementary) const;
};

} // namespace io
} // namespace CTL

#include "io/basetypeio.tpp"

/*! \file */
///@{

/*!
 * \def SINGLE_MODULE_FALLBACK
 * If this macro has been defined, the BaseTypeIO assumes that you have only one detector module
 * if the number of modules is not given at all, i.e. neither by an argument passed to the read
 * functions (nbModules = 0) nor by the meta information in the file (e.g. the DEN format has not
 * such meta info inherently).
 * In case you remove this macro, the read functions
 * readProjections, readSingleView, readFullGeometry and readSingleViewGeometry will throw an
 * exception (std::runtime_error) if the number of modules is not available.
 */

///@}

#endif // BASETYPEIO_H
