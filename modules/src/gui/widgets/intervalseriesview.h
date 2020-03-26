#ifndef CTL_INTERVALSERIESVIEW_H
#define CTL_INTERVALSERIESVIEW_H

#include <QtCharts>

namespace CTL {

class IntervalDataSeries;

namespace gui {

class IntervalSeriesView : public QChartView
{
    Q_OBJECT
public:
    explicit IntervalSeriesView(QWidget* parent = nullptr);

    static void plot(const IntervalDataSeries& intervalSeries,
                     const QString& labelX = "x",
                     const QString& labelY = "y");

    void setData(const IntervalDataSeries& intervalSeries);

public slots:
    void autoZoom() const;

protected:
    void mouseDoubleClickEvent(QMouseEvent *event) override;

private:
    QAreaSeries* _areaSeries;
    QLineSeries* _upper;
    QLineSeries* _lower;
    QChart* _chart;
};

} // namespace gui
} // namespace CTL

#endif // CTL_INTERVALSERIESVIEW_H
