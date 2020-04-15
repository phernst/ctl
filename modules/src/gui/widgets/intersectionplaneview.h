#ifndef CTL_PLANEVISUALIZER_H
#define CTL_PLANEVISUALIZER_H

#include "img/voxelvolume.h"
#include <QWidget>
#include <QQuaternion>

// forward delcarations
namespace CTL {
class AbstractDetector;
class AbstractGantry;
class AbstractSource;
class SimpleCTsystem;
} // namespace CTL
class QGridLayout;
namespace Qt3DCore {
class QEntity;
} // namespace Qt3DCore
namespace Qt3DExtras {
class Qt3DWindow;
class QOrbitCameraController;
} // namespace Qt3DExtras
namespace Qt3DRender {
class QCamera;
class QMaterial;
} // namespace Qt3DRender

namespace CTL {
namespace gui {

/*!
 * \class IntersectionPlaneView
 *
 * \brief The IntersectionPlaneView class visualizes planes intersecting a volume.
 *
 * This class can be used to visualize how a certain plane intersects a given volume. Specify the
 * plane parameters (in spherical coordinates) by setPlaneParameter(). The size of the visualized
 * plane is set by setPlaneSize().
 *
 * Size and position of the volume can be defined by setVolumeDim(). This is done either from its
 * individual specifications (i.e. number of voxels, voxel size, and spatial position offset) with
 * setVolumeDim(const VoxelVolume<float>::Dimensions&, const VoxelVolume<float>::Offset&,
 * const VoxelVolume<float>::VoxelSize&) or simply by copying the specs of a given volume data
 * object using setVolumeDim(const VoxelVolume<float>&).
 *
 *
 * The following IO operations are supported by this widget:
 *
 * - Zooming:
 *    - Scroll mouse wheel up/down to zoom in/out.
 * - Camera positioning / orientation:
 *    - Hold left mouse button + move up/down/left/right to move the camera position in the
 * corresponding direction
 *    - Hold right mouse button + move up/down/left/right to rotate the camera direction
 *
 * To clear all items from the scene, use clearScene(). Camera position can be reset to its original
 * setting by resetCamera(). The resetView() combines the effects of clearing the scene and
 * resetting the camera.
 *
 * Example:
 * \code
 * // create a volume with total size 100mm x 100mm x 200mm
 * // (note: total size is the product of number of voxels and voxel size)
 * VoxelVolume<float> volume(1, 1, 1, 100.0, 100.0, 200.0);
 *
 * // (static version) using the plot() command
 * gui::IntersectionPlaneView::plot(volume, 10.0_deg, 30.0_deg, 50.0);
 *
 * // (property-based version)
 * auto viewer = new gui::IntersectionPlaneView;
 * viewer->setVolumeDim(volume);
 * viewer->setPlaneParameter(10.0_deg, 30.0_deg, 50.0);
 * viewer->setPlaneSize( { 250.0, 250.0 } ); // creates a plane with 250mm x 250mm size
 * viewer->show();
 * \endcode
 *
 * ![Visualization from the example above (property-based version).](gui/IntersectionPlaneView.png)
 *
 * There are many different ways to specify the volume that is visualized in the scene, as can be
 * seen in the following example:
 * \code
 * VoxelVolume<float> volume(100, 200, 300);
 * volume.setVoxelSize(1.5, 0.5, 1.0);
 * volume.setVolumeOffset(0.0f, -50.0f, 0.0f);
 *
 * auto viewer = new gui::IntersectionPlaneView;
 * viewer->setPlaneParameter(10.0_deg, 30.0_deg, 50.0);
 * viewer->setPlaneSize( { 300.0, 300.0 } );
 *
 * // all versions lead to the same result
 * viewer->setVolumeDim(volume);
 * //viewer->setVolumeDim(volume.dimensions(), volume.voxelSize(), volume.offset());
 * //viewer->setVolumeDim( {100, 200, 300}, {1.5, 0.5, 1.0}, {0.0f, -50.0f, 0.0f} );
 * //viewer->setVolumeDim(150.0f, 100.0f, 300.0f, {0.0f, -50.0f, 0.0f});
 * viewer->show();
 * \endcode
 *
 */
class IntersectionPlaneView : public QWidget
{
    Q_OBJECT
public:
    explicit IntersectionPlaneView(QWidget* parent = nullptr,
                                   float visualScale = 50.0f);

    void setPlaneSize(double width, double height);
    void setPlaneSize(const QSizeF& size);
    void setVolumeDim(const VoxelVolume<float>& volume);
    void setVolumeDim(float sizeX, float sizeY, float sizeZ,
                      const VoxelVolume<float>::Offset& offset = { 0.0f, 0.0f, 0.0f});
    void setVolumeDim(const VoxelVolume<float>::Dimensions& dimensions,
                      const VoxelVolume<float>::VoxelSize& voxelSize,
                      const VoxelVolume<float>::Offset& offset = { 0.0f, 0.0f, 0.0f});

    static void plot(const VoxelVolume<float>& volume,
                     double azimuth, double polar, double distance, float visualScale = 50.0f);

public slots:
    void clearScene();
    void resetCamera();
    void resetView();
    void setPlaneParameter(double azimuth, double polar, double distance);

private:
    QGridLayout* _mainLayout;

    Qt3DExtras::Qt3DWindow* _view;
    Qt3DCore::QEntity* _rootEntity;
    Qt3DRender::QCamera* _camera;
    Qt3DExtras::QOrbitCameraController* _camController;
    Qt3DRender::QMaterial* _defaultMaterial;

    VoxelVolume<float>::Dimensions _volDim;
    VoxelVolume<float>::Offset _volOffset;
    VoxelVolume<float>::VoxelSize _volVoxSize;

    QSizeF _planeSize;
    QVector3D _planeTranslation;
    QQuaternion _planeRotation;

    float _visualScale = 50.0f;

    void addAxis(Qt::Axis axis, float lineLength = 10.0f);
    void addBoxObject(const QVector3D& dimensions,
                      const QVector3D& translation,
                      const QQuaternion& rotation,
                      Qt3DRender::QMaterial* material = nullptr);
    void addCoordinateSystem();
    void addPlane();
    void addVolume();
    void initializeView();
    void redraw();
};

} // namespace gui
} // namespace CTL

#endif // CTL_PLANEVISUALIZER_H
