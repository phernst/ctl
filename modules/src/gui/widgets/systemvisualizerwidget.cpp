#include "systemvisualizerwidget.h"
#include "components/genericdetector.h"
#include "components/genericgantry.h"
#include "components/genericsource.h"
#include "acquisition/simplectsystem.h"
#include "img/voxelvolume.h"
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
#include <Qt3DRender/QCamera>
#include <Qt3DRender/QFrameGraphNode>

SystemVisualizerWidget::SystemVisualizerWidget(QWidget* parent)
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

void SystemVisualizerWidget::visualizeSystem(const CTL::SimpleCTsystem& system)
{
    clearScene();
    addSystemVisualization(system);
}

void SystemVisualizerWidget::addSystemVisualization(const CTL::SimpleCTsystem& system)
{
    addDetectorComponent(system.gantry(), system.detector());
    addSourceComponent(system.gantry(), system.source());
}

void SystemVisualizerWidget::initializeView()
{
    // define permanent objects
    _defaultMaterial->setObjectName("permanent");
    _camController->setObjectName("permanent");

    // initialize camera
    _camera->lens()->setPerspectiveProjection(45.0f, 1.0, 0.1f, 10000.0f);
    _camController->setLinearSpeed(50.0f * VIS_SCALE);
    _camController->setLookSpeed(180.0f);
    _camController->setCamera(_camera);
    qDebug() << "camera set up";

    _view->setRootEntity(_rootEntity);
    qDebug() << "prepare finished";

    _mainLayout->addWidget(QWidget::createWindowContainer(_view, this), 0, 0);
    qDebug() << "widget set";
}

void SystemVisualizerWidget::resetCamera()
{
    static const QVector3D startPos(10.0f * VIS_SCALE, -10.0f * VIS_SCALE, -40.0f * VIS_SCALE);
    _camera->setPosition(startPos);
    _camera->setViewCenter(QVector3D(0.0f, 0.0f, 0.0f));
    _camera->setUpVector(QVector3D(0, -1, 0));
}

void SystemVisualizerWidget::resetView()
{
    clearScene();
    resetCamera();
}

void SystemVisualizerWidget::clearScene()
{
    QList<QObject*> deleteList;

    for(auto child : _rootEntity->children())
        if(child->objectName() != "permanent")
            deleteList.append(child);

    for(auto obj : deleteList)
        delete obj;
}

void SystemVisualizerWidget::addCoordinateSystem()
{
    // Lines
    addAxis(Qt::XAxis);
    addAxis(Qt::YAxis);
    addAxis(Qt::ZAxis);
}

void SystemVisualizerWidget::addBoxObject(const QVector3D& dimensions,
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

void SystemVisualizerWidget::addDetectorComponent(CTL::AbstractGantry* gantry,
                                                  CTL::AbstractDetector* detector)
{
    static constexpr float MOD_THICKNESS = 0.1f;

    const auto detPos = gantry->detectorPosition();
    const auto detRot = gantry->detectorRotation();

    const auto modSize = detector->moduleDimensions();
    const QVector3D moduleBoxSize(modSize.width(), modSize.height(), MOD_THICKNESS * VIS_SCALE);

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

void SystemVisualizerWidget::addSourceComponent(CTL::AbstractGantry* gantry,
                                                CTL::AbstractSource* )
{
    static constexpr float SRC_LENGTH = 1.0f;

    const QVector3D srcBoxSize(0.25f * VIS_SCALE, 0.25f * VIS_SCALE, SRC_LENGTH * VIS_SCALE);

    auto srcPos = gantry->sourcePosition();
    auto srcRot = gantry->sourceRotation();

    auto srcRotQuaternion = toQQuaternion(srcRot);
    auto srcPosQVector = toQVector3D(srcPos);

    QVector3D boxExtentCompensation(0.0f, 0.0f, -srcBoxSize.z() / 2.0f);
    boxExtentCompensation = srcRotQuaternion.rotatedVector(boxExtentCompensation);

    addBoxObject(srcBoxSize, srcPosQVector + boxExtentCompensation, srcRotQuaternion);
}

void SystemVisualizerWidget::addVolume(const CTL::VoxelVolume<uchar>& volume)
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

void SystemVisualizerWidget::addAxis(Qt::Axis axis, float lineLength)
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

    lineMesh->setRadius(LINE_THICKNESS * VIS_SCALE);
    lineMesh->setLength(lineLength);

    coneMesh->setLength(1.0f * VIS_SCALE);
    coneMesh->setBottomRadius(2.0f * LINE_THICKNESS * VIS_SCALE);

    auto fnt = textMesh->font();
    fnt.setPixelSize(int(RELATIVE_TEXT_SIZE * VIS_SCALE) + 1);
    textMesh->setFont(fnt);
    textMesh->setDepth(0.1f * VIS_SCALE);

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
