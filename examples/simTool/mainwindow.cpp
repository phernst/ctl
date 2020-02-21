#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "acquisition/ctsystembuilder.h"
#include "acquisition/systemblueprints.h"
#include "acquisition/trajectories.h"
#include "io/den/den.h"
#include "mat/mat.h"

#include "ocl/openclconfig.h"
#include "projectors/raycasterprojector.h"
#include "projectors/arealfocalspotextension.h"
#include "projectors/poissonnoiseextension.h"

#include <QFileDialog>

using namespace CTL;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    _CTSystem(SimpleCTsystem::fromCTsystem(CTsystemBuilder::createFromBlueprint(blueprints::GenericTubularCT())))
{
    ui->setupUi(this);

    setWindowTitle("CT-Simulator");

    if (!OCL::OpenCLConfig::instance().isValid())
        throw std::runtime_error("no OpenCL device found (GPU or CPU)!");

    updateGeometryPreview();

    // linear arrangement of detector modules
    ui->_W_projectionViewer->setModuleLayout(
                ModuleLayout::canonicLayout(1, _CTSystem.detector()->nbDetectorModules()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::updateGeometryPreview()
{
    ui->_W_geometryPreview->setAcquisitionSetup(currentSetup());

    if(ui->_SB_nbViews->value() == 1)
        ui->_W_geometryPreview->showFullAcquisition();
    else
        ui->_W_geometryPreview->showSourceTrajectory();
}

void MainWindow::updatePitchFactor()
{
    static const double detWidth = _CTSystem.detector()->pixelDimensions().height() *
            _CTSystem.detector()->nbPixelPerModule().height();
    double pitchFactor = double(ui->_SB_nbViews->value()) / double(ui->_SB_nbRotations->value()) *
            ui->_SB_tableFeed->value() / detWidth;

    ui->_L_pitchFactor->setText(QString::number(pitchFactor));
}

void MainWindow::on__PB_loadPhantom_clicked()
{
    auto fn = QFileDialog::getOpenFileName(this, "Load object data", QDir::currentPath(), "*.den");

    if(fn.isEmpty())
        return;

    ui->_LE_loadFileName->setText(fn);

    loadDenFile(fn);

    ui->_statusBar->showMessage("Object data loaded.");
}

void MainWindow::loadDenFile(const QString &fileName)
{
    io::BaseTypeIO<io::DenFileIO> fileIO;

    auto dataType = io::den::getDataType(fileName);

    switch (dataType) {
    case io::den::Type::unDef:
        return;
    case io::den::Type::uChar:
    {
        auto volume = fileIO.readVolume<uchar>(fileName);
        setVolumeData(volume);
        break;
    }
    case io::den::Type::uShort:
    {
        auto volume = fileIO.readVolume<ushort>(fileName);
        setVolumeData(volume);
        _volumeData = _volumeData - 1000.0f;
        break;
    }
    case io::den::Type::Float:
    {
        auto volume = fileIO.readVolume<float>(fileName);
        setVolumeData(volume);
        break;
    }
    case io::den::Type::Double:
    {
        auto volume = fileIO.readVolume<double>(fileName);
        setVolumeData(volume);
        break;
     }
    }

    ui->_W_volumeView->setVolumeData(_volumeData);
    setDimensionLabelText(_volumeData.nbVoxels().x,
                          _volumeData.nbVoxels().y,
                          _volumeData.nbVoxels().z);
}

void MainWindow::saveDenFile(const QString &fileName)
{
    io::BaseTypeIO<io::DenFileIO> fileIO;
    bool ok;

    ok = fileIO.write(GeometryEncoder::encodeFullGeometry(currentSetup()), fileName + "_pmat");
    if(ok)
        ui->_statusBar->showMessage("Projection matrices saved successfully.");

    ok = fileIO.write(_projectionData.combined(
                            ModuleLayout::canonicLayout(1, _projectionData.dimensions().nbModules)),
                      fileName);
    if(ok)
        ui->_statusBar->showMessage("Projections saved successfully.");
}

void MainWindow::setDimensionLabelText(uint x, uint y, uint z)
{
    QString text = QString::number(x) + " x " +
            QString::number(y) + " x " +
            QString::number(z);

    ui->_L_phantomDim->setText(text);
}

AcquisitionSetup MainWindow::currentSetup() const
{
    constexpr double DEG_TO_RAD = PI / 180.0;

    // gather acquisition parameter from GUI
    auto nbViews = ui->_SB_nbViews->value();
    auto nbRotations = ui->_SB_nbRotations->value();
    auto tableFeed = ui->_SB_tableFeed->value();
    auto startAngle = ui->_SB_initGantryAngle->value() * DEG_TO_RAD;
    auto startPitch = ui->_SB_initTablePos->value();

    const auto angleIncrement = double(360.0_deg) * double(nbRotations) / double(nbViews);

    // set focal spot
    QSizeF focalSpotSize(ui->_SB_focalSpotX->value(),
                         ui->_SB_focalSpotY->value());
    _CTSystem.source()->setFocalSpotSize(focalSpotSize);

    // set photons
    auto nbPhotons = ui->_SB_nbPhotons->value();
    auto reqOutputFactor = double(nbPhotons) / _CTSystem.photonsPerPixelMean();
    auto reqOutputValue = static_cast<XrayLaser*>(_CTSystem.source())->radiationOutput() *
            reqOutputFactor;
    static_cast<XrayLaser*>(_CTSystem.source())->setRadiationOutput(double(reqOutputValue));

    AcquisitionSetup acqSetup(_CTSystem);
    acqSetup.setNbViews(nbViews);
    acqSetup.applyPreparationProtocol(protocols::HelicalTrajectory(angleIncrement, tableFeed,
                                                                   startPitch, startAngle));

    return acqSetup;
}

void MainWindow::on__PB_simulateScan_clicked()
{
    ui->_statusBar->showMessage("Simulation running...");

    auto acqSetup = currentSetup();

    CTL::VoxelVolume<float>::VoxelSize voxSize = { float(ui->_SB_voxSizeX->value()),
                                                   float(ui->_SB_voxSizeY->value()),
                                                   float(ui->_SB_voxSizeZ->value()) };
    _volumeData.setVoxelSize(voxSize);

    AbstractProjector* projector = nullptr;

    AbstractProjector* actualForwardProj = new OCL::RayCasterProjector;
    projector = actualForwardProj;

    if(ui->_CB_simulateFocalSpot->isChecked())
    {
        auto areaExt = new ArealFocalSpotExtension(projector);
        areaExt->setDiscretization(QSize(5, 5));
        projector = areaExt;
    }
    if(ui->_CB_simulatePoisson->isChecked())
    {
        auto poissonExt = new PoissonNoiseExtension(projector);
        projector = poissonExt;
    }

    projector->configure(acqSetup);

    QObject::connect(projector->notifier(), &ProjectorNotifier::projectionFinished, this, &MainWindow::showProjectionDone);

    if(ui->_RB_hounsfield->isChecked())
    {        
        const auto muWater = ui->_SB_muWater->value();
        _projectionData = projector->project((_volumeData*(muWater/1000.0)) + muWater);
    }
    else
        _projectionData = projector->project(_volumeData);

    ui->_W_projectionViewer->setData(_projectionData);

    delete projector;

    ui->_statusBar->showMessage("Simulation finished.");
}

void MainWindow::on__PB_saveProj_clicked()
{
    auto fn = QFileDialog::getSaveFileName(this, "Save projection data", QDir::currentPath(), ".den");

    if(fn.isEmpty())
        return;

    saveDenFile(fn);
}

void MainWindow::showProjectionDone(int view)
{
    ui->_statusBar->showMessage("Projection " + QString::number(view) + " finished.");
    QApplication::processEvents();
}


void MainWindow::on__PB_fitToVolumeSize_clicked()
{
    double volLength = double(_volumeData.nbVoxels().z) * ui->_SB_voxSizeZ->value();
    double reqPitch = volLength / double(ui->_SB_nbViews->value() - 1);

    ui->_SB_tableFeed->setValue(reqPitch);
}

void MainWindow::on__PB_centerTrajectory_clicked()
{
    double trajectoryLength = double(ui->_SB_nbViews->value() - 1) * ui->_SB_tableFeed->value();

    ui->_SB_initTablePos->setValue(-trajectoryLength / 2.0);
}

void MainWindow::on__SB_nbViews_valueChanged(int nbViews)
{
    ui->_PB_fitToVolumeSize->setEnabled(nbViews != 1);
}
