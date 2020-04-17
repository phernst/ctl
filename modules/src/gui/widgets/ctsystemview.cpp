#include "ctsystemview.h"
#include "components/genericdetector.h"
#include "components/genericgantry.h"
#include "components/genericsource.h"
#include "acquisition/simplectsystem.h"
#include "img/voxelvolume.h"
#include "gui/util/qttype_utils.h"

#include <QGridLayout>
#include <QKeyEvent>
#include <Qt3DCore/QTransform>
#include <Qt3DExtras/QConeMesh>
#include <Qt3DExtras/QCuboidMesh>
#include <Qt3DExtras/QCylinderMesh>
#include <Qt3DExtras/QExtrudedTextMesh>
#include <Qt3DExtras/QOrbitCameraController>
#include <Qt3DExtras/QPhongMaterial>
#include <Qt3DExtras/QPhongAlphaMaterial>
#include <Qt3DRender/QCamera>

namespace CTL {
namespace gui {

/*!
 * Creates a CTSystemView and sets its parent to \a parent.
 *
 * If specified, sets the scaling for the visual appearance of components within the scene to
 * \a visualScale.
 */
CTSystemView::CTSystemView(QWidget* parent, float visualScale)
    : QWidget(parent)
    , _mainLayout(new QGridLayout(this))
    , _view(new details::CTL3DWindow)
    , _rootEntity(new Qt3DCore::QEntity())
    , _camera(_view->camera())
    , _camController(new Qt3DExtras::QOrbitCameraController(_rootEntity))
    , _defaultMaterial(new Qt3DExtras::QPhongMaterial(_rootEntity))
    , _visualScale(visualScale)
{
    initializeView();
    resetView();
    addCoordinateSystem();

    resize(800, 600);
    setWindowTitle("CT system view");
}

/*!
 * Creates a CTSystemView for visualization of \a system and shows the window.
 *
 * If specified, sets the scaling for the visual appearance of components within the scene to
 * \a visualScale.
 *
 * The widget will be deleted automatically if the window is closed.
 *
 * Example: create a visualization of a GenericTubularCT with three different visual scales
 * \code
 * auto system = SimpleCTSystem::fromCTsystem(
 *             CTsystemBuilder::createFromBlueprint(blueprints::GenericTubularCT()));
 *
 * gui::CTSystemView::plot(system, 25.0f);
 * gui::CTSystemView::plot(system);        // visualScale = 50 [default value]
 * gui::CTSystemView::plot(system, 75.0f);
 * \endcode
 *
 * ![Resulting visualization from the example above (window size and zoom adjusted).](gui/CTSystemView_scales.png)
 */
void CTSystemView::plot(SimpleCTSystem system, float visualScale)
{
    auto viewer = new CTSystemView(nullptr, visualScale);
    viewer->setAttribute(Qt::WA_DeleteOnClose);
    viewer->setCTSystem(std::move(system));

    viewer->show();
}

/*!
 * Sets the system to be visualized by this instance to \a system. This overrides any previous
 * visualization.
 */
void CTSystemView::setCTSystem(const SimpleCTSystem& system)
{
    clearScene();
    addSystemVisualization(system);
}

/*!
 * Adds a visualization of \a system to the scene of this instance.
 *
 * Example:
 * \code
 * auto system = SimpleCTSystem::fromCTsystem(
 *             CTsystemBuilder::createFromBlueprint(blueprints::GenericTubularCT()));
 *
 * auto viewer = new gui::CTSystemView;
 * viewer->setCTSystem(system);
 *
 * // rotate system to 45 degree position and visualize as well
 * static_cast<TubularGantry*>(system.gantry())->setRotationAngle(45.0_deg);
 * viewer->addSystemVisualization(system);
 *
 * viewer->show();
 * \endcode
 *
 * ![Resulting visualization. The view has been zoomed in and its camera position adjusted slightly for better visibility.](gui/CTSystemView_2systems.png)
 */
void CTSystemView::addSystemVisualization(const SimpleCTSystem& system)
{
    addDetectorComponent(system.gantry(), system.detector());
    addSourceComponent(system.gantry(), system.source());
}

/*!
 * Adds a visualization of \a volume to the scene of this instance. The volume will be shown in its
 * real dimensions and voxels will appear as translucent black boxes with alpha channel
 * corresponding to the value of the voxel (higher values appear less transparent).
 *
 * Note that, if once added, the volume visualization will be permanent throughout the lifetime of
 * this instance. That means in particular that it will not be removed when calling clearScene() or
 * resetView().
 *
 * Example:
 * \code
 * auto system = SimpleCTSystem::fromCTsystem(
 *             CTsystemBuilder::createFromBlueprint(blueprints::GenericTubularCT()));
 *
 * // create a volume with 10x10x10 voxels (each of size 10mm x 10mm x 10mm)
 * auto volume = VoxelVolume<uchar>(10,10,10,10.0,10.0,10.0);
 * volume.fill(20); // fill with value 20 for a highly transparent visualization
 *
 * auto viewer = new gui::CTSystemView;
 * viewer->setCTSystem(system);
 * viewer->addVolume(vol);
 * viewer->show();
 * \endcode
 *
 * ![Resulting visualization. The view has been zoomed in for better visibility.](gui/CTSystemView_volume.png)
 */
void CTSystemView::addVolume(const VoxelVolume<uchar>& volume)
{
    const QQuaternion identityQuaternion;
    const QVector3D voxelSize(volume.voxelSize().x, volume.voxelSize().y, volume.voxelSize().z);
    const uint X = volume.nbVoxels().x, Y = volume.nbVoxels().y, Z = volume.nbVoxels().z;
    // volume offset
    QVector3D volExtentCompensation(X - 1, Y - 1, Z - 1);
    volExtentCompensation *= 0.5f * voxelSize;
    QVector3D volumeOffset(volume.offset().x, volume.offset().y, volume.offset().z);
    volumeOffset -= volExtentCompensation;

    for(uint x = 0; x < X; ++x)
        for(uint y = 0; y < Y; ++y)
            for(uint z = 0; z < Z; ++z)
                if(volume(x,y,z))
                {
                    auto material = new Qt3DExtras::QPhongAlphaMaterial(_rootEntity);
                    material->setAlpha(volume(x,y,z) / 255.0f);
                    material->setObjectName("permanent");

                    QVector3D translation(x * voxelSize.x(), y * voxelSize.y(), z * voxelSize.z());
                    translation += volumeOffset;
                    addBoxObject(voxelSize, translation, identityQuaternion, material);
                }
}

/*!
 * Clears the scene of this instance. This removes all system visualizations that have been added to
 * the scene.
 *
 * Note that this does not remove coordinate axes and visualized volumes (if those had been added).
 *
 * \sa addVolume(), resetView().
 */
void CTSystemView::clearScene()
{
    QList<QObject*> deleteList;

    for(auto child : _rootEntity->children())
        if(child->objectName() != "permanent")
            deleteList.append(child);

    for(auto obj : deleteList)
        delete obj;
}

/*!
 * Restores the initial camera position.
 */
void CTSystemView::resetCamera()
{
    static const QVector3D startPos(10.0f * _visualScale, -10.0f * _visualScale, -40.0f * _visualScale);
    _camera->setPosition(startPos);
    _camera->setViewCenter(QVector3D(0.0f, 0.0f, 0.0f));
    _camera->setUpVector(QVector3D(0, -1, 0));
}

/*!
 * Resets the view by clearing its scene and resetting the camera position.
 *
 * \sa clearScene(), resetCamera().
 */
void CTSystemView::resetView()
{
    clearScene();
    resetCamera();
}

void CTSystemView::initializeView()
{
    // define permanent objects
    _defaultMaterial->setObjectName("permanent");
    _camController->setObjectName("permanent");

    // initialize camera
    _camera->lens()->setPerspectiveProjection(45.0f, 1.0, 0.1f, 10000.0f);
    _camController->setLinearSpeed(50.0f * _visualScale);
    _camController->setLookSpeed(180.0f);
    _camController->setCamera(_camera);
    qDebug() << "camera set up";

    _view->setRootEntity(_rootEntity);
    qDebug() << "prepare finished";

    _mainLayout->addWidget(QWidget::createWindowContainer(_view, this), 0, 0);
    qDebug() << "widget set";
}

void CTSystemView::addCoordinateSystem()
{
    // Lines
    addAxis(Qt::XAxis);
    addAxis(Qt::YAxis);
    addAxis(Qt::ZAxis);
}

void CTSystemView::addBoxObject(const QVector3D& dimensions,
                                const QVector3D& translation,
                                const QQuaternion& rotation,
                                Qt3DRender::QMaterial *material)
{
    auto boxEntity = new Qt3DCore::QEntity(_rootEntity);
    auto boxMesh = new Qt3DExtras::QCuboidMesh;
    auto boxTransform = new Qt3DCore::QTransform;

    if(material)
        boxEntity->setObjectName(material->objectName());
    else
        material = _defaultMaterial;

    boxMesh->setXExtent(dimensions.x());
    boxMesh->setYExtent(dimensions.y());
    boxMesh->setZExtent(dimensions.z());

    boxTransform->setTranslation(translation);
    boxTransform->setRotation(rotation);

    boxEntity->addComponent(boxMesh);
    boxEntity->addComponent(boxTransform);
    boxEntity->addComponent(material);
}

void CTSystemView::addDetectorComponent(AbstractGantry* gantry, AbstractDetector* detector)
{
    static constexpr float MOD_THICKNESS = 0.1f;

    const auto detPos = gantry->detectorPosition();
    const auto detRot = gantry->detectorRotation();

    const auto modSize = detector->moduleDimensions();
    const QVector3D moduleBoxSize(modSize.width(), modSize.height(), MOD_THICKNESS * _visualScale);

    for(uint mod = 0; mod < detector->nbDetectorModules(); ++mod)
    {
        auto modLoc = detector->moduleLocation(mod);

        auto modulePos = detPos + detRot.transposed() * modLoc.position;
        auto moduleRot = modLoc.rotation * detRot;

        auto moduleRotQuaternion = toQQuaternion(moduleRot.transposed());
        auto modulePosQVector = toQVector3D(modulePos);

        QVector3D boxExtentCompensation(0.0f, 0.0f, moduleBoxSize.z() / 2.0f);
        boxExtentCompensation = moduleRotQuaternion.rotatedVector(boxExtentCompensation);

        addBoxObject(moduleBoxSize, modulePosQVector + boxExtentCompensation, moduleRotQuaternion);
    }
}

void CTSystemView::addSourceComponent(AbstractGantry* gantry, AbstractSource* )
{
    static constexpr float SRC_LENGTH = 1.0f;

    const QVector3D srcBoxSize(0.25f * _visualScale, 0.25f * _visualScale, SRC_LENGTH * _visualScale);

    auto srcPos = gantry->sourcePosition();
    auto srcRot = gantry->sourceRotation();

    auto srcRotQuaternion = toQQuaternion(srcRot);
    auto srcPosQVector = toQVector3D(srcPos);

    QVector3D boxExtentCompensation(0.0f, 0.0f, -srcBoxSize.z() / 2.0f);
    boxExtentCompensation = srcRotQuaternion.rotatedVector(boxExtentCompensation);

    addBoxObject(srcBoxSize, srcPosQVector + boxExtentCompensation, srcRotQuaternion);
}

void CTSystemView::addAxis(Qt::Axis axis, float lineLength)
{
    lineLength *= _visualScale;

    const float LINE_THICKNESS = 0.05f;
    const float RELATIVE_TEXT_SIZE = 0.666f;

    auto transformAxis = new Qt3DCore::QTransform;
    auto transformCone = new Qt3DCore::QTransform;
    auto transformText = new Qt3DCore::QTransform;

    auto lineEntity = new Qt3DCore::QEntity(_rootEntity);
    auto coneEntity = new Qt3DCore::QEntity(_rootEntity);
    auto textEntity = new Qt3DCore::QEntity(_rootEntity);

    auto lineMesh = new Qt3DExtras::QCylinderMesh;
    auto coneMesh = new Qt3DExtras::QConeMesh;
    auto textMesh = new Qt3DExtras::QExtrudedTextMesh;

    lineMesh->setRadius(LINE_THICKNESS * _visualScale);
    lineMesh->setLength(lineLength);

    coneMesh->setLength(1.0f * _visualScale);
    coneMesh->setBottomRadius(2.0f * LINE_THICKNESS * _visualScale);

    auto fnt = textMesh->font();
    fnt.setPixelSize(int(RELATIVE_TEXT_SIZE * _visualScale) + 1);
    textMesh->setFont(fnt);
    textMesh->setDepth(0.1f * _visualScale);

    // compute tranformations depending on chosen axis
    const float lineCenterOffset = lineLength / 2.0f;
    const float textOffset = 1.2f * lineCenterOffset;

    auto axisMaterial = new Qt3DExtras::QPhongMaterial(_rootEntity);
    axisMaterial->setObjectName("permanent");

    switch(axis)
    {
    case Qt::XAxis:
        transformAxis->setRotationZ(-90.0f);

        transformCone->setRotationZ(-90.0f);
        transformCone->setTranslation(QVector3D(lineCenterOffset, 0.0f, 0.0f));

        transformText->setTranslation(QVector3D(textOffset, 0.0f, 0.0f));
        textMesh->setText("x");

        axisMaterial->setAmbient(Qt::red);
        break;
    case Qt::YAxis:
        transformCone->setTranslation(QVector3D(0.0f, lineCenterOffset, 0.0f));

        transformText->setTranslation(QVector3D(0.0f, textOffset, 0.0f));
        textMesh->setText("y");

        axisMaterial->setAmbient(Qt::darkGreen);
        break;
    case Qt::ZAxis:
        transformAxis->setRotationX(90.0f);

        transformCone->setRotationX(90.0f);
        transformCone->setTranslation(QVector3D(0.0f, 0.0f, lineCenterOffset));

        transformText->setTranslation(QVector3D(0.0f, 0.0f, textOffset));
        textMesh->setText("z");

        axisMaterial->setAmbient(Qt::blue);
        break;
    }
    transformText->setRotationY(180.0f);

    textEntity->setObjectName("permanent");
    textEntity->addComponent(textMesh);
    textEntity->addComponent(transformText);
    textEntity->addComponent(_defaultMaterial);

    lineEntity->setObjectName("permanent");
    lineEntity->addComponent(lineMesh);
    lineEntity->addComponent(transformAxis);
    lineEntity->addComponent(axisMaterial);

    coneEntity->setObjectName("permanent");
    coneEntity->addComponent(coneMesh);
    coneEntity->addComponent(transformCone);
    coneEntity->addComponent(axisMaterial);
}

void details::CTL3DWindow::keyPressEvent(QKeyEvent* e)
{
    if(e->modifiers() == Qt::CTRL && e->key() == Qt::Key_S)
        emit saveRequest();
    else
        Qt3DWindow::keyPressEvent(e);
}

} // namespace gui
} // namespace CTL
