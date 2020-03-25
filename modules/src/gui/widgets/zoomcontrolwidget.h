#ifndef CTL_ZOOMCONTROLWIDGET_H
#define CTL_ZOOMCONTROLWIDGET_H

#include <QWidget>

namespace Ui {
class ZoomControlWidget;
}

namespace CTL {
namespace gui {

class ZoomControlWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ZoomControlWidget(QWidget *parent = nullptr);
    ~ZoomControlWidget();

    void setZoomPresets(QPair<QString, double> preset1,
                        QPair<QString, double> preset2,
                        QPair<QString, double> preset3);

public slots:
    void setZoomValueSilent(double zoom);

signals:
    void zoomRequested(double zoom);

private:
    Ui::ZoomControlWidget *ui;

    QPair<QString, double> _preset1 = { QStringLiteral("0.5x"), 0.5 };
    QPair<QString, double> _preset2 = { QStringLiteral("1.0x"), 1.0 };
    QPair<QString, double> _preset3 = { QStringLiteral("2.0x"), 2.0 };

private slots:
    void setZoomPreset1();
    void setZoomPreset2();
    void setZoomPreset3();
    void updatePresetButtonText();
};

} // namespace gui
} // namespace CTL

#endif // CTL_ZOOMCONTROLWIDGET_H
