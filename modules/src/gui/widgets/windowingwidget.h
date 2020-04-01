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

public slots:
    void setWindowDataSilent(double from, double to);

signals:
    void windowingChanged();
    void autoWindowingRequested();

private:
    Ui::WindowingWidget *ui;

    void addInvalidEffect(QWidget* reciever);
    void removeInvalidEffects();
    void blockSignalsTopBottom();
    void blockSignalsCenterWidth();
    void unblockSignalsTopBottom();
    void unblockSignalsCenterWidth();
    void updateFromToValues();
    void updateCenterWidthValues();

private slots:
    void fromChanged();
    void toChanged();
    void centerChanged();
    void widthChanged();

    bool checkFromValid();
    bool checkToValid();
};

}
}


#endif // CTL_WINDOWINGWIDGET_H
