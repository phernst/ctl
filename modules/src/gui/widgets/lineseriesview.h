#ifndef LINESERIESVIEW_H
#define LINESERIESVIEW_H

#include <QtCharts>

namespace CTL {
namespace gui {

class LineSeriesView : public QChartView
{
    Q_OBJECT
public:
    explicit LineSeriesView(QWidget *parent = nullptr);

    static void plot(const QList<QPointF>& lineSeries,
                     const QString& labelX = "x",
                     const QString& labelY = "y");

};

} // namespace gui
} // namespace CTL

#endif // LINESERIESVIEW_H
