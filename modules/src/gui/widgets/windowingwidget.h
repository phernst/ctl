#ifndef CTL_WINDOWINGWIDGET_H
#define CTL_WINDOWINGWIDGET_H

#include <QWidget>

namespace Ui {
class WindowingWidget;
}

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

signals:
    void windowingChanged();
    void autoWindowingRequested();

private:
    Ui::WindowingWidget *ui;

    void updateFromTo();
    void updateCenterWidth();

private slots:
    void fromChanged();
    void toChanged();
    void centerChanged();
    void widthChanged();

    void checkFromValid();
    void checkToValid();
};

#endif // CTL_WINDOWINGWIDGET_H
