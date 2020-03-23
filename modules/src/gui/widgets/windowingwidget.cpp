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

namespace CTL {
namespace gui {

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

void WindowingWidget::setWindowFromTo(const QPair<double, double>& window)
{
    setWindowDataSilent(window.first, window.second);

    emit windowingChanged();
}

void WindowingWidget::setWindowCenterWidth(const QPair<double, double>& window)
{
    setWindowDataSilent(window.second - window.first,
                        window.first + 0.5 * (window.second - window.first));

    emit windowingChanged();
}

void WindowingWidget::setWindowDataSilent(double from, double to)
{
    // block all signals (silent change)
    blockSignalsTopBottom();
    blockSignalsCenterWidth();

    ui->_SB_windowBottom->setValue(from);
    ui->_SB_windowTop->setValue(to);
    ui->_SB_windowCenter->setValue(from + 0.5 * (to - from));
    ui->_SB_windowWidth->setValue(to - from);

    // restore all signals
    unblockSignalsTopBottom();
    unblockSignalsCenterWidth();

    checkFromValid();
    checkToValid();
}

void WindowingWidget::addInvalidEffect(QWidget *reciever)
{
    auto colorize = new QGraphicsColorizeEffect;
    colorize->setColor(Qt::red);
    reciever->setGraphicsEffect(colorize);
}

void WindowingWidget::removeInvalidEffects()
{
    ui->_SB_windowBottom->setGraphicsEffect(nullptr);
    ui->_SB_windowTop->setGraphicsEffect(nullptr);
}

void WindowingWidget::blockSignalsTopBottom()
{
    ui->_SB_windowBottom->blockSignals(true);
    ui->_SB_windowTop->blockSignals(true);
}

void WindowingWidget::blockSignalsCenterWidth()
{
    ui->_SB_windowCenter->blockSignals(true);
    ui->_SB_windowWidth->blockSignals(true);
}

void WindowingWidget::unblockSignalsTopBottom()
{
    ui->_SB_windowBottom->blockSignals(false);
    ui->_SB_windowTop->blockSignals(false);
}

void WindowingWidget::unblockSignalsCenterWidth()
{
    ui->_SB_windowCenter->blockSignals(false);
    ui->_SB_windowWidth->blockSignals(false);
}


void WindowingWidget::updateFromToValues()
{
    double center = ui->_SB_windowCenter->value();
    double width  = ui->_SB_windowWidth->value();

    blockSignalsTopBottom();
    ui->_SB_windowBottom->setValue(center - width/2.0);
    ui->_SB_windowTop->setValue(center + width/2.0);
    unblockSignalsTopBottom();
}

void WindowingWidget::updateCenterWidthValues()
{
    double bottom = ui->_SB_windowBottom->value();
    double top    = ui->_SB_windowTop->value();

    blockSignalsCenterWidth();
    ui->_SB_windowCenter->setValue((top+bottom)/2.0);
    ui->_SB_windowWidth->setValue(top-bottom);
    unblockSignalsCenterWidth();
}

void WindowingWidget::fromChanged()
{
    updateCenterWidthValues();

    if(checkFromValid())
        emit windowingChanged();
}

void WindowingWidget::toChanged()
{
    updateCenterWidthValues();

    if(checkToValid())
        emit windowingChanged();
}

void WindowingWidget::centerChanged()
{
    updateFromToValues();
    emit windowingChanged();
}

void WindowingWidget::widthChanged()
{
    updateFromToValues();
    emit windowingChanged();
}

bool WindowingWidget::checkFromValid()
{
    const auto curFrom = ui->_SB_windowBottom->value();
    const auto curTo   = ui->_SB_windowTop->value();

    if(curFrom > curTo)
    {
        addInvalidEffect(ui->_SB_windowBottom);

        return false;
    }
    else
        removeInvalidEffects();

    return true;
}

bool WindowingWidget::checkToValid()
{
    const auto curFrom = ui->_SB_windowBottom->value();
    const auto curTo   = ui->_SB_windowTop->value();

    if(curTo < curFrom)
    {
        addInvalidEffect(ui->_SB_windowTop);

        return false;
    }
    else
        removeInvalidEffects();

     return true;
}

}
}
