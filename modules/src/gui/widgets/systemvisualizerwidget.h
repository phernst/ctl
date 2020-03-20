#ifndef CTL_SYSTEMVISUALIZERWIDGET_H
#define CTL_SYSTEMVISUALIZERWIDGET_H

#include <QWidget>

const float VIS_SCALE = 50.0f;

// forward delcarations
namespace CTL {
class AbstractDetector;
class AbstractGantry;
class AbstractSource;
class SimpleCTsystem;
template <typename T>
class VoxelVolume;
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

class SystemVisualizerWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SystemVisualizerWidget(QWidget* parent = nullptr);

signals:

public slots:
    void clearScene();
    void resetCamera();
    void resetView();

    void visualizeSystem(const CTL::SimpleCTsystem& system);

    void addSystemVisualization(const CTL::SimpleCTsystem& system);
    void addDetectorComponent(CTL::AbstractGantry* gantry, CTL::AbstractDetector* detector);
    void addSourceComponent(CTL::AbstractGantry* gantry, CTL::AbstractSource* source);
    void addVolume(const CTL::VoxelVolume<uchar>& volume);

protected:
    QGridLayout* _mainLayout;

    Qt3DExtras::Qt3DWindow* _view;
    Qt3DCore::QEntity* _rootEntity;

    void addBoxObject(const QVector3D& dimensions,
                      const QVector3D& translation,
                      const QQuaternion& rotation,
                      Qt3DRender::QMaterial* material = nullptr);

private:
    Qt3DRender::QCamera* _camera;
    Qt3DExtras::QOrbitCameraController* _camController;
    Qt3DRender::QMaterial* _defaultMaterial;

    void initializeView();
    void addAxis(Qt::Axis axis, float lineLength = 10.0f * VIS_SCALE);
    void addCoordinateSystem();
};

#endif // CTL_SYSTEMVISUALIZERWIDGET_H
