#include "windowingwidget.h"
#include "ui_windowingwidget.h"

#include <QGraphicsColorizeEffect>

WindowingWidget::WindowingWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WindowingWidget)
{
    ui->setupUi(this);

    connect(ui->_SB_windowBottom, SIGNAL(valueChanged(double)), SLOT(fromChanged()));
    connect(ui->_SB_windowTop, SIGNAL(valueChanged(double)), SLOT(toChanged()));
    connect(ui->_SB_windowCenter, SIGNAL(valueChanged(double)), SLOT(centerChanged()));
    connect(ui->_SB_windowWidth, SIGNAL(valueChanged(double)), SLOT(widthChanged()));

    connect(ui->_PB_autoWindow, SIGNAL(clicked(bool)), SIGNAL(autoWindowingRequested()));
}

WindowingWidget::~WindowingWidget()
{
    delete ui;
}

QPair<double, double> WindowingWidget::windowFromTo() const
{
    double bottom = ui->_SB_windowBottom->value();
    double top = ui->_SB_windowTop->value();

    return qMakePair(bottom, top);
}

QPair<double, double> WindowingWidget::windowCenterWidth() const
{
    double center = ui->_SB_windowCenter->value();
    double width  = ui->_SB_windowWidth->value();

    return qMakePair(center, width);
}

void WindowingWidget::setWindowFromTo(const QPair<double, double> &window)
{
    ui->_SB_windowBottom->blockSignals(true);
    ui->_SB_windowTop->blockSignals(true);
    ui->_SB_windowBottom->setValue(window.first);
    ui->_SB_windowTop->setValue(window.second);
    ui->_SB_windowBottom->blockSignals(false);
    ui->_SB_windowTop->blockSignals(false);

    checkFromValid();
    checkToValid();

    updateCenterWidth();
}

void WindowingWidget::setWindowCenterWidth(const QPair<double, double> &window)
{
    ui->_SB_windowCenter->blockSignals(true);
    ui->_SB_windowWidth->blockSignals(true);
    ui->_SB_windowCenter->setValue(window.first);
    ui->_SB_windowWidth->setValue(window.second);
    ui->_SB_windowCenter->blockSignals(false);
    ui->_SB_windowWidth->blockSignals(false);

    updateFromTo();
}

void WindowingWidget::updateFromTo()
{
    double center = ui->_SB_windowCenter->value();
    double width  = ui->_SB_windowWidth->value();

    ui->_SB_windowBottom->blockSignals(true);
    ui->_SB_windowTop->blockSignals(true);
    ui->_SB_windowBottom->setValue(center - width/2.0);
    ui->_SB_windowTop->setValue(center + width/2.0);
    ui->_SB_windowBottom->blockSignals(false);
    ui->_SB_windowTop->blockSignals(false);

    checkFromValid();
    checkToValid();

    emit windowingChanged();
}

void WindowingWidget::updateCenterWidth()
{
    double bottom = ui->_SB_windowBottom->value();
    double top = ui->_SB_windowTop->value();

    ui->_SB_windowCenter->blockSignals(true);
    ui->_SB_windowWidth->blockSignals(true);
    ui->_SB_windowCenter->setValue((top+bottom)/2.0);
    ui->_SB_windowWidth->setValue(top-bottom);
    ui->_SB_windowCenter->blockSignals(false);
    ui->_SB_windowWidth->blockSignals(false);

    if(ui->_SB_windowBottom->graphicsEffect() || ui->_SB_windowTop->graphicsEffect())
        return;

    emit windowingChanged();
}

void WindowingWidget::fromChanged()
{
    checkFromValid();

    updateCenterWidth();
}

void WindowingWidget::toChanged()
{
    checkToValid();

    updateCenterWidth();
}

void WindowingWidget::centerChanged()
{
    updateFromTo();
}

void WindowingWidget::widthChanged()
{
    updateFromTo();
}

void WindowingWidget::checkFromValid()
{
    auto curFrom = ui->_SB_windowBottom->value();
    auto curTo   = ui->_SB_windowTop->value();

    if(curFrom > curTo)
    {
        auto colorize = new QGraphicsColorizeEffect;
        colorize->setColor(Qt::red);
        ui->_SB_windowBottom->setGraphicsEffect(colorize);
    }
    else
    {
        ui->_SB_windowBottom->setGraphicsEffect(nullptr);
        ui->_SB_windowTop->setGraphicsEffect(nullptr);
    }
}

void WindowingWidget::checkToValid()
{
    auto curFrom = ui->_SB_windowBottom->value();
    auto curTo   = ui->_SB_windowTop->value();

    if(curTo < curFrom)
    {
        auto colorize = new QGraphicsColorizeEffect;
        colorize->setColor(Qt::red);
        ui->_SB_windowTop->setGraphicsEffect(colorize);
    }
    else
    {
        ui->_SB_windowBottom->setGraphicsEffect(nullptr);
        ui->_SB_windowTop->setGraphicsEffect(nullptr);
    }
}
