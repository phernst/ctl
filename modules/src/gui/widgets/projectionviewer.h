#ifndef CTL_PROJECTIONVIEWER_H
#define CTL_PROJECTIONVIEWER_H

#include <QWidget>

#include "img/projectiondata.h"

namespace Ui {
class ProjectionViewer;
}

namespace CTL {
namespace gui {

class ProjectionViewer : public QWidget
{
    Q_OBJECT

public:
    explicit ProjectionViewer(QWidget *parent = nullptr);
    ProjectionViewer(ProjectionData projections, QWidget *parent = nullptr);
    ~ProjectionViewer();

    void setData(ProjectionData projections);
    void setModuleLayout(const ModuleLayout& layout);

    int currentView() const;

public slots:
    void autoResize();
    void showView(int view);

private:
    Ui::ProjectionViewer *ui;

    ProjectionData _data = ProjectionData(0,0,0);
    ModuleLayout _modLayout;

private slots:
    void updateSliderRange();
    void updatePixelInfo(int x, int y, float value);
    void windowingUpdate();
    void setZoomValueSilent(double zoom);
};

} // namespace gui
} // namespace CTL

#endif // CTL_PROJECTIONVIEWER_H
