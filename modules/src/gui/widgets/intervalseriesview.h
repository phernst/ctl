#ifndef CTL_INTERVALSERIESVIEW_H
#define CTL_INTERVALSERIESVIEW_H

#include "chartviewbase.h"

namespace CTL {

class IntervalDataSeries;

namespace gui {

/*!
 * \class IntervalSeriesView
 *
 * \brief The IntervalSeriesView class provides basic visualization of IntervalDataSeries data.
 *
 * This class can be used to visualize data stored in an IntervalDataSeries. For convenience, the
 * plot() method can be used to achieve a one-line solution, creating a widget that will be
 * destroyed once it is closed by the user.
 *
 * Data will be visualized as a bar plot. Each bar represents the bin integral value stored in the
 * corresponding bin of the IntervalDataSeries that is visualized. Axis labels can be specified
 * using setLabelX() and setLabelY() or by passing the labels as arguments when using the plot()
 * method, respectively. Logarithmic *y*-axis visualization can be enabled using setLogAxisY(true),
 * or by passing the corresponding flag as last argument of plot().
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
 * The following example shows how to visualize data sampled from a data model:
 * \code
 * // create a model of an X-ray spectrum for tube voltage 85 keV
 * auto model = TASMIPSpectrumModel(85.0);
 *
 * // sample 100 energy bins in the range [0, 100] keV
 * auto samples = IntervalDataSeries::sampledFromModel(model, 0.0, 100.0, 100);
 *
 * // (static version) using the plot() command
 * gui::IntervalSeriesView::plot(samples, "Energy [keV]", "Flux [photons/(mm^2*mAs)]");
 *
 * // (property-based version) alternatively
 * auto viewer = new gui::IntervalSeriesView;                   // needs to be deleted at an appropriate time
 * viewer->setData(samples);
 * viewer->setRangeX(10.0, 85.0);                               // visualize specific range
 * viewer->chart()->setTheme(QtCharts::QChart::ChartThemeDark); // change visual appearance of the chart
 * viewer->resize(600,400);
 * viewer->show();
 * \endcode
 *
 * ![Resulting visualization from the example above. (a) static version, (b) property-based version.](gui/IntervalSeriesView.png)
 */

class IntervalSeriesView : public ChartViewBase
{
    Q_OBJECT
public:
    explicit IntervalSeriesView(QWidget* parent = nullptr);

    static void plot(const IntervalDataSeries& intervalSeries,
                     const QString& labelX = "x",
                     const QString& labelY = "y",
                     bool logAxisY = false);

    void setData(const IntervalDataSeries& intervalSeries);

private:
    static constexpr auto BAR_GAP = 0.0001;

    void copyDataToClipboard() const override;
    double suitableLogMinVal(const IntervalDataSeries& intervalSeries);
};

} // namespace gui
} // namespace CTL

#endif // CTL_INTERVALSERIESVIEW_H
