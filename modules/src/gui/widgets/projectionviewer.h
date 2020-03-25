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

    static void plot(ProjectionData projections, const ModuleLayout& layout = {});

    void setData(ProjectionData projections);
    void setModuleLayout(const ModuleLayout& layout);

    int currentView() const;

public slots:
    void autoResize();
    void showView(int view);

protected:
    void keyPressEvent(QKeyEvent *event) override;

private:
    Ui::ProjectionViewer *ui;

    ProjectionData _data = ProjectionData(0,0,0);
    ModuleLayout _modLayout;

private slots:
    void updateSliderRange();
    void updatePixelInfo(int x, int y, float value);
    void windowingUpdate();
};

} // namespace gui
} // namespace CTL

#endif // CTL_PROJECTIONVIEWER_H
