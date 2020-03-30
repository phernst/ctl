#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

namespace CTL {
class AbstractDetector;
class AbstractSource;
class AbstractGantry;
namespace gui {
class AcquisitionVisualizerWidget;
}
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

public slots:

private slots:
    // for button signals
    void on__PB_circTraj_clicked();
    void on__PB_helicalTraj_clicked();
    void on__PB_resetCamera_clicked();
    void on__PB_wobbleTraj_clicked();
    void on__PB_addVoxelVolume_clicked();
    void on__PB_loadPmats_clicked();

    // for checkboxs signals
    void on__CB_sourceOnly_toggled(bool checked);
    void on__CB_stackAnimation_toggled(bool checked);

    // for slider signals
    void on__SL_srcToDetDist_valueChanged(int value);
    void on__SL_srcToIsoDist_valueChanged(int value);

private:
    Ui::MainWindow* ui;

    CTL::gui::AcquisitionVisualizerWidget* _avWid;

    CTL::AbstractDetector* _flatPanel;
    CTL::AbstractDetector* _curvedDetector;
    CTL::AbstractSource* _source;
    CTL::AbstractGantry* _carmGantry;

    void constructDefaultComponents();
};

#endif // MAINWINDOW_H
