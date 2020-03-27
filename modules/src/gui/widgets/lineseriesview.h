#ifndef LINESERIESVIEW_H
#define LINESERIESVIEW_H

#include <QtCharts>

namespace CTL {

class XYDataSeries;

namespace gui {

class LineSeriesView : public QChartView
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
    void autoRange() const;
    void setLabelX(const QString& label);
    void setLabelY(const QString& label);
    void setLogAxisY(bool enabled);
    void setUseNiceX(bool enabled);
    void setShowPoints(bool enabled = true);
    void toggleLinLogY();

protected:
    void mouseDoubleClickEvent(QMouseEvent *event) override;

private:
    QLineSeries* _dataSeries;
    QLineSeries* _dataSeriesLog;
    QChart* _chart;
    bool _useNiceX = false;

    void setSeriesShow(QAbstractSeries* series, bool shown);
    void switchToLinAxisY();
    void switchToLogAxisY();
    void updateLogData();
    bool yAxisIsLinear() const;
};

} // namespace gui
} // namespace CTL

#endif // LINESERIESVIEW_H
