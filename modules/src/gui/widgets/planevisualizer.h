#ifndef CTL_PLANEVISUALIZER_H
#define CTL_PLANEVISUALIZER_H

#include "img/voxelvolume.h"
#include <QWidget>
#include <QQuaternion>

constexpr float PLANEVIS_VIS_SCALE = 50.0f;

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

class PlaneVisualizer : public QWidget
{
    Q_OBJECT
public:
    explicit PlaneVisualizer(QWidget* parent = nullptr);

    void setPlaneParameter(double azimuth, double polar, double distance);
    void setPlaneSize(const QSizeF& size);

    void setVolumeDim(const CTL::VoxelVolume<float>& volume);
    void setVolumeDim(const CTL::VoxelVolume<float>::Dimensions& dimensions,
                      const CTL::VoxelVolume<float>::Offset& offset,
                      const CTL::VoxelVolume<float>::VoxelSize& voxelSize);

public slots:
    void clearScene();
    void resetCamera();
    void resetView();

private:
    QGridLayout* _mainLayout;

    Qt3DExtras::Qt3DWindow* _view;
    Qt3DCore::QEntity* _rootEntity;
    Qt3DRender::QCamera* _camera;
    Qt3DExtras::QOrbitCameraController* _camController;
    Qt3DRender::QMaterial* _defaultMaterial;

    CTL::VoxelVolume<float>::Dimensions _volDim;
    CTL::VoxelVolume<float>::Offset _volOffset;
    CTL::VoxelVolume<float>::VoxelSize _volVoxSize;

    QSizeF _planeSize;
    QVector3D _planeTranslation;
    QQuaternion _planeRotation;

    void addAxis(Qt::Axis axis, float lineLength = 10.0f * PLANEVIS_VIS_SCALE);
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

#endif // CTL_PLANEVISUALIZER_H
