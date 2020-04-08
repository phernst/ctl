#include "lineseriesview.h"

#include "models/xydataseries.h"

#include <QLineSeries>
#include <QValueAxis>
#include <QLogValueAxis>

using namespace QtCharts;

namespace CTL {
namespace gui {

/*!
 * Creates a LineSeriesView and sets its parent to \a parent.
 */
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

/*!
 * Creates a LineSeriesView for \a lineSeries and shows the window.
 *
 * Labels of the axes can be specified by \a labelX and \a labelY. If left empty, default axis
 * labels are "x" and "y". To create a plot with a logarithmic *y*-axis, pass \c true for
 * \a logAxisY.
 *
 * The widget will be deleted automatically if the window is closed.
 *
 * Example:
 * \code
 * // get the attenuation model of lead from the database
 * auto attModel = attenuationModel(database::Element::Pb);
 *
 * // sample 100 values in the energy range [10, 120] keV
 * auto sampledValues = XYDataSeries::sampledFromModel(attModel, 10.0, 120.0, 100);
 *
 * // plot sampled values in logarithmic scale
 * gui::LineSeriesView::plot(sampledValues, "Energy [keV]", "Mass atten. coeff. [cm^2/g]", true);
 * \endcode
 *
 * ![Resulting visualization from the example above.](gui/LineSeriesView_plot.png)
 */
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

/*!
 * Convenience overload. See plot(const XYDataSeries&, const QString&, const QString&, bool).
 */
void LineSeriesView::plot(const QList<QPointF>& lineSeries,
                          const QString& labelX, const QString& labelY, bool logAxisY)
{
    plot(XYDataSeries(lineSeries), labelX, labelY, logAxisY);
}

void LineSeriesView::setData(const XYDataSeries& lineSeries)
{
    _dataSeries->clear();
    _dataSeries->append(lineSeries.data());

    _dataSeriesLog->clear();
    for(const auto& pt : _dataSeries->pointsVector())
        if(pt.y() > 0.0)
            _dataSeriesLog->append(pt);

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

} // namespace gui
} // namespace CTL
