#include "zoomcontrolwidget.h"
#include "ui_zoomcontrolwidget.h"

namespace CTL {
namespace gui {

ZoomControlWidget::ZoomControlWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ZoomControlWidget)
{
    ui->setupUi(this);

    connect(ui->_PB_zoomPreset1, &QPushButton::clicked, this, &ZoomControlWidget::setZoomPreset1);
    connect(ui->_PB_zoomPreset2, &QPushButton::clicked, this, &ZoomControlWidget::setZoomPreset2);
    connect(ui->_PB_zoomPreset3, &QPushButton::clicked, this, &ZoomControlWidget::setZoomPreset3);
    connect(ui->_SB_zoom, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ZoomControlWidget::zoomRequested);

    updatePresetButtonText();
}

ZoomControlWidget::~ZoomControlWidget()
{
    delete ui;
}

void ZoomControlWidget::setZoomPresets(QPair<QString, double> preset1,
                                       QPair<QString, double> preset2,
                                       QPair<QString, double> preset3)
{
    _preset1 = preset1;
    _preset2 = preset2;
    _preset3 = preset3;

    updatePresetButtonText();
}

void ZoomControlWidget::setZoomValueSilent(double zoom)
{
    ui->_SB_zoom->blockSignals(true);
    ui->_SB_zoom->setValue(zoom);
    ui->_SB_zoom->blockSignals(false);
}

void ZoomControlWidget::setZoomPreset1() { emit zoomRequested(_preset1.second); }

void ZoomControlWidget::setZoomPreset2() { emit zoomRequested(_preset2.second); }

void ZoomControlWidget::setZoomPreset3() { emit zoomRequested(_preset3.second); }

void ZoomControlWidget::updatePresetButtonText()
{
    ui->_PB_zoomPreset1->setText(_preset1.first);
    ui->_PB_zoomPreset2->setText(_preset2.first);
    ui->_PB_zoomPreset3->setText(_preset3.first);
}

} // namespace gui
} // namespace CTL
