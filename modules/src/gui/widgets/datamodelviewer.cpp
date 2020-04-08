#include "datamodelviewer.h"
#include "ui_datamodelviewer.h"

#include "processing/coordinates.h"
#include "lineseriesview.h"
#include "intervalseriesview.h"

#include <qmath.h>

namespace CTL {
namespace gui {

/*!
 * Creates a DataModelViewer and sets its parent to \a parent.
 */
DataModelViewer::DataModelViewer(QWidget *parent)
    : QWidget(parent)
    , _lineView(new LineSeriesView)
    , _intervalView(new IntervalSeriesView)
    , ui(new Ui::DataModelViewer)
{
    ui->setupUi(this);

    _lineView->setShowPoints();
    ui->_stackedWidget->addWidget(_lineView);
    ui->_stackedWidget->addWidget(_intervalView);
    ui->_stackedWidget->setCurrentWidget(_lineView);

    connect(ui->_RB_values, &QRadioButton::toggled, this, &DataModelViewer::updatePlot);
    connect(ui->_SB_rangeFrom, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &DataModelViewer::updatePlot);
    connect(ui->_SB_rangeTo, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &DataModelViewer::updatePlot);
    connect(ui->_PB_reduceSampling, &QPushButton::clicked, this, &DataModelViewer::reduceSamplingDensity);
    connect(ui->_PB_increaseSampling, &QPushButton::clicked, this, &DataModelViewer::increaseSamplingDensity);
    connect(ui->_SB_nbSamples, QOverload<int>::of(&QSpinBox::valueChanged), this, &DataModelViewer::updatePlot);
    connect(ui->_W_parameterEditor, &details::ParameterConfigWidget::parameterChanged, this, &DataModelViewer::setModelParameter);
    connect(ui->_PB_linLogY, &QPushButton::clicked, this, &DataModelViewer::toggleLogY);
    connect(ui->_CB_niceX, &QCheckBox::toggled, _lineView, &LineSeriesView::setUseNiceX);
    connect(ui->_CB_niceX, &QCheckBox::toggled, _intervalView, &IntervalSeriesView::setUseNiceX);

    setWindowTitle("Data Model Viewer");
}

/*!
 * Deletes this instance.
 */
DataModelViewer::~DataModelViewer()
{
    delete ui;
}

/*!
 * Creates a DataModelViewer for \a model and shows the window.
 *
 * Labels of the axes can be specified by \a labelX and \a labelY. If left empty, default axis
 * labels are "x" and "y".
 *
 * Depending on whether or not \a model is integrable (see AbstractIntegrableDataModel), the widget
 * also offers the option to visualize bin integrals.
 *
 * The widget will be deleted automatically if the window is closed.
 *
 * Example:
 * \code
 * // get the attenuation model of lead from the database
 * auto attModel = attenuationModel(database::Element::Pb);
 *
 * // visualize
 * gui::DataModelViewer::plot(attModel, "Energy [keV]", "Mass atten. coeff. [cm^2/g]");
 * \endcode
 */
void DataModelViewer::plot(std::shared_ptr<AbstractDataModel> model,
                           const QString& labelX, const QString& labelY)
{
    auto viewer = new DataModelViewer;
    viewer->setAttribute(Qt::WA_DeleteOnClose);

    viewer->setData(std::move(model));

    viewer->setLabelX(labelX);
    viewer->setLabelY(labelY);

    viewer->resize(800, 600);
    viewer->show();
}

/*!
 * Returns the viewport for displaying the line series data in this instance. Use this to adjust its
 * specific settings if required.
 *
 * Example: setting a specific axis range for the line series view and changing the chart title
 * \code
 * auto viewer = new gui::DataModelViewer;
 * // ...
 *
 * viewer->dataViewValues()->setRangeX(20.0, 30.0);
 * viewer->dataViewValues()->chart()->setTitle("Example title");
 * viewer->show();
 * \endcode
 *
 * \sa LineSeriesView.
 */
LineSeriesView* DataModelViewer::dataViewValues() const
{
    return _lineView;
}

/*!
 * Returns the viewport for displaying the bin integral data in this instance. Use this to adjust
 * its specific settings if required.
 *
 * Example: setting a dark theme for the bin integral chart
 * \code
 * auto viewer = new gui::DataModelViewer;
 * // ...
 *
 * viewer->dataViewBinIntegrals()->chart()->setTheme(QtCharts::QChart::ChartThemeDark);
 * viewer->show();
 * \endcode
 *
 * \sa IntervalSeriesView.
 */
IntervalSeriesView* DataModelViewer::dataViewBinIntegrals() const
{
    return _intervalView;
}

/*!
 * Sets the data model visualized by this instance to \a model.
 *
 * Note that the model is accessed through a shared_ptr, meaning that any changes in its parameters
 * will effect all instances of the object. If model paramters are changed from outside this viewer
 * instance, the plot is not updated automatically but needs to be refreshed with updatePlot(), if
 * necessary.
 */
void DataModelViewer::setData(std::shared_ptr<AbstractDataModel> model)
{
    _model = std::move(model);
    if(_model->isIntegrable())
        ui->_RB_binIntegrals->setEnabled(true);
    else
    {
        ui->_RB_binIntegrals->setEnabled(false);
        ui->_RB_values->setChecked(true);
    }

    _lineView->chart()->setTitle(_model->name());
    _intervalView->chart()->setTitle(_model->name());

    ui->_W_parameterEditor->updateInterface(_model->parameter());
    ui->_W_parameterEditor->isEmpty() ? ui->_GB_parameter->hide() : ui->_GB_parameter->show();

    updatePlot();
}

// public slots

/*!
 * Increases the number of sampling points by 25% of their current value.
 */
void DataModelViewer::increaseSamplingDensity()
{
    setNumberOfSamples(qCeil(ui->_SB_nbSamples->value() * 1.25));
}

/*!
 * Hides the model parameter GUI element if \a hide = \c true and shows it otherwise.
 */
void DataModelViewer::hideParameterGUI(bool hide)
{
    hide ? ui->_GB_parameter->hide() : ui->_GB_parameter->show();
}

/*!
 * Reduces the number of sampling points to 80% of their current value.
 */
void DataModelViewer::reduceSamplingDensity()
{
    setNumberOfSamples(qCeil(ui->_SB_nbSamples->value() * 0.8));
}

/*!
 * Sets the label of the *x*-axis of both plot types to \a label.
 */
void DataModelViewer::setLabelX(const QString& label)
{
    _lineView->setLabelX(label);
    _intervalView->setLabelX(label);
}

/*!
 * Sets the label of the *y*-axis of both plot types to \a label.
 */
void DataModelViewer::setLabelY(const QString& label)
{
    _lineView->setLabelY(label);
    _intervalView->setLabelY(label);
}

/*!
 * Sets the number of sampling points to \a nbSamples.
 */
void DataModelViewer::setNumberOfSamples(int nbSamples)
{
    ui->_SB_nbSamples->setValue(nbSamples);
}

/*!
 * Sets the range within which the model is sampled to [\a from, \a to].
 */
void DataModelViewer::setSamplingRange(float from, float to)
{
    ui->_SB_rangeFrom->setValue(from);
    ui->_SB_rangeTo->setValue(to);
}

/*!
 * Toggles between linear and logarithmic *y*-axis display.
 */
void DataModelViewer::toggleLogY()
{
    _lineView->toggleLinLogY();
    _intervalView->toggleLinLogY();

    if(QObject::sender() != ui->_PB_linLogY)
    {
        ui->_PB_linLogY->blockSignals(true);
        ui->_PB_linLogY->toggle();
        ui->_PB_linLogY->blockSignals(false);
    }
}

/*!
 * Updates the current plot. This will readout all UI elements for information on sampling and
 * performs a new sampling of the values from the model.
 *
 * Usually called automatically. Call this method manually if model parameters have been changed
 * from outside this instance and a corresponding update of the plot is desired.
 */
void DataModelViewer::updatePlot()
{
    const auto nbSamples = ui->_SB_nbSamples->value();
    const auto range = SamplingRange(ui->_SB_rangeFrom->value(), ui->_SB_rangeTo->value());

    if(ui->_RB_values->isChecked())
    {
        auto sampledValues = XYDataSeries::sampledFromModel(*_model, range.linspace(nbSamples));

        _lineView->setData(sampledValues.data());
        ui->_stackedWidget->setCurrentWidget(_lineView);
    }
    else // bin integrals is checked
    {
        auto sampledValues = IntervalDataSeries::sampledFromModel(
                    static_cast<const AbstractIntegrableDataModel&>(*_model),
                    range.start(), range.end(), nbSamples);

        _intervalView->setData(sampledValues);
        ui->_stackedWidget->setCurrentWidget(_intervalView);
    }
}

void DataModelViewer::keyPressEvent(QKeyEvent *event)
{
    if(event->modifiers() == Qt::CTRL && event->key() == Qt::Key_S)
    {
        if(ui->_stackedWidget->currentWidget() == _lineView)
            _lineView->saveDialog();
        else
            _intervalView->saveDialog();

        event->accept();
    }

    QWidget::keyPressEvent(event);
}

void DataModelViewer::setModelParameter(QVariant parameter)
{
    _model->setParameter(parameter);
    updatePlot();
}


namespace details {

ParameterConfigWidget::ParameterConfigWidget(QWidget* parent)
    : QWidget(parent)
    , _layout(new QGridLayout)
{
    this->setLayout(_layout);
}

bool ParameterConfigWidget::isEmpty() const
{
    return !static_cast<bool>(_layout->count());
}

void ParameterConfigWidget::updateInterface(QVariant templateParameter)
{
    clearLayout();

    QVariantMap dataMap = templateParameter.toMap();

    int row = 0;
    QVariantMap::const_iterator i = dataMap.constBegin();
    while (i != dataMap.constEnd())
    {
        if(i.value().type() == QVariant::Bool)
        {
            auto widget = new QCheckBox("enable");
            widget->setChecked(i.value().toBool());
            _layout->addWidget(widget, row, 1);
            connect(widget, SIGNAL(toggled(bool)), this, SLOT(somethingChanged()));
        }
        else if(i.value().type() == QVariant::Int || i.value().type() == QVariant::UInt)
        {
            auto widget = new QSpinBox;
            widget->setValue(i.value().toInt());
            widget->setRange(-100000,100000);
            _layout->addWidget(widget, row, 1);
            connect(widget, SIGNAL(valueChanged(int)), this, SLOT(somethingChanged()));
        }
        else if(i.value().type() == QVariant::Double || i.value().type() == 38)
        {
            auto widget = new QDoubleSpinBox;
            widget->setValue(i.value().toDouble());
            widget->setRange(-100000.0,100000.0);
            _layout->addWidget(widget, row, 1);
            connect(widget, SIGNAL(valueChanged(double)), this, SLOT(somethingChanged()));
        }
        else // cannot be represented
        {
            ++i;
            continue;
        }

        _layout->addWidget(new QLabel(i.key()), row, 0);
        ++row;
        ++i;
    }

}

void ParameterConfigWidget::clearLayout()
{
    const auto nbItems = _layout->count();

    for(int item = nbItems-1; item >=0 ; --item)
    {
        QWidget* w = _layout->itemAt(item)->widget();
        if(w != NULL)
        {
            _layout->removeWidget(w);
            delete w;
        }
    }
}

QVariant ParameterConfigWidget::parsedInputWidget(QWidget *widget)
{
    QVariant ret;

    if(dynamic_cast<QCheckBox*>(widget))
        ret = dynamic_cast<QCheckBox*>(widget)->isChecked();
    else if(dynamic_cast<QSpinBox*>(widget))
        ret = dynamic_cast<QSpinBox*>(widget)->value();
    else if(dynamic_cast<QDoubleSpinBox*>(widget))
        ret = dynamic_cast<QDoubleSpinBox*>(widget)->value();

    return ret;
}

void ParameterConfigWidget::somethingChanged()
{
    const auto nbItems = _layout->count();
    const auto nbRows = nbItems / 2;

    QVariantMap map;
    for (int row = 0; row < nbRows; ++row)
    {
        auto name = static_cast<QLabel*>(_layout->itemAtPosition(row, 0)->widget())->text();
        map.insert(name, parsedInputWidget(_layout->itemAtPosition(row, 1)->widget()));
    }

    emit parameterChanged(map);
}

} // namespace details

} // namespace gui
} // namespace CTL
