#include "lineseriesview.h"

#include "models/xydataseries.h"

namespace CTL {
namespace gui {

LineSeriesView::LineSeriesView(QWidget* parent)
    : QChartView(parent)
    , _data(new QLineSeries)
    , _chart(new QChart)
{
    setWindowTitle("Line Series View");

    setChart(_chart);
    _chart->addSeries(_data);
    _chart->legend()->hide();
    _chart->createDefaultAxes();

    setRubberBand(QChartView::RectangleRubberBand);
}

void LineSeriesView::plot(const XYDataSeries& lineSeries,
                          const QString& labelX, const QString& labelY)
{
    auto viewer = new LineSeriesView;
    viewer->setAttribute(Qt::WA_DeleteOnClose);

    viewer->setData(lineSeries);

    viewer->_chart->axisX()->setTitleText(labelX);
    viewer->_chart->axisY()->setTitleText(labelY);

    viewer->resize(500,400);
    viewer->show();
}

void LineSeriesView::plot(const QList<QPointF>& lineSeries,
                          const QString& labelX, const QString& labelY)
{
    plot(XYDataSeries(lineSeries), labelX, labelY);
}

void LineSeriesView::setData(const XYDataSeries& lineSeries)
{
    _data->clear();
    _data->append(lineSeries.data());

    const auto samplingPts = lineSeries.samplingPoints();
    QPair<float, float> xRange(*std::min_element(samplingPts.cbegin(), samplingPts.cend()),
                               *std::max_element(samplingPts.cbegin(), samplingPts.cend()));
    _chart->axisX(_data)->setRange(xRange.first, xRange.second);
    _chart->axisY(_data)->setRange(lineSeries.min(), 1.05 * lineSeries.max());
}

void LineSeriesView::setData(const QList<QPointF>& lineSeries)
{
    setData(XYDataSeries(lineSeries));
}

void LineSeriesView::setShowPoints(bool enabled)
{
    _data->setPointsVisible(enabled);
}

} // namespace gui
} // namespace CTL
