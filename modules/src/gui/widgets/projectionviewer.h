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
    ~ProjectionViewer();

    void setData(CTL::ProjectionData projections);
    void setModuleLayout(const CTL::ModuleLayout& layout);

    int currentView() const;

public slots:
    void showView(int view);

private:
    Ui::ProjectionViewer *ui;

    CTL::ProjectionData _data = CTL::ProjectionData(0,0,0);
    CTL::ModuleLayout _modLayout;

private slots:
    void updateSliderRange();
    void windowingUpdate();
    void setZoomValueSilent(double zoom);
};

} // namespace gui
} // namespace CTL

#endif // CTL_PROJECTIONVIEWER_H
