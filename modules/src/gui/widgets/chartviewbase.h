#ifndef CHARTVIEWBASE_H
#define CHARTVIEWBASE_H

#include <QChartView>

namespace QtCharts {
class QLineSeries;
}

namespace CTL {
namespace gui {

class ChartViewBase : public QtCharts::QChartView
{
    Q_OBJECT
public:
    explicit ChartViewBase(QWidget* parent = nullptr);

    QImage image(const QSize& renderSize = QSize());

public slots:
    void autoRange();
    bool save(const QString& fileName);
    void saveDialog();
    void setLabelX(const QString& label);
    void setLabelY(const QString& label);
    void setLogAxisY(bool enabled);
    void setRangeX(double from, double to);
    virtual void setRangeY(double from, double to) = 0;
    void setUseNiceX(bool enabled);
    void toggleLinLogY();

protected:
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private:
    QtCharts::QAbstractSeries* _plottableSeries;
    QtCharts::QAbstractSeries* _plottableSeriesLog;
    QtCharts::QLineSeries* _dataSeries;
    QtCharts::QLineSeries* _dataSeriesLog;
    QtCharts::QChart* _chart;
    bool _useNiceX = false;

    void setSeriesShow(QtCharts::QAbstractSeries* series, bool shown);
    void switchToLinAxisY();
    void switchToLogAxisY();
    bool yAxisIsLinear() const;

    void mySetAxisX(QtCharts::QAbstractAxis* axisX, QtCharts::QAbstractSeries* series);
    void mySetAxisY(QtCharts::QAbstractAxis* axisY, QtCharts::QAbstractSeries* series);
    QtCharts::QAbstractAxis* myAxisX(QtCharts::QAbstractSeries* series);
    QtCharts::QAbstractAxis* myAxisY(QtCharts::QAbstractSeries* series);
};


} // namespace gui
} // namespace CTL

#endif // CHARTVIEWBASE_H
