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

    void setData(std::shared_ptr<AbstractDataModel> model);

    static void plot(std::shared_ptr<AbstractDataModel> model,
                     const QString& labelX, const QString& labelY);

public slots:
    void updatePlot();
    void setNumberOfSamples(int nbSamples);
    void increaseSamplingDensity();
    void reduceSamplingDensity();
    void setXLabel(const QString& label);
    void setYLabel(const QString& label);

private:
    LineSeriesView* _lineView;
    IntervalSeriesView* _intervalView;
    Ui::DataModelViewer *ui;

    std::shared_ptr<AbstractDataModel> _model;

    void setNumberSamplesSilent(int val);
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
