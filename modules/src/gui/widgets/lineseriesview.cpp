#include "lineseriesview.h"

namespace CTL {
namespace gui {

LineSeriesView::LineSeriesView(QWidget* parent)
    : QChartView(parent)
{
    setWindowTitle("Line Series View");
}

void LineSeriesView::plot(const QList<QPointF>& lineSeries,
                          const QString& labelX, const QString& labelY)
{
    auto viewer = new LineSeriesView;
    viewer->setAttribute(Qt::WA_DeleteOnClose);

    auto chart = new QChart;

    auto series = new QLineSeries;
    series->append(lineSeries);

    chart->legend()->hide();
    chart->addSeries(series);
    chart->createDefaultAxes();
    chart->axisX()->setTitleText(labelX);
    chart->axisY()->setTitleText(labelY);

    viewer->setChart(chart);
    viewer->resize(500,400);
    viewer->show();
}

} // namespace gui
} // namespace CTL
