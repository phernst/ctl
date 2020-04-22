#ifndef CTL_CTSYSTEMVIEW_H
#define CTL_CTSYSTEMVIEW_H

#include <QWidget>
#include <Qt3DWindow>

// forward delcarations
namespace CTL {
class AbstractDetector;
class AbstractGantry;
class AbstractSource;
class SimpleCTSystem;
template <typename T>
class VoxelVolume;
} // namespace CTL
class QGridLayout;
namespace Qt3DCore {
class QEntity;
} // namespace Qt3DCore
namespace Qt3DExtras {
class QOrbitCameraController;
} // namespace Qt3DExtras
namespace Qt3DRender {
class QCamera;
class QMaterial;
} // namespace Qt3DRender


namespace CTL {
namespace gui {

namespace details {
class CTL3DWindow;
}

/*!
 * \class CTSystemView
 *
 * \brief The CTSystemView class provides a tool for visualization of a SimpleCTSystem
 *
 * This class can be used to visualize the positioning of source and detector component of a
 * SimpleCTSystem in an interactive 3D viewer. For convenience, the plot() method can be used to
 * achieve a one-line solution, creating a widget that will be destroyed once it is closed by the
 * user.
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
 * The system to be visualized is set via setCTSystem(). It is also possible to add further systems
 * to the same visualization scene with addSystemVisualization(). To clear all previously added
 * systems from the scene, use clearScene(). Camera position can be reset to its original setting
 * by resetCamera(). The resetView() combines the effects of clearing the scene and resetting the
 * camera.
 *
 * Example:
 * \code
 * auto system = SimpleCTSystem::fromCTSystem(
 *             CTSystemBuilder::createFromBlueprint(blueprints::GenericTubularCT()));
 *
 * // (static version) using the plot() command
 * gui::CTSystemView::plot(system);
 *
 * // (property-based version)
 * auto viewer = new gui::CTSystemView; // needs to be deleted at an appropriate time
 * viewer->setCTSystem(system);
 * viewer->show();
 * \endcode
 *
 * ![Visualization (static version) of a system, created from the GenericTubularCT blueprint.](gui/CTSystemView.png)
 */

class CTSystemView : public QWidget
{
    Q_OBJECT
public:
    explicit CTSystemView(QWidget* parent = nullptr, float visualScale = 50.0f);

    void setCTSystem(const SimpleCTSystem& system);

    static void plot(SimpleCTSystem system, float visualScale = 50.0f);

    void addSystemVisualization(const SimpleCTSystem& system);
    void addVolume(const VoxelVolume<uchar>& volume);

public slots:
    void clearScene();
    void resetCamera();
    void resetView();

protected:
    QGridLayout* _mainLayout;

    details::CTL3DWindow* _view;
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

namespace details {

class CTL3DWindow : public Qt3DExtras::Qt3DWindow
{
    Q_OBJECT
public:
    using Qt3DExtras::Qt3DWindow::Qt3DWindow;

protected:
    void keyPressEvent(QKeyEvent* e) override;

signals:
    void saveRequest();
};

} // namespace details

} // namespace gui
} // namespace CTL

#endif // CTL_CTSYSTEMVIEW_H
