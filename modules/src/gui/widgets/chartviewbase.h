#ifndef CTL_CHARTVIEWBASE_H
#define CTL_CHARTVIEWBASE_H

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
    QImage image(const QSize& renderSize = QSize());

public slots:
    void autoRange();
    bool save(const QString& fileName);
    void saveDialog();
    void setLabelX(const QString& label);
    void setLabelY(const QString& label);
    void setLogAxisY(bool enabled);
    void setOverRangeY(bool enabled);
    void setRangeX(double from, double to);
    void setRangeY(double from, double to);
    void setUseNiceX(bool enabled);
    void toggleLinLogY();

protected:
    explicit ChartViewBase(QWidget* parent = nullptr);

    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

    void mySetAxisX(QtCharts::QAbstractAxis* axisX, QtCharts::QAbstractSeries* series);
    void mySetAxisY(QtCharts::QAbstractAxis* axisY, QtCharts::QAbstractSeries* series);
    QtCharts::QAbstractAxis* myAxisX(QtCharts::QAbstractSeries* series);
    QtCharts::QAbstractAxis* myAxisY(QtCharts::QAbstractSeries* series);
    void setSeriesShow(QtCharts::QAbstractSeries* series, bool shown);
    void switchToLinAxisY();
    void switchToLogAxisY();
    bool yAxisIsLinear() const;

    QtCharts::QChart* _chart;
    QtCharts::QAbstractSeries* _plottableSeries;
    QtCharts::QAbstractSeries* _plottableSeriesLog;
    QtCharts::QLineSeries* _dataSeries;
    QtCharts::QLineSeries* _dataSeriesLog;

private:
    bool _overRangeY = false;
    bool _useNiceX = false;
};


} // namespace gui
} // namespace CTL

#endif // CTL_CHARTVIEWBASE_H
