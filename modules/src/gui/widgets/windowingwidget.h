#ifndef CTL_WINDOWINGWIDGET_H
#define CTL_WINDOWINGWIDGET_H

#include <QWidget>

namespace Ui {
class WindowingWidget;
}

namespace CTL {
namespace gui {

class WindowingWidget : public QWidget
{
    Q_OBJECT

public:
    explicit WindowingWidget(QWidget *parent = nullptr);
    ~WindowingWidget();

    QPair<double, double> windowFromTo() const;
    QPair<double, double> windowCenterWidth() const;

    void setWindowFromTo(const QPair<double, double>& window);
    void setWindowCenterWidth(const QPair<double, double> &window);
    void setPresets(QPair<QString, QPair<double, double>> preset1,
                    QPair<QString, QPair<double, double>> preset2);

public slots:
    void setWindowDataSilent(double from, double to);

signals:
    void windowingChanged();
    void autoWindowingRequested();

private:
    Ui::WindowingWidget *ui;

    QPair<QString, QPair<double, double>> _preset1 = { QStringLiteral("Preset 1"), qMakePair(0.0, 1.0) };
    QPair<QString, QPair<double, double>> _preset2 = { QStringLiteral("Preset 2"), qMakePair(-1.0, 1.0) };

    void addInvalidEffect(QWidget* reciever);
    void removeInvalidEffects();
    void blockSignalsTopBottom();
    void blockSignalsCenterWidth();
    void unblockSignalsTopBottom();
    void unblockSignalsCenterWidth();
    void updateFromToValues();
    void updateCenterWidthValues();

private slots:
    void centerChanged();
    bool checkFromValid();
    bool checkToValid();
    void fromChanged();
    void setPreset1();
    void setPreset2();
    void toChanged();
    void updatePresetButtonText();
    void widthChanged();

};

}
}


#endif // CTL_WINDOWINGWIDGET_H
