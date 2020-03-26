#ifndef LINESERIESVIEW_H
#define LINESERIESVIEW_H

#include <QtCharts>

namespace CTL {

class XYDataSeries;

namespace gui {

class LineSeriesView : public QChartView
{
    Q_OBJECT
public:
    explicit LineSeriesView(QWidget *parent = nullptr);

    static void plot(const XYDataSeries& lineSeries,
                     const QString& labelX = "x",
                     const QString& labelY = "y");
    static void plot(const QList<QPointF>& lineSeries,
                     const QString& labelX = "x",
                     const QString& labelY = "y");

    void setData(const XYDataSeries& lineSeries);
    void setData(const QList<QPointF>& lineSeries);

public slots:
    void setShowPoints(bool enabled = true);

private:
    QLineSeries* _data;
    QChart* _chart;
};

} // namespace gui
} // namespace CTL

#endif // LINESERIESVIEW_H
