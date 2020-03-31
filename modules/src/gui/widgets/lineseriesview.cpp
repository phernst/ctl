#include "lineseriesview.h"

#include "models/xydataseries.h"

#include <QLineSeries>
#include <QValueAxis>
#include <QLogValueAxis>

using namespace QtCharts;

namespace CTL {
namespace gui {

LineSeriesView::LineSeriesView(QWidget* parent)
    : ChartViewBase(parent)
{
    setWindowTitle("Line Series View");

    _plottableSeries = _dataSeries;
    _plottableSeriesLog = _dataSeriesLog;
    _chart->addSeries(_plottableSeries);
    _chart->addSeries(_plottableSeriesLog);

    mySetAxisX(new QValueAxis, _dataSeries);
    mySetAxisY(new QValueAxis, _dataSeries);
    mySetAxisX(new QValueAxis, _dataSeriesLog);
    mySetAxisY(new QLogValueAxis, _dataSeriesLog);

    setSeriesShow(_plottableSeriesLog, false);
}

void LineSeriesView::plot(const XYDataSeries& lineSeries,
                          const QString& labelX, const QString& labelY, bool logAxisY)
{
    auto viewer = new LineSeriesView;
    viewer->setAttribute(Qt::WA_DeleteOnClose);

    if(logAxisY)
        viewer->switchToLogAxisY();

    viewer->setData(lineSeries);

    viewer->setLabelX(labelX);
    viewer->setLabelY(labelY);

    viewer->resize(500,400);
    viewer->show();
}

void LineSeriesView::plot(const QList<QPointF>& lineSeries,
                          const QString& labelX, const QString& labelY, bool logAxisY)
{
    plot(XYDataSeries(lineSeries), labelX, labelY, logAxisY);
}

void LineSeriesView::setData(const XYDataSeries& lineSeries)
{
    _dataSeries->clear();
    _dataSeries->append(lineSeries.data());

    if(!yAxisIsLinear())
        updateLogData();

    autoRange();
}

void LineSeriesView::setData(const QList<QPointF>& lineSeries)
{
    setData(XYDataSeries(lineSeries));
}

void LineSeriesView::setShowPoints(bool enabled)
{
    _dataSeries->setPointsVisible(enabled);
    _dataSeriesLog->setPointsVisible(enabled);
}

void LineSeriesView::updateLogData()
{
    _dataSeriesLog->clear();
    for(const auto& pt : _dataSeries->pointsVector())
        if(pt.y() > 0.0)
            _dataSeriesLog->append(pt);
}


} // namespace gui
} // namespace CTL
