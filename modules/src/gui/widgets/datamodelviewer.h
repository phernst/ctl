#ifndef CTL_DATAMODELVIEWER_H
#define CTL_DATAMODELVIEWER_H

#include <QWidget>
#include "models/abstractdatamodel.h"
#include "models/intervaldataseries.h"
#include "models/xydataseries.h"

namespace Ui {
class DataModelViewer;
}

class QGridLayout;

namespace CTL {
namespace gui {

class LineSeriesView;
class IntervalSeriesView;

/*!
 * \class DataModelViewer
 *
 * \brief The DataModelViewer class provides a visualization tool for data model types.
 *
 * This class can be used to visualize data models of subclasses of AbstractDataModel and
 * AbstractIntegrableDataModel. For convenience, the plot() method can be used to achieve a
 * one-line solution, creating a widget that will be destroyed once it is closed by the user.
 *
 * Data is visualized as a line plot (using LineSeriesView). In case of an integrable model, bin
 * integral data can also be visualized as a bar plot (using IntervalSeriesView). View modes can be
 * toggled using the GUI. The number of values that are sampled from the model (and visualized in
 * the viewport) can be adjusted via the GUI or using the corresponding slots setNumberOfSamples(),
 * increaseSamplingDensity(), and reduceSamplingDensity().
 * Axis labels (identical for both plot types) can be specified using setLabelX() and setLabelY() or
 * by passing the labels as arguments when using the plot() method, respectively.
 * Linear/logarithmic *y*-axis visualization can be toggled using toggleLogY() as well as via the
 * GUI.
 *
 * In case the visualized data model has parameters, these can be adjusted directly within this
 * viewer's GUI. The parameter GUI can be hidden using hideParamterGUI(), if desired. It becomes
 * hidden automatically in case the model does not have any parameters.
 * Modifying model parameters from outside the viewer will not automatically update the plot. If
 * such a setting is required, use updatePlot() to enforce redrawing of the plot. (Note that this
 * will not update the entries in the parameter GUI.)
 *
 * The following IO operations are supported by this class:
 * - Zooming:
 *    - Hold left mouse button + drag rectangle to zoom into that particular section of the plot.
 *    - Right click to zoom out.
 *    - Double-click left to request automatic zooming (ie. min/max).
 * - Save to image:
 *    - Press CRTL + S to open a dialog for saving the current figure to a file.
 *
 * The following example shows how to visualize a data model with the DataModelViewer class:
 * \code
 * // create a model of an X-ray spectrum for tube voltage 85 keV
 * auto model = std::make_shared<TASMIPSpectrumModel>(85.0);
 *
 * // (static version) using the plot() command
 * gui::DataModelViewer::plot(model);
 *
 * // (property-based version) alternatively
 * auto viewer = new gui::DataModelViewer; // needs to be deleted at an appropriate time
 * viewer->setData(model);
 * viewer->dataViewValues()->chart()->setTheme(QtCharts::QChart::ChartThemeDark);
 * viewer->resize(750,400);
 * viewer->show();
 * \endcode
 *
 * ![Resulting visualization from the example above. (a) static version, (b) property-based version.](gui/DataModelViewer.png)
 *
 * As can be seen from the example, the property-based version allows accessing the viewports, and
 * thereby, grants control over their specific settings (such as visual appearance or axis ranges).
 * The viewports are accessible via dataViewValues() and dataViewBinIntegrals() for the line and
 * bar plot view, respectively.
 *
 * The DataModelViewer can also be used conveniently to inspect compositions of multiple models. The
 * following example illustrates that for a sum of two step function models:
 * \code
 * auto step1 = std::make_shared<StepFunctionModel>(10.0, 3.0, StepFunctionModel::RightIsZero);
 * auto step2 = std::make_shared<StepFunctionModel>(25.0, 1.0, StepFunctionModel::LeftIsZero);
 *
 * // visualize
 * gui::DataModelViewer::plot(step1);
 * gui::DataModelViewer::plot(step2);
 * gui::DataModelViewer::plot(step1 + step2);
 * \endcode
 *
 * ![Resulting visualization from the composition example. (a) only model 'step1', (b)  only model 'step2', (c) sum of 'step1' and 'step2'.](gui/DataModelViewer_composition.png)
 */

class DataModelViewer : public QWidget
{
    Q_OBJECT

public:
    explicit DataModelViewer(QWidget *parent = nullptr);
    ~DataModelViewer();

    LineSeriesView* dataViewValues() const;
    IntervalSeriesView* dataViewBinIntegrals() const;
    void setData(std::shared_ptr<AbstractDataModel> model);

    static void plot(std::shared_ptr<AbstractDataModel> model,
                     const QString& labelX = "x", const QString& labelY = "y");

public slots:
    void increaseSamplingDensity();
    void hideParameterGUI(bool hide = true);
    void reduceSamplingDensity();
    void setLabelX(const QString& label);
    void setLabelY(const QString& label);
    void setNumberOfSamples(int nbSamples);
    void setSamplingRange(float from, float to);
    void toggleLogY();
    void updatePlot();

protected:
    void keyPressEvent(QKeyEvent *event) override;

private:
    LineSeriesView* _lineView;
    IntervalSeriesView* _intervalView;
    Ui::DataModelViewer *ui;

    std::shared_ptr<AbstractDataModel> _model;

    void setModelParameter(QVariant parameter);
};

namespace details {

class ParameterConfigWidget : public QWidget
{
    Q_OBJECT

public:
    ParameterConfigWidget(QWidget* parent = nullptr);

    bool isEmpty() const;

public slots:
    void updateInterface(QVariant templateParameter);

private:
    QGridLayout* _layout;

    void clearLayout();
    static QVariant parsedInputWidget(QWidget* widget);

signals:
    void parameterChanged(QVariant parameter);

private slots:
    void somethingChanged();
};

} // namespace details

} // namespace gui
} // namespace CTL

#endif // CTL_DATAMODELVIEWER_H
