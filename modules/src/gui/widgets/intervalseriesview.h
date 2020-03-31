#ifndef CTL_INTERVALSERIESVIEW_H
#define CTL_INTERVALSERIESVIEW_H

#include "chartviewbase.h"

namespace CTL {

class IntervalDataSeries;

namespace gui {

class IntervalSeriesView : public ChartViewBase
{
    Q_OBJECT
public:
    explicit IntervalSeriesView(QWidget* parent = nullptr);

    static void plot(const IntervalDataSeries& intervalSeries,
                     const QString& labelX = "x",
                     const QString& labelY = "y",
                     bool logAxisY = false);

    void setData(const IntervalDataSeries& intervalSeries);

private:
    double suitableLogMinVal(const IntervalDataSeries& intervalSeries);
};

} // namespace gui
} // namespace CTL

#endif // CTL_INTERVALSERIESVIEW_H
