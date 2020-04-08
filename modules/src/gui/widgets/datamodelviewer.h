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
