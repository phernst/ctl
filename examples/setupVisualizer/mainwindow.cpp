#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "acquisition/acquisitionsetup.h"
#include "acquisition/geometrydecoder.h"
#include "acquisition/trajectories.h"
#include "components/carmgantry.h"
#include "components/cylindricaldetector.h"
#include "components/flatpaneldetector.h"
#include "components/tubulargantry.h"
#include "components/xraylaser.h"
#include "gui/widgets/acquisitionsetupview.h"
#include "img/voxelvolume.h"
#include "io/den/denfileio.h"
#include "mat/mat.h"

#include <QDebug>
#include <QFile>
#include <QtWidgets/QFileDialog>

const int ANIM_FRAME_TIME = 42;

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setWindowTitle("Visualizer");
    _avWid = ui->widget;

    constructDefaultComponents();

    _avWid->setFocus();
}

MainWindow::~MainWindow()
{
    delete _flatPanel;
    delete _curvedDetector;
    delete _source;
    delete _carmGantry;

    delete ui;
}

void MainWindow::constructDefaultComponents()
{
    _flatPanel = new CTL::FlatPanelDetector(QSize(600, 500), QSizeF(0.5, 0.5));
    _curvedDetector = new CTL::CylindricalDetector(QSize(16, 64), QSizeF(1.0, 1.0), 40, 1.0_deg,
                                                   0.1);
    _source = new CTL::XrayLaser("X-ray source");
    _carmGantry = new CTL::CarmGantry(1200.0);
}

void MainWindow::on__PB_circTraj_clicked()
{
    // get parameter
    auto srcToDet = ui->_SL_srcToDetDist->value();
    auto srcToIso = ui->_SL_srcToIsoDist->value();
    auto nbProj = ui->_SB_nbProj->value();

    // set-up system
    CTL::CTsystem mySystem;
    auto gantryCpy = _carmGantry->clone();
    static_cast<CTL::CarmGantry*>(gantryCpy)->setCarmSpan(srcToDet);

    mySystem.addComponent(_source->clone());
    mySystem.addComponent(gantryCpy);

    if(ui->_PB_flatPanelDet->isChecked())
        mySystem.addComponent(_flatPanel->clone());
    else
        mySystem.addComponent(_curvedDetector->clone());

    auto carmSys = CTL::SimpleCTsystem::fromCTsystem(mySystem);

    // create acquisition
    CTL::AcquisitionSetup acqProt(carmSys);
    acqProt.setNbViews(nbProj);
    acqProt.applyPreparationProtocol(CTL::protocols::ShortScanTrajectory(srcToIso));

    // show animation
    _avWid->setAcquisitionSetup(std::move(acqProt));
    _avWid->animateAcquisition(ANIM_FRAME_TIME);
}

void MainWindow::on__PB_helicalTraj_clicked()
{
    // get parameter
    auto srcToDet = ui->_SL_srcToDetDist->value();
    auto srcToIso = ui->_SL_srcToIsoDist->value();
    auto nbProj = ui->_SB_nbProj->value();
    auto pitch = ui->_SB_pitch->value();
    auto angleIncr = ui->_SB_nbRotations->value() * 360.0_deg / nbProj;

    // set-up system
    CTL::CTsystem mySystem;
    auto gantry = new CTL::TubularGantry(srcToDet, srcToIso, "Gantry");

    mySystem.addComponent(_source->clone());
    mySystem.addComponent(gantry);

    if(ui->_PB_flatPanelDet->isChecked())
        mySystem.addComponent(_flatPanel->clone());
    else
        mySystem.addComponent(_curvedDetector->clone());

    auto tubeSys = CTL::SimpleCTsystem::fromCTsystem(mySystem);

    // create acquisition
    CTL::AcquisitionSetup acqProt(tubeSys);
    acqProt.setNbViews(nbProj);
    acqProt.applyPreparationProtocol(CTL::protocols::HelicalTrajectory(angleIncr, pitch,
                                                                       -0.5*(pitch*nbProj)));

    // show animation
    _avWid->setAcquisitionSetup(std::move(acqProt));
    _avWid->animateAcquisition(ANIM_FRAME_TIME);
}

void MainWindow::on__PB_wobbleTraj_clicked()
{
    // get parameter
    auto srcToDet = ui->_SL_srcToDetDist->value();
    auto srcToIso = ui->_SL_srcToIsoDist->value();
    auto nbProj = ui->_SB_nbProj->value();
    auto nbWobbles = double(ui->_SB_nbWobbles->value());
    auto wobbleAmpl = ui->_SB_wobbleAmpl->value() * PI / 180.0;

    // set-up system
    CTL::CTsystem mySystem;
    auto gantryCpy = _carmGantry->clone();
    static_cast<CTL::CarmGantry*>(gantryCpy)->setCarmSpan(srcToDet);

    mySystem.addComponent(_source->clone());
    mySystem.addComponent(gantryCpy);

    if(ui->_PB_flatPanelDet->isChecked())
        mySystem.addComponent(_flatPanel->clone());
    else
        mySystem.addComponent(_curvedDetector->clone());

    auto carmSys = CTL::SimpleCTsystem::fromCTsystem(mySystem);

    // create acquisition
    CTL::AcquisitionSetup acqProt(carmSys);
    acqProt.setNbViews(nbProj);
    acqProt.applyPreparationProtocol(CTL::protocols::WobbleTrajectory(200.0_deg, srcToIso, 0.0,
                                                                      wobbleAmpl, nbWobbles));

    // show animation
    _avWid->setAcquisitionSetup(std::move(acqProt));
    _avWid->animateAcquisition(ANIM_FRAME_TIME);
}

void MainWindow::on__CB_stackAnimation_toggled(bool checked)
{
    _avWid->setAnimationStacking(checked);
}

void MainWindow::on__PB_resetCamera_clicked() { _avWid->resetCamera(); }

void MainWindow::on__CB_sourceOnly_toggled(bool checked) { _avWid->setSourceOnly(checked); }

void MainWindow::on__SL_srcToDetDist_valueChanged(int value)
{
    ui->_L_srcToDtctr->setText(QString::number(value) + QString(" mm"));
}

void MainWindow::on__SL_srcToIsoDist_valueChanged(int value)
{
    ui->_L_srcToIso->setText(QString::number(value) + QString(" mm"));
}

void MainWindow::on__PB_addVoxelVolume_clicked()
{
    QFile volFile(":/binary/voxelHead.den");
    volFile.open(QFile::ReadOnly);
    QDataStream in(&volFile);
    in.skipRawData(6);
    auto dataSize = 16 * 16 * 16;
    std::vector<uchar> volData(static_cast<size_t>(dataSize));
    in.readRawData(reinterpret_cast<char*>(volData.data()), int(dataSize));

    CTL::VoxelVolume<uchar> volume(16, 16, 16, 15.6f, 15.6f, 15.6f, std::move(volData));
    _avWid->addVolume(volume);
}

void MainWindow::on__PB_loadPmats_clicked()
{
    auto fName = QFileDialog::getOpenFileName(this,"Projection matrix file (DenFile)");

    if(fName.isEmpty())
        return;

    try {
        QSize nbPixel = { ui->_SB_pMatDetPixWidth->value(), ui->_SB_pMatDetPixHeight->value() };
        QSizeF pixelSize = { ui->_SB_pMatPixSizeWidth->value(), ui->_SB_pMatPixSizeHeight->value() };

        CTL::io::BaseTypeIO< CTL::io::DenFileIO > io;
        auto geo = io.readFullGeometry(fName, ui->_SB_nbModules->value());

        auto setup = CTL::GeometryDecoder::decodeFullGeometry(geo, nbPixel, pixelSize);

        _avWid->clearScene();
        _avWid->setAcquisitionSetup(std::move(setup));
        _avWid->animateAcquisition(ANIM_FRAME_TIME);
    } catch (const std::exception& e) {
        qCritical() << "an error has occurred while loading a file: " << e.what();
    }
}
