#ifndef CTL_LINESERIESVIEW_H
#define CTL_LINESERIESVIEW_H

#include "chartviewbase.h"

namespace CTL {

class XYDataSeries;

namespace gui {

/*!
 * \class LineSeriesView
 *
 * \brief The LineSeriesView class provides basic visualization of XYDataSeries data.
 *
 * This class can be used to visualize data stored in an XYDataSeries. For convenience, the plot()
 * method can be used to achieve a one-line solution, creating a widget that will be destroyed once
 * it is closed by the user.
 *
 * Data will be visualized as a line plot. Individual data points are not shown by default.
 * Use setShowPoints() to enable this. Axis labels can be specified using setLabelX() and
 * setLabelY() or by passing the labels as arguments when using the plot() method, respectively.
 * Logarithmic *y*-axis visualization can be enabled using setLogAxisY(true), or by passing the
 * corresponding flag as last argument of plot().
 *
 * The following IO operations are supported by this class:
 * - Zooming:
 *    - Hold left mouse button + drag rectangle to zoom into that particular section of the plot.
 *    - Right click to zoom out.
 *    - Double-click left to request automatic zooming (ie. min/max).
 * - Copying data to clipboard:
 *    - Press CTRL + C to copy the values in the current plot to the clipboard. Each data point
 * will be on a separate line with x and y value separated by a single whitespace.
 * - Save to image:
 *    - Press CTRL + S to open a dialog for saving the current figure to a file.
 *
 * Axis ranges (both x and y) can also be defined explicitely using setRangeX() and setRangeY().
 *
 * The following example shows how to visualize some random data points using LineSeriesView:
 * \code
 * XYDataSeries data;
 *
 * // generate some data points with random y-values in [0,1)
 * std::random_device rd;
 * std::mt19937 rng(rd());
 * std::uniform_real_distribution<float> dis(0.0, 1.0);
 * for(int i = 0; i < 101; ++i)
 *     data.append(float(i), dis(rng));
 *
 * // (static version) using the plot() command
 * gui::LineSeriesView::plot(data, "Index", "Random value [0,1)");
 *
 * // (property-based version) alternatively
 * auto viewer = new gui::LineSeriesView; // needs to be deleted at an appropriate time
 * viewer->setData(data);
 * viewer->setShowPoints();               // turns on display of data points
 * viewer->setLogAxisY(true);             // turns on logarithmic y-axis scaling
 * viewer->resize(600,400);
 * viewer->show();
 * \endcode
 *
 * ![Resulting visualization from the example above. (a) static version, (b) property-based version.](gui/LineSeriesView.png)
 */

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

public slots:
    void setShowPoints(bool enabled = true);

};

} // namespace gui
} // namespace CTL

#endif // CTL_LINESERIESVIEW_H
