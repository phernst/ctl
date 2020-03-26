#include "intervalseriesview.h"

#include "models/intervaldataseries.h"

namespace CTL {
namespace gui {

IntervalSeriesView::IntervalSeriesView(QWidget* parent)
    : QChartView(parent)
    , _areaSeries(new QAreaSeries)
    , _upper(new QLineSeries)
    , _lower(new QLineSeries)
    , _chart(new QChart)
{
    _areaSeries->setLowerSeries(_lower);
    _areaSeries->setUpperSeries(_upper);
    _areaSeries->setColor(Qt::lightGray);
    _areaSeries->setBorderColor(Qt::darkGray);

    _chart->addSeries(_areaSeries);
    _chart->legend()->hide();
    _chart->createDefaultAxes();

    setChart(_chart);
    setRubberBand(QChartView::RectangleRubberBand);
    setWindowTitle("Bar Series View");
}

void IntervalSeriesView::plot(const IntervalDataSeries& intervalSeries,
                         const QString& labelX, const QString& labelY)
{
    auto viewer = new IntervalSeriesView;
    viewer->setAttribute(Qt::WA_DeleteOnClose);

    viewer->setData(intervalSeries);

    viewer->_chart->axisX()->setTitleText(labelX);
    viewer->_chart->axisY()->setTitleText(labelY);

    viewer->resize(500,400);
    viewer->show();
}

void IntervalSeriesView::setData(const IntervalDataSeries& intervalSeries)
{
    _upper->clear();
    _lower->clear();

    const auto eps = 0.0001;
    const auto binWidth = intervalSeries.binWidth();
    const auto xRange = intervalSeries.samplingRange();

    for(const auto& pt : intervalSeries.data())
    {
        _upper->append(pt.x() - (0.5 - eps) * binWidth, pt.y());
        _upper->append(pt.x() + 0.5 * binWidth, pt.y());
        _upper->append(pt.x() + (0.5 + eps) * binWidth, 0.0);

        _lower->append(pt.x() - (0.5 - eps) * binWidth, 0.0);
        _lower->append(pt.x() + 0.5 * binWidth, 0.0);
        _upper->append(pt.x() + (0.5 + eps) * binWidth, 0.0);
    }

    _chart->axisX(_areaSeries)->setRange(xRange.start(), xRange.end());
    _chart->axisY(_areaSeries)->setRange(intervalSeries.min(), 1.05 * intervalSeries.max());
}

void IntervalSeriesView::autoZoom() const
{
    auto compareX = [] (const QPointF& a, const QPointF& b) { return a.x()<b.x(); };
    auto compareY = [] (const QPointF& a, const QPointF& b) { return a.y()<b.y(); };

    const auto dataPts = _areaSeries->upperSeries()->pointsVector();
    QPair<double, double> xRange(std::min_element(dataPts.cbegin(), dataPts.cend(), compareX)->x(),
                                 std::max_element(dataPts.cbegin(), dataPts.cend(), compareX)->x());
    QPair<double, double> yRange(std::min_element(dataPts.cbegin(), dataPts.cend(), compareY)->y(),
                                 std::max_element(dataPts.cbegin(), dataPts.cend(), compareY)->y());

    _chart->axisX(_areaSeries)->setRange(xRange.first, xRange.second);
    _chart->axisY(_areaSeries)->setRange(yRange.first, 1.05 * yRange.second);
}

void IntervalSeriesView::mouseDoubleClickEvent(QMouseEvent* event)
{
    if(event->button() == Qt::LeftButton)
        autoZoom();

    event->accept();
}

} // namespace gui
} // namespace CTL
