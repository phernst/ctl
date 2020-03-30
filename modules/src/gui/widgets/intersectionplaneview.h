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

class IntersectionPlaneView : public QWidget
{
    Q_OBJECT
public:
    explicit IntersectionPlaneView(QWidget* parent = nullptr,
                                   float visualScale = 50.0f);

    void setPlaneSize(const QSizeF& size);
    void setVolumeDim(const VoxelVolume<float>& volume);
    void setVolumeDim(const VoxelVolume<float>::Dimensions& dimensions,
                      const VoxelVolume<float>::Offset& offset,
                      const VoxelVolume<float>::VoxelSize& voxelSize);

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
