#include "planevisualizer.h"
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

PlaneVisualizer::PlaneVisualizer(QWidget* parent)
    : QWidget(parent)
    , _mainLayout(new QGridLayout(this))
    , _view(new Qt3DExtras::Qt3DWindow())
    , _rootEntity(new Qt3DCore::QEntity())
    , _camera(_view->camera())
    , _camController(new Qt3DExtras::QOrbitCameraController(_rootEntity))
    , _defaultMaterial(new Qt3DExtras::QPhongMaterial(_rootEntity))
{
    initializeView();
    resetView();
    addCoordinateSystem();
}

void PlaneVisualizer::setPlaneParameter(double azimuth, double polar, double distance)
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
    r2 /= r2.norm();
    r1 = mat::cross(r2, r3);

    auto rotationMatrix = mat::horzcat(mat::horzcat(r1, r2), r3);
    _planeRotation = toQQuaternion(rotationMatrix);
    _planeTranslation = toQVector3D(rotationMatrix * mat::Matrix<3, 1>{ 0.0, 0.0, distance });

    redraw();
}

void PlaneVisualizer::setPlaneSize(const QSizeF &size)
{
    _planeSize = size;

    redraw();
}

void PlaneVisualizer::setVolumeDim(const CTL::VoxelVolume<float> &volume)
{
    _volDim = volume.dimensions();
    _volOffset = volume.offset();
    _volVoxSize = volume.voxelSize();

    redraw();
}

void PlaneVisualizer::setVolumeDim(const CTL::VoxelVolume<float>::Dimensions &dimensions,
                                   const CTL::VoxelVolume<float>::Offset &offset,
                                   const CTL::VoxelVolume<float>::VoxelSize &voxelSize)
{
    _volDim = dimensions;
    _volOffset = offset;
    _volVoxSize = voxelSize;

    redraw();
}

void PlaneVisualizer::initializeView()
{
    // define permanent objects
    _defaultMaterial->setObjectName("permanent");
    _camController->setObjectName("permanent");

    // initialize camera
    _camera->lens()->setPerspectiveProjection(45.0f, 1.0, 0.1f, 10000.0f);
    _camController->setLinearSpeed(50.0f * PLANEVIS_VIS_SCALE);
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

void PlaneVisualizer::resetCamera()
{
    static const QVector3D startPos( 10.0f * PLANEVIS_VIS_SCALE,
                                    -10.0f * PLANEVIS_VIS_SCALE,
                                    -40.0f * PLANEVIS_VIS_SCALE);
    _camera->setPosition(startPos);
    _camera->setViewCenter(QVector3D(0.0f, 0.0f, 0.0f));
    _camera->setUpVector(QVector3D(0, -1, 0));
}

void PlaneVisualizer::resetView()
{
    clearScene();
    resetCamera();
}

void PlaneVisualizer::clearScene()
{
    QList<QObject*> deleteList;

    for(auto child : _rootEntity->children())
        if(child->objectName() != "permanent")
            deleteList.append(child);

    for(auto obj : deleteList)
        delete obj;
}

void PlaneVisualizer::addCoordinateSystem()
{
    // Lines
    addAxis(Qt::XAxis);
    addAxis(Qt::YAxis);
    addAxis(Qt::ZAxis);
}

void PlaneVisualizer::addBoxObject(const QVector3D& dimensions,
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

void PlaneVisualizer::addAxis(Qt::Axis axis, float lineLength)
{
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

    lineMesh->setRadius(LINE_THICKNESS * PLANEVIS_VIS_SCALE);
    lineMesh->setLength(lineLength);

    coneMesh->setLength(1.0f * PLANEVIS_VIS_SCALE);
    coneMesh->setBottomRadius(2.0f * LINE_THICKNESS * PLANEVIS_VIS_SCALE);

    auto fnt = textMesh->font();
    fnt.setPixelSize(int(RELATIVE_TEXT_SIZE * PLANEVIS_VIS_SCALE) + 1);
    textMesh->setFont(fnt);
    textMesh->setDepth(0.1f * PLANEVIS_VIS_SCALE);

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

void PlaneVisualizer::addVolume()
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

void PlaneVisualizer::addPlane()
{
    const float PLANE_THICKNESS_RATIO = 0.01f;

    const QVector3D planeSize(_planeSize.width(), _planeSize.height(),
                              PLANE_THICKNESS_RATIO * std::max(_planeSize.width(), _planeSize.height()));

    auto material = new Qt3DExtras::QPhongAlphaMaterial(_rootEntity);
    material->setAlpha(90.0f / 255.0f);
    material->setAmbient(Qt::darkGreen);

    addBoxObject(planeSize, _planeTranslation, _planeRotation, material);
}

void PlaneVisualizer::redraw()
{
    clearScene();
    addPlane();
    addVolume();
    addPlane();
}
