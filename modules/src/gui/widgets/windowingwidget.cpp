#include "windowingwidget.h"
#include "ui_windowingwidget.h"

#include <QGraphicsColorizeEffect>

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
    connect(ui->_PB_preset1, &QPushButton::clicked, this, &WindowingWidget::setPreset1);
    connect(ui->_PB_preset2, &QPushButton::clicked, this, &WindowingWidget::setPreset2);

    updatePresetButtonText();
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

void WindowingWidget::setPresets(QPair<QString, QPair<double, double>> preset1,
                                 QPair<QString, QPair<double, double>> preset2)
{
    _preset1 = preset1;
    _preset2 = preset2;

    updatePresetButtonText();
}

void WindowingWidget::updatePresetButtonText()
{
    auto toolTipText = [] (const QPair<double, double>& window)
    {
        return QStringLiteral("(") + QString::number(window.first) + QStringLiteral(",")
                + QString::number(window.second) + QStringLiteral(")");
    };

    ui->_PB_preset1->setText(_preset1.first);
    ui->_PB_preset2->setText(_preset2.first);

    ui->_PB_preset1->setToolTip(toolTipText(_preset1.second));
    ui->_PB_preset2->setToolTip(toolTipText(_preset2.second));
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

void WindowingWidget::setPreset1()
{
    setWindowFromTo(_preset1.second);
}

void WindowingWidget::setPreset2()
{
    setWindowFromTo(_preset2.second);
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
