#ifndef CTL_VOLUMEVIEWER_H
#define CTL_VOLUMEVIEWER_H

#include <QWidget>
#include "img/compositevolume.h"

namespace Ui {
class VolumeViewer;
}

namespace CTL {
namespace gui {

class Chunk2DView;

class VolumeViewer : public QWidget
{
    Q_OBJECT

public:
    enum class WindowPreset {
        Abdomen,
        Angio,
        Bone,
        Brain,
        Chest,
        Lungs
    };

    explicit VolumeViewer(QWidget *parent = nullptr);
    VolumeViewer(CompositeVolume volume, QWidget *parent = nullptr);
    ~VolumeViewer();

    static void plot(CompositeVolume data);
    static void plot(SpectralVolumeData data);

    const CompositeVolume& data() const;
    Chunk2DView* dataView() const;
    void setData(SpectralVolumeData data);
    void setData(CompositeVolume data);
    void setWindowPresets(WindowPreset preset1, WindowPreset preset2);
    void setWindowPresetsInMu(WindowPreset preset1, WindowPreset preset2, float referenceEnergy);
    void setWindowPresets(QPair<QString, QPair<double, double>> preset1,
                          QPair<QString, QPair<double, double>> preset2);

public slots:
    void autoResize();
    void hideCompositeOverview(bool hide = true);
    void setAutoMouseWindowScaling();
    void showSlice(int slice);
    void showSubvolume(int subvolume);

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
