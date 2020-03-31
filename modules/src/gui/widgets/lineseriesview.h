#ifndef LINESERIESVIEW_H
#define LINESERIESVIEW_H

#include "chartviewbase.h"

namespace CTL {

class XYDataSeries;

namespace gui {

class LineSeriesView : public ChartViewBase
{
    Q_OBJECT
public:
    explicit LineSeriesView(QWidget *parent = nullptr);

    static void plot(const XYDataSeries& lineSeries,
                     const QString& labelX = "x",
                     const QString& labelY = "y",
                     bool logAxisY = false);
    static void plot(const QList<QPointF>& lineSeries,
                     const QString& labelX = "x",
                     const QString& labelY = "y",
                     bool logAxisY = false);

    void setData(const XYDataSeries& lineSeries);
    void setData(const QList<QPointF>& lineSeries);

    QImage image(const QSize& renderSize = QSize());

public slots:
    void setShowPoints(bool enabled = true);

private:
    void updateLogData();
};

} // namespace gui
} // namespace CTL

#endif // LINESERIESVIEW_H
