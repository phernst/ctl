#include "intervalseriesview.h"

#include "models/intervaldataseries.h"

#include <QAreaSeries>
#include <QLineSeries>
#include <QValueAxis>
#include <QLogValueAxis>
#include <QApplication>
#include <QClipboard>
#include <limits>

using namespace QtCharts;

namespace CTL {
namespace gui {

/*!
 * Creates an IntervalSeriesView and sets its parent to \a parent.
 *
 * By default, this class uses "Y axis over ranging" (see setOverRangeY()).
 */
IntervalSeriesView::IntervalSeriesView(QWidget* parent)
    : ChartViewBase(parent)
{
    setWindowTitle("Interval Series View");

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

/*!
 * Creates an IntervalSeriesView for \a intervalSeries and shows the window.
 *
 * Labels of the axes can be specified by \a labelX and \a labelY. If left empty, default axis
 * labels are "x" and "y". To create a plot with a logarithmic *y*-axis, pass \c true for
 * \a logAxisY.
 *
 * The widget will be deleted automatically if the window is closed.
 *
 * Example: visualize spectrum of an X-ray tube with 120 keV tube voltage
 * \code
 * XrayTube tube;
 * tube.setTubeVoltage(120.0f);
 *
 * gui::IntervalSeriesView::plot(tube.spectrum(100), "Energy [keV]", " Relative flux");
 * \endcode
 *
 * ![Resulting visualization from the example above.](gui/IntervalSeriesView_plot.png)
 */
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

/*!
 * Sets the series visualized by this instance to \a intervalSeries.
 *
 * Applies a min/max range (see autoRange()). By default, this class uses "Y axis over ranging"
 * (see setOverRangeY()).
 */
void IntervalSeriesView::setData(const IntervalDataSeries& intervalSeries)
{
    _dataSeries->clear();
    _dataSeriesLog->clear();

    const auto binWidth = intervalSeries.binWidth();
    const auto logMinVal = suitableLogMinVal(intervalSeries);

    for(const auto& pt : intervalSeries.data())
    {
        _dataSeries->append(pt.x() - (0.5 - BAR_GAP) * binWidth, pt.y());
        _dataSeries->append(pt.x() + 0.5 * binWidth, pt.y());
        _dataSeries->append(pt.x() + (0.5 + BAR_GAP) * binWidth, 0.0);

        const auto clampedLogVal = std::max( { pt.y(), logMinVal } );
        _dataSeriesLog->append(pt.x() - (0.5 - BAR_GAP) * binWidth, clampedLogVal);
        _dataSeriesLog->append(pt.x() + 0.5 * binWidth, clampedLogVal);
        _dataSeriesLog->append(pt.x() + (0.5 + BAR_GAP) * binWidth, logMinVal);
    }

    autoRange();
}

void IntervalSeriesView::copyDataToClipboard() const
{
    const auto dataPts = yAxisIsLinear() ? _dataSeries->pointsVector()
                                         : _dataSeriesLog->pointsVector();

    QStringList list;
    for(auto dataIt = dataPts.cbegin(); dataIt < dataPts.cend(); dataIt += 3)
    {
        const auto binWidth = ((dataIt+1)->x() - dataIt->x()) / (1.0 - BAR_GAP);

        const auto x = (dataIt+1)->x() - 0.5 * binWidth;
        const auto y = dataIt->y();
        list << QString::number(x) + QStringLiteral(" ") + QString::number(y);
    }

    QApplication::clipboard()->setText(list.join("\n"));
}

/*!
 * Finds a suitable lower end point for the bars in logarithmic scale plot mode. This returns a
 * value of 0.01 times the smallest positive value occuring in \a intervalSeries (bound to a
 * minimum value of std::numeric_limits<double>::min()).
 *
 * This leads to a visualization in which the lower end of all bars is two decades below the minimal
 * occuring upper end of all bars.
 */
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
