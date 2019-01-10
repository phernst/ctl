#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "acquisition/acquisitionsetup.h"
#include <img/projectiondata.h>
#include <img/voxelvolume.h>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:

    void showProjectionDone(int);
    void updateGeometryPreview();
    void updatePitchFactor();
    void on__PB_loadPhantom_clicked();
    void on__PB_fitToVolumeSize_clicked();
    void on__PB_centerTrajectory_clicked();
    void on__SB_nbViews_valueChanged(int nbViews);
    void on__PB_simulateScan_clicked();
    void on__PB_saveProj_clicked();

private:
    Ui::MainWindow *ui;

    CTL::SimpleCTsystem _CTSystem;
    
    CTL::VoxelVolume<float> _volumeData = CTL::VoxelVolume<float>(0,0,0);
    CTL::ProjectionData _projectionData = CTL::ProjectionData(0,0,0);

    void loadDenFile(const QString& fileName);
    void saveDenFile(const QString& fileName);
    void setDimensionLabelText(uint x, uint y, uint z);
    template<typename T>
    void setVolumeData(const CTL::VoxelVolume<T>& data);
    CTL::AcquisitionSetup currentSetup() const;
};

template<typename T>
void MainWindow::setVolumeData(const CTL::VoxelVolume<T> &data)
{
    _volumeData = CTL::VoxelVolume<float>(data.nbVoxels().x,
                                        data.nbVoxels().y,
                                        data.nbVoxels().z);

    _volumeData.allocateMemory();
    auto dataPtr = _volumeData.rawData();
    auto srcPtr  = data.constData();

    for(size_t vox=0; vox < data.totalVoxelCount(); ++vox)
       dataPtr[vox] = static_cast<float>(srcPtr[vox]);
}
#endif // MAINWINDOW_H
