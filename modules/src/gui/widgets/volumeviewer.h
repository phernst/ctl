#ifndef CTL_VOLUMEVIEWER_H
#define CTL_VOLUMEVIEWER_H

#include <QWidget>
#include "img/compositevolume.h"

namespace Ui {
class VolumeViewer;
}

namespace CTL {
namespace gui {

class VolumeViewer : public QWidget
{
    Q_OBJECT

public:
    explicit VolumeViewer(QWidget *parent = nullptr);
    VolumeViewer(CompositeVolume volume, QWidget *parent = nullptr);
    ~VolumeViewer();

    void setData(SpectralVolumeData data);
    void setData(CompositeVolume data);

    static void plot(CompositeVolume data);
    static void plot(SpectralVolumeData data);

public slots:
    void autoResize();
    void hideCompositeOverview(bool hide = true);
    void setAutoMouseWindowScaling();
    void showSlice(int slice);

protected:
    void keyPressEvent(QKeyEvent *event) override;

private:
    Ui::VolumeViewer *ui;

    CompositeVolume _compData;

    const SpectralVolumeData& selectedVolume() const;

private slots:
    void sliceDirectionChanged();
    void updateVolumeOverview();
    void updateSliderRange();
    void updatePixelInfo(int x, int y, float value);
    void volumeSelectionChanged();
    void windowingUpdate();
};

} // namespace gui
} // namespace CTL

#endif // CTL_VOLUMEVIEWER_H
