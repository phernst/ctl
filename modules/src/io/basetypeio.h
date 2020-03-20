#ifndef CTL_BASETYPEIO_H
#define CTL_BASETYPEIO_H

#include "abstractbasetypeio.h"
#include <memory>

namespace CTL {
namespace io {

/*!
 * \class BaseTypeIO
 *
 * \brief Interface to read and write basic CTL container types.
 *
 * This class provides the interface to read and write all basic container classes the are used
 * throughout the CTL. These basic container types are:
 * \li VoxelVolume<T>
 * \li Chunk2D<T>
 * \li SingleViewData
 * \li ProjectionData
 * \li SingleViewGeometry
 * \li FullGeometry
 *
 * Each type can be read from a file using the dedicated 'read' method (e.g. readProjections() for
 * ProjectionData). Additionally, meta information contained in the file can be obtained from the
 * metaInfo() method. Use the write() method to store the content of a container in a file.
 *
 * The BaseTypeIO is an interface that, internally, uses a FileIOImplementer to perform the actual
 * read/write processes in a specific file format. To be compatible with BaseTypeIO, a
 * FileIOImplementer needs to provide the following (public) methods:
 * \code
 * QVariantMap metaInfo(const QString& fileName) const;
 *
 * template <typename T>
 * std::vector<T> readAll(const QString& fileName) const;
 *
 * template <typename T>
 * std::vector<T> readChunk(const QString& fileName, uint chunkNb) const;
 *
 * template <typename T>
 * bool write(const std::vector<T>& data, const QVariantMap& metaInfo, const QString& fileName) const;
 * \endcode
 *
 * These functions must fulfill the following conditions:
 * \li metaInfo(): extract all available meta information from the file and store it in a
 * QVariantMap. Please check the predefined meta information keys in metainfokeys.h for some
 * standardized keys.
 * Minimum requirement for a valid metaInfo() method is to include the dimensions of the data as the
 * key-value pair (meta_info::dimensions, meta_info::Dimensions \a dim), whereby \a dim contains
 * exactly as many unsigned integer values as there are dimensions in the data (one value for each
 * dimension with the information, how many elements the data contains in this dimension). The
 * dimension values must be sorted according to the data order described in readAll().
 * \li readAll(): read the entire data from the file and store it in an std::vector with
 * row-major order (i.e. (*x* -> *y*) -> *z* for volumes; ((*columns* -> *rows*) -> *modules*) -> *views*
 * for projections or projection matrices).
 * \li readChunk(): read a particular two-dimensional chunk of data from the file into and store it
 * in an std::vector with row-major order (i.e. *x* -> *y* for volume slices; *columns* -> *rows* for
 * projections or projection matrices). The chunk number \a chunkNb refers to the *z*-slice (volumes)
 * or to the index in the one-dimensional mapping *modules* -> *views* (projections / projection
 * matrices).
 * \li write(): write all data from an std::vector (which contains row-major order sequential data)
 * to a file. This also includes writing all meta information passed in \a metaInfo that is
 * supported by the file type.
 */

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
    bool write(const SingleViewData& data, const QString& fileName,
               QVariantMap supplementaryMetaInfo = {}) const;
    bool write(const ProjectionData& data, const QString& fileName,
               QVariantMap supplementaryMetaInfo = {}) const;
    bool write(const SingleViewGeometry& data, const QString& fileName,
               QVariantMap supplementaryMetaInfo = {}) const;
    bool write(const FullGeometry& data, const QString& fileName,
               QVariantMap supplementaryMetaInfo = {}) const;

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
        public: bool write(const SingleViewData& data, const QString& fileName,
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
        public: bool write(const SingleViewGeometry& data, const QString& fileName,
                           QVariantMap supplementaryMetaInfo = {}) const override;
    };

    // ### MAKE FUNCTIONS FOR INSTANTIATION OF A CONCRETE IO OBJECT ###
    static auto makeMetaInfoReader() -> std::unique_ptr<MetaInfoReader>;
    template<typename T>
    static auto makeVolumeIO() -> std::unique_ptr<VolumeIO<T>>;
    static auto makeProjectionDataIO() -> std::unique_ptr<ProjectionDataIO>;
    static auto makeProjectionMatrixIO() -> std::unique_ptr<ProjectionMatrixIO>;

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
 * \def NO_SINGLE_MODULE_FALLBACK
 * By default, the BaseTypeIO assumes that you have only one detector module in all methods where
 * the number of modules is not available directly , i.e. neither as an argument passed to the read
 * functions (nbModules = 0) nor by the meta information in the file (e.g. the DEN format has not
 * such meta info inherently).
 * If this behavior is not desired, define NO_SINGLE_MODULE_FALLBACK. Then, the read functions
 * readProjections, readSingleView, readFullGeometry and readSingleViewGeometry will throw an
 * exception (std::runtime_error) if the number of modules is not available.
 */

///@}

#endif // CTL_BASETYPEIO_H
