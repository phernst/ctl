#ifndef CTL_CTSYSTEMVIEW_H
#define CTL_CTSYSTEMVIEW_H

#include <QWidget>

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

namespace CTL {
namespace gui {

class CTSystemView : public QWidget
{
    Q_OBJECT
public:
    explicit CTSystemView(QWidget* parent = nullptr, float visualScale = 50.0f);

    void setCTSystem(const SimpleCTsystem& system);

    static void plot(SimpleCTsystem system, float visualScale = 50.0f);

    void addSystemVisualization(const SimpleCTsystem& system);
    void addVolume(const VoxelVolume<uchar>& volume);

public slots:
    void clearScene();
    void resetCamera();
    void resetView();

protected:
    QGridLayout* _mainLayout;

    Qt3DExtras::Qt3DWindow* _view;
    Qt3DCore::QEntity* _rootEntity;

    void addBoxObject(const QVector3D& dimensions,
                      const QVector3D& translation,
                      const QQuaternion& rotation,
                      Qt3DRender::QMaterial* material = nullptr);
    void addDetectorComponent(AbstractGantry* gantry, AbstractDetector* detector);
    void addSourceComponent(AbstractGantry* gantry, AbstractSource* source);

private:
    Qt3DRender::QCamera* _camera;
    Qt3DExtras::QOrbitCameraController* _camController;
    Qt3DRender::QMaterial* _defaultMaterial;

    float _visualScale = 50.0f;

    void initializeView();
    void addAxis(Qt::Axis axis, float lineLength = 10.0f);
    void addCoordinateSystem();
};

} // namespace gui
} // namespace CTL

#endif // CTL_CTSYSTEMVIEW_H
