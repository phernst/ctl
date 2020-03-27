#ifndef CTL_INTERVALSERIESVIEW_H
#define CTL_INTERVALSERIESVIEW_H

#include <QtCharts>

namespace CTL {

class IntervalDataSeries;

namespace gui {

class IntervalSeriesView : public QChartView
{
    Q_OBJECT
public:
    explicit IntervalSeriesView(QWidget* parent = nullptr);

    static void plot(const IntervalDataSeries& intervalSeries,
                     const QString& labelX = "x",
                     const QString& labelY = "y",
                     bool logAxisY = false);

    void setData(const IntervalDataSeries& intervalSeries);

    QImage image(const QSize& renderSize = QSize());

public slots:
    void autoRange();
    bool save(const QString& fileName);
    void saveDialog();
    void setLabelX(const QString& label);
    void setLabelY(const QString& label);
    void setLogAxisY(bool enabled);
    void setRangeX(double from, double to);
    void setRangeY(double from, double to);
    void setUseNiceX(bool enabled);
    void toggleLinLogY();

protected:
    void keyPressEvent(QKeyEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;

private:
    QAreaSeries* _areaSeries;
    QAreaSeries* _areaSeriesLog;
    QLineSeries* _upper;
    QLineSeries* _upperLog;
    QChart* _chart;
    bool _useNiceX = false;

    void setSeriesShow(QAbstractSeries* series, bool shown);
    double suitableLogMinVal(const IntervalDataSeries& intervalSeries);
    void switchToLinAxisY();
    void switchToLogAxisY();
    bool yAxisIsLinear() const;
};

} // namespace gui
} // namespace CTL

#endif // CTL_INTERVALSERIESVIEW_H
