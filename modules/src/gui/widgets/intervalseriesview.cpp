#include "intervalseriesview.h"

#include "models/intervaldataseries.h"

#include <QAreaSeries>
#include <QLineSeries>
#include <QValueAxis>
#include <QLogValueAxis>
#include <limits>

using namespace QtCharts;

namespace CTL {
namespace gui {

IntervalSeriesView::IntervalSeriesView(QWidget* parent)
    : ChartViewBase(parent)
{
    setWindowTitle("Bar Series View");

    auto areaSeries = new QAreaSeries;
    auto areaSeriesLog = new QAreaSeries;
    areaSeries->setUpperSeries(_dataSeries);
    areaSeries->setColor(Qt::lightGray);
    areaSeries->setBorderColor(Qt::darkGray);

    areaSeriesLog->setUpperSeries(_dataSeriesLog);
    areaSeriesLog->setColor(Qt::lightGray);
    areaSeriesLog->setBorderColor(Qt::darkGray);

    _plottableSeries = areaSeries;
    _plottableSeriesLog = areaSeriesLog;

    _chart->addSeries(areaSeries);
    _chart->addSeries(areaSeriesLog);
    _chart->legend()->hide();

    mySetAxisX(new QValueAxis, areaSeries);
    mySetAxisY(new QValueAxis, areaSeries);
    mySetAxisX(new QValueAxis, areaSeriesLog);
    mySetAxisY(new QLogValueAxis, areaSeriesLog);

    setSeriesShow(_plottableSeriesLog, false);
    setOverRangeY(true);
}

void IntervalSeriesView::plot(const IntervalDataSeries& intervalSeries,
                         const QString& labelX, const QString& labelY, bool logAxisY)
{
    auto viewer = new IntervalSeriesView;
    viewer->setAttribute(Qt::WA_DeleteOnClose);

    if(logAxisY)
        viewer->switchToLogAxisY();

    viewer->setData(intervalSeries);

    viewer->setLabelX(labelX);
    viewer->setLabelY(labelY);

    viewer->resize(500,400);
    viewer->show();
}

void IntervalSeriesView::setData(const IntervalDataSeries& intervalSeries)
{
    _dataSeries->clear();
    _dataSeriesLog->clear();

    static const auto eps = 0.0001;
    const auto binWidth = intervalSeries.binWidth();
    const auto logMinVal = suitableLogMinVal(intervalSeries);

    for(const auto& pt : intervalSeries.data())
    {
        _dataSeries->append(pt.x() - (0.5 - eps) * binWidth, pt.y());
        _dataSeries->append(pt.x() + 0.5 * binWidth, pt.y());
        _dataSeries->append(pt.x() + (0.5 + eps) * binWidth, 0.0);

        const auto clampedLogVal = std::max( { pt.y(), logMinVal } );
        _dataSeriesLog->append(pt.x() - (0.5 - eps) * binWidth, clampedLogVal);
        _dataSeriesLog->append(pt.x() + 0.5 * binWidth, clampedLogVal);
        _dataSeriesLog->append(pt.x() + (0.5 + eps) * binWidth, logMinVal);
    }

    autoRange();
}

double IntervalSeriesView::suitableLogMinVal(const IntervalDataSeries& intervalSeries)
{
    static const auto bottomScale = 0.01;

    std::vector<double> tmp;
    tmp.reserve(intervalSeries.size());

    for(const auto pt : intervalSeries.data())
        if(pt.y() > 0.0)
            tmp.push_back(pt.y());

    const auto minVal = *std::min_element(tmp.cbegin(), tmp.cend());

    return std::max(bottomScale * minVal, std::numeric_limits<double>::min());
}

} // namespace gui
} // namespace CTL
