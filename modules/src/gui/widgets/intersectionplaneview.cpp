#include "intersectionplaneview.h"
#include "mat/mat.h"

#include "gui/util/qttype_utils.h"

#include <QGridLayout>
#include <Qt3DCore/QTransform>
#include <Qt3DExtras/QConeMesh>
#include <Qt3DExtras/QCuboidMesh>
#include <Qt3DExtras/QCylinderMesh>
#include <Qt3DExtras/QExtrudedTextMesh>
#include <Qt3DExtras/QOrbitCameraController>
#include <Qt3DExtras/QPhongMaterial>
#include <Qt3DExtras/QPhongAlphaMaterial>
#include <Qt3DExtras/Qt3DWindow>
#include <Qt3DRender/QPointLight>
#include <Qt3DRender/QCamera>
#include <Qt3DRender/QFrameGraphNode>

namespace CTL {
namespace gui {

/*!
 * Constructs an IntersectionPlaneView with the given \a parent.
 *
 * If specified, sets the scaling for the visual appearance of components within the scene to
 * \a visualScale (see CTSystemView::plot() for an example).
 */
IntersectionPlaneView::IntersectionPlaneView(QWidget* parent, float visualScale)
    : QWidget(parent)
    , _mainLayout(new QGridLayout(this))
    , _view(new Qt3DExtras::Qt3DWindow())
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

    setWindowTitle("Intersection plane view");
}

/*!
 * Creates an IntersectionPlaneView for visualization of \a volume and the plane specified by the
 * spherical coordinate tuple (\a azimuth, \a polar, \a distance). Note that only the outer bounding
 * box of \a volume is visualized in the scene for means of simplicity.
 * The \a planeSize argument can be used to explicitely specify how large the intersection plane
 * shall be drawn (sizes in mm). If unset, a default size will be used that is a square with edge
 * length of sqrt(3) times the largest edge length of the volume.
 *
 * If specified, the scaling for the visual appearance of components within the scene is set to
 * \a visualScale.
 *
 * The widget will be deleted automatically if the window is closed.
 *
 * Example:
 * \code
 * // create a volume with total size 100mm x 100mm x 200mm
 * // (note: total size is the product of number of voxels and voxel size)
 * VoxelVolume<float> volume(1, 1, 1, 100.0, 100.0, 200.0);
 *
 * // (static version) using the plot() command
 * gui::IntersectionPlaneView::plot(volume, 10.0_deg, 30.0_deg, 50.0);
 * \endcode
 */
void IntersectionPlaneView::plot(const VoxelVolume<float>& volume,
                                 double azimuth, double polar, double distance,
                                 const QSizeF& planeSize, float visualScale)
{
    auto viewer = new IntersectionPlaneView(nullptr, visualScale);
    viewer->setAttribute(Qt::WA_DeleteOnClose);
    viewer->setVolumeDim(volume);
    viewer->setPlaneParameter(azimuth, polar, distance);
    viewer->setPlaneSize(planeSize);

    viewer->show();
}

/*!
 * Sets the size of the visualized plane to \a width x \a height (both in mm) and updates the
 * visualization.
 *
 * If no plane size is set explicitely (or an empty size is set), a default size will be used
 * that is a square with edge length of sqrt(3) times the largest edge length of the volume.
 */
void IntersectionPlaneView::setPlaneSize(double width, double height)
{
    _planeSize = QSizeF(width, height);

    redraw();
}

/*!
 * Sets the size of the visualized plane to \a size (width x height, in mm) and updates the
 * visualization.
 *
 * If no plane size is set explicitely (or an empty size is set), a default size will be used
 * that is a square with edge length of sqrt(3) times the largest edge length of the volume.
 */
void IntersectionPlaneView::setPlaneSize(const QSizeF& size)
{
    _planeSize = size;

    redraw();
}

/*!
 * Sets the size and position of the volume visualized in the scene based on the specification of
 * \a volume. No copy of actual data is created; only information about the size and position of
 * the volume is extracted.
 *
 * Note that only the outer bounding box of \a volume is visualized in the scene for means of
 * simplicity.
 *
 * Updates the visualization.
 *
 * Example:
 * \code
 * // creating a volume object
 * VoxelVolume<float> volume(100, 200, 300);
 * volume.setVoxelSize(1.5, 0.5, 1.0);
 * volume.setVolumeOffset(0.0f, -50.0f, 0.0f);
 *
 * auto viewer = new gui::IntersectionPlaneView;
 * viewer->setPlaneParameter(10.0_deg, 30.0_deg, 50.0);
 * viewer->setPlaneSize(300.0, 300.0);
 *
 * viewer->setVolumeDim(volume);
 * viewer->show();
 * \endcode
 *
 * Note that the discretization of the volume has no influence on the visualization. Therefore,
 * replacing \c volume in the example above by the following code would yield the same result:
 * \code
 * // creating a volume object with one (big) voxel
 * VoxelVolume<float> volume(1, 1, 1);          // we use a single voxel here
 * volume.setVoxelSize(150.0f, 100.0f, 300.0f); // this is the total size of our volume
 * volume.setVolumeOffset(0.0f, -50.0f, 0.0f);
 *
 * // ...
 * \endcode
 */
void IntersectionPlaneView::setVolumeDim(const VoxelVolume<float>& volume)
{
    setVolumeDim(volume.dimensions(), volume.voxelSize(), volume.offset());
}

/*!
 * Sets the size of the volume visualized in the scene to \a sizeX x \a sizeY x \a sizeZ (all in mm)
 * and sets the position offset to \a offset.
 *
 * Updates the visualization.
 *
 * Example:
 * \code
 * auto viewer = new gui::IntersectionPlaneView;
 * viewer->setPlaneParameter(10.0_deg, 30.0_deg, 50.0);
 * viewer->setPlaneSize(300.0, 300.0);
 * viewer->setVolumeDim(200.0f, 150.0f, 300.0f, {0.0f, 0.0f, 100.0f});
 * viewer->show();
 * \endcode
 */
void IntersectionPlaneView::setVolumeDim(float sizeX, float sizeY, float sizeZ,
                                         const VoxelVolume<float>::Offset& offset)
{
    setVolumeDim( {1, 1, 1 }, {sizeX, sizeY, sizeZ}, offset);
}

/*!
 * Sets the size of the volume visualized in the scene based on \a dimensions and \a voxelSize. An
 * offset can be specified by \a offset to shift the volume to a specific position.
 *
 * Note that only the outer bounding box of the specified volume is visualized in the scene for
 * means of simplicity. This means, in particular, that the number of voxels specified by
 * \a dimensions has no effect, except for being used to determine the total size of the volume
 * (as a product with their individual size).
 *
 * Updates the visualization.
 *
 * Example:
 * \code
 * auto viewer = new gui::IntersectionPlaneView;
 * viewer->setPlaneParameter(10.0_deg, 30.0_deg, 50.0);
 * viewer->setPlaneSize(450.0, 450.0);
 * viewer->setVolumeDim( {256, 256, 256}, {1.0, 1.0, 1.0});
 * viewer->show();
 * \endcode
 */
void IntersectionPlaneView::setVolumeDim(const VoxelVolume<float>::Dimensions& dimensions,
                                         const VoxelVolume<float>::VoxelSize& voxelSize,
                                         const VoxelVolume<float>::Offset& offset)
{
    _volDim = dimensions;
    _volOffset = offset;
    _volVoxSize = voxelSize;

    redraw();
}

// public slots
/*!
 * Clears the scene of this instance. This removes all items that have been added to the scene.
 * Does not remove coordinate axes.
 *
 * \sa resetView().
 */
void IntersectionPlaneView::clearScene()
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
void IntersectionPlaneView::resetCamera()
{
    static const QVector3D startPos( 750.0f, -300.0f, -750.0f );
    _camera->setPosition(startPos);
    _camera->setViewCenter(QVector3D(0.0f, 0.0f, 0.0f));
    _camera->setUpVector(QVector3D(0, -1, 0));
}

/*!
 * Resets the view by clearing its scene and resetting the camera position.
 *
 * \sa clearScene(), resetCamera().
 */
void IntersectionPlaneView::resetView()
{
    clearScene();
    resetCamera();
}

/*!
 * Sets the parameters of the visualized plane to the spherical coordinate tuple (\a azimuth,
 * \a polar, \a distance) and updates the visualization.
 *
 * Example:
 * \code
 * auto viewer = new gui::IntersectionPlaneView;
 * viewer->setPlaneSize(300.0, 300.0);
 * viewer->setVolumeDim(200.0f, 200.0f, 200.0f);
 *
 * // three different planes
 * viewer->setPlaneParameter(10.0_deg, 30.0_deg, 0.0);    // (a)
 * //viewer->setPlaneParameter(10.0_deg, 30.0_deg, 75.0); // (b)
 * //viewer->setPlaneParameter(45.0_deg, 30.0_deg, 0.0);  // (c)
 * //viewer->setPlaneParameter(45.0_deg, 120.0_deg, 0.0); // (d)
 *
 * viewer->show();
 * \endcode
 *
 * ![Resulting visualization (window size and zoom adjusted).](gui/IntersectionPlaneView_planes.png)
 */
void IntersectionPlaneView::setPlaneParameter(double azimuth, double polar, double distance)
{
    using namespace CTL;
    mat::Matrix<3, 1> planeNormal{ std::sin(polar) * std::cos(azimuth),
                                   std::sin(polar) * std::sin(azimuth),
                                   std::cos(polar) };

    mat::Matrix<3, 1> r1, r2(0.0), r3{ planeNormal.get<0>(), planeNormal.get<1>(), planeNormal.get<2>() };

    // find axis that is as most as perpendicular to r3
    uint axis = std::abs(r3.get<0>()) < std::abs(r3.get<1>()) ? 0 : 1;
    axis = std::abs(r3(axis)) < std::abs(r3.get<2>()) ? axis : 2;
    r2(axis) = 1.0;
    r2 = mat::cross(r3, r2);
    r2.normalize();
    r1 = mat::cross(r2, r3);

    auto rotationMatrix = mat::horzcat(mat::horzcat(r1, r2), r3);
    _planeRotation = toQQuaternion(rotationMatrix);
    _planeTranslation = toQVector3D(rotationMatrix * mat::Matrix<3, 1>{ 0.0, 0.0, distance });

    redraw();
}

// private

void IntersectionPlaneView::initializeView()
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

    // add light source
    auto lightEntity = new Qt3DCore::QEntity(_rootEntity);
    auto lightSource = new Qt3DRender::QPointLight;
    auto lightTransform = new Qt3DCore::QTransform;
    lightSource->setColor("white");
    lightSource->setIntensity(0.2f);
    lightTransform->setTranslation( { -5000.0, -5000.0, 0.0} );
    lightEntity->setObjectName("permanent");
    lightEntity->addComponent(lightSource);
    lightEntity->addComponent(lightTransform);

    _mainLayout->addWidget(QWidget::createWindowContainer(_view, this), 0, 0);
    qDebug() << "widget set";
}

void IntersectionPlaneView::addCoordinateSystem()
{
    // Lines
    addAxis(Qt::XAxis);
    addAxis(Qt::YAxis);
    addAxis(Qt::ZAxis);
}

void IntersectionPlaneView::addBoxObject(const QVector3D& dimensions,
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

void IntersectionPlaneView::addAxis(Qt::Axis axis, float lineLength)
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

void IntersectionPlaneView::addVolume()
{
    const QQuaternion identityQuaternion;
    const uint X = _volDim.x, Y = _volDim.y, Z = _volDim.z;
    const QVector3D volumeSize(X * _volVoxSize.x, Y * _volVoxSize.y, Z * _volVoxSize.z);

    // volume offset
    QVector3D volumeOffset(_volOffset.x, _volOffset.y, _volOffset.z);

    auto material = new Qt3DExtras::QPhongMaterial(_rootEntity);
    material->setDiffuse(Qt::darkGray);
    material->setSpecular(Qt::lightGray);

    QVector3D translation(0.0, 0.0, 0.0);
    translation += volumeOffset;
    addBoxObject(volumeSize, translation, identityQuaternion, material);
}

void IntersectionPlaneView::addPlane()
{
    const float PLANE_THICKNESS_RATIO = 0.01f;

    const QSizeF size = _planeSize.isEmpty() ? planeSizeByVolDim() : _planeSize;

    const QVector3D planeSize(size.width(), size.height(),
                              PLANE_THICKNESS_RATIO * std::max(size.width(), size.height()));

    auto material = new Qt3DExtras::QPhongAlphaMaterial(_rootEntity);
    material->setAlpha(90.0f / 255.0f);
    material->setAmbient(Qt::darkGreen);

    addBoxObject(planeSize, _planeTranslation, _planeRotation, material);
}

void IntersectionPlaneView::redraw()
{
    clearScene();
    addPlane();
    addVolume();
    addPlane();
}

QSizeF IntersectionPlaneView::planeSizeByVolDim()
{
    const auto maxDim = std::max(std::max(_volDim.x * _volVoxSize.x,
                                          _volDim.y * _volVoxSize.y),
                                 _volDim.z * _volVoxSize.z);

    return { double(maxDim) * std::sqrt(3.0), double(maxDim) * std::sqrt(3.0) };
}

} // namespace gui
} // namespace CTL
