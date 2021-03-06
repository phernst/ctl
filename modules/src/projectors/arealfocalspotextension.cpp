#include "arealfocalspotextension.h"
#include "components/genericgantry.h"
#include "components/genericsource.h"
#include "acquisition/preparesteps.h"

#include <future>

namespace CTL {

DECLARE_SERIALIZABLE_TYPE(ArealFocalSpotExtension)

/*!
 * Constructs an ArealFocalSpotExtension with a focal spot sub-sampling given by \a discretization
 * and linearization approximation enabled if \a lowExtinctionApproximation = \c true.
 *
 * \sa setDiscretization(), enableLowExtinctionApproximation()
 */
ArealFocalSpotExtension::ArealFocalSpotExtension(const QSize& discretization,
                                                 bool lowExtinctionApproximation)
    : _discretizationSteps(discretization)
    , _lowExtinctionApprox(lowExtinctionApproximation)
{
}

/*!
 * Re-implementation of the configuration step. This takes copies of the AcquisitionSetup
 * and the AbstractProjectorConfig. The actual configure() method of the nested projector is
 * called within project(). Use setDiscretization() to change the level of discretization.
 */
void ArealFocalSpotExtension::configure(const AcquisitionSetup& setup)
{
    _setup = setup;

    ProjectorExtension::configure(setup);
}

/*!
 * Re-implementation of the projection step.
 *
 * This method invokes an individual projection computation (i.e. delegation to the project() method
 * of the nested projector object) for each of the discretization points that are requested. Returns
 * the average of the resulting individual projection images.
 *
 * In short, the work flow is as follows:
 *
 *
 * \c foreach \a discretizationPoint
 * \li Create a copy of the AcquisitionSetup
 * \li Compute offset of \a discretizationPoint w.r.t. to focal spot center (i.e. [0,0,0])
 * for the specific focal spot size of all views in the setup
 * \li Add this offset to the preparation pipeline for the corresponding views
 * \li Call configure() of the nested projector with the resulting setup
 * \li Invoke project() of the nested projector
 * \li Accumulate projections (in intensity domain, unless low extinction approximation activated)
 *
 * \c end \c foreach
 * \li Return averaged projections.
 *
 * Detailled description:\n
 * Technically, this is realized using the source displacement functionality. The focal spot covers
 * the area of \f$\left[-\frac{w_{fs}}{2},\frac{w_{fs}}{2}\right]\times
 * \left[-\frac{h_{fs}}{2},\frac{h_{fs}}{2}\right]\f$, where \f$w_{fs}\f$ and \f$h_{fs}\f$ denote
 * the width and height of the focal spot, respectively. This area is discretized into
 * \c _discretizationSteps.width() x \c _discretizationSteps.height() steps. For each of these
 * points, the corresponding spatial shift (w.r.t. the point source location used in conventional
 * methods) is added to the source displacement. Afterwards, the nested projector is configured
 * (using its configure() method) and and projections are simulated with the resulting
 * system (using project() from the nested projector). Note that adding this displacement component
 * is required (instead of simply setting it) in order to allow for other sources of displacement
 * that already exist in the system before simulating the focal spot extent.
 *
 * This extension will invoke an individual projection computation for each of the sampling points.
 * Hence, computation time increases (at least) linearly with the number of requested sampling
 * points. Furthermore, required system memory is doubled, since two full sets of projections need
 * to be kept in memory simultaneously.
 *
 * By default, projections are averaged in intensity domain.
 * This makes the extension non-linear. To enforce averaging in extinction domain, and by that,
 * make the extension linear, enable the low extinction approximation (see
 * enableLowExtinctionApproximation()).
 */
ProjectionData ArealFocalSpotExtension::extendedProject(const MetaProjector& nestedProjector)
{
    ProjectionData ret(0, 0, 0);

    // get focal spot sampling points
    QVector<QPointF> discGrid = discretizationGrid();

    const int nbSamplingPts = discGrid.size();
    // foreach sampling point
    bool first = true;

    // potential parallelization
    auto processProj = [&ret] (ProjectionData* nextProj)
    {
        nextProj->transformToIntensity();
        ret += *nextProj;
    };

    std::future<void> fut = std::async([]{});
    ProjectionData nextProj(0,0,0);

    uint ptCount = 1;
    for(const auto& point : discGrid)
    {   
        emit notifier()->information("Processing sub-sample " + QString::number(ptCount++) + "/" +
                                     QString::number(nbSamplingPts) + " of areal focal spot.");

        AcquisitionSetup tmpSetup(_setup);

        for(uint view = 0; view < _setup.nbViews(); ++view)
        {
            _setup.prepareView(view);
            auto xraySource = _setup.system()->source();

            // preparation of spot position
            const auto spotSize = xraySource->focalSpotSize();
            const Vector3x1 focalSpotShift = {
                point.x() * spotSize.width(), point.y() * spotSize.height(), 0.0 };
            const mat::Location additionalDisplacement(focalSpotShift, mat::eye<3>());
            auto displacer = std::make_shared<prepare::GantryDisplacementParam>();
            displacer->setSourceDisplacement(_setup.system()->gantry()->sourceDisplacement());
            displacer->incrementSourceDisplacement(additionalDisplacement);

            // preparation of photon flux
            auto intensityMod = std::make_shared<prepare::SourceParam>();
            intensityMod->setFluxModifier(xraySource->fluxModifier() / nbSamplingPts);

            tmpSetup.view(view).addPrepareStep(displacer);
            tmpSetup.view(view).addPrepareStep(intensityMod);
        }

        // re-configure projector
        ProjectorExtension::configure(tmpSetup);

        // projecting volume
        if(first)
        {
            ret = nestedProjector.project();
            if(!_lowExtinctionApprox) // average in intensity domain required
                ret.transformToIntensity();
        }
        else
        {
            auto proj = nestedProjector.project();
            if(!_lowExtinctionApprox)
            {
                fut.wait();
                nextProj = std::move(proj);
                fut = std::async(processProj, &nextProj);
            }
            else // average in extinction domain
                ret += proj;
        }

        first = false;
    }

    // average result with respect to number of focal spot sampling points
    fut.wait();
    ret /= nbSamplingPts;

    if(!_lowExtinctionApprox)
        ret.transformToExtinction();

    return ret;
}

/*!
 * Sets the discretization of the focal spot to \a discretization. The focal spot will then be
 * sampled with \c discretization.width() x \c discretization.height() sampling points. These
 * sampling dimensions correspond to the width and height of the focal spot (or x and y direction
 * in CT coordinates).
 */
void ArealFocalSpotExtension::setDiscretization(const QSize& discretization)
{
    _discretizationSteps = discretization;
}

/*!
 * Sets the use of the low extinction approximation to \a enable.
 *
 * When activated, the low extinction approximation causes projection images of individual focal
 * spot sub-samples to be averaged in extinction domain instead of in intensity domain. This is an
 * approximation that allows the ArealFocalSpotExtension to become a linear extension, which has
 * potential performance benefit when used in combination with other extensions. However, the
 * result will become inaccurate, particularly if strong extinction gradients are present in the
 * projection images. For low extinction (and esp. their gradients), the approximation is
 * acceptable.
 *
 * From a mathematical point of view, this requires:
 * \f$
 * -\ln\frac{1}{F}\sum_{f=1}^{F}\exp(-\epsilon_{f})\approx\frac{1}{F}\sum_{f=1}^{F}\epsilon_{f},
 * \f$
 *
 * where \f$\epsilon_{f}\f$ denotes the extinction value of a certain pixel for focal spot
 * sub-sample \f$f\f$. It can be shown that this is fulfilled for \f$\epsilon_{f}\ll 1\f$ (overall
 * low extinction values) or \f$\epsilon_{f}=\epsilon+\delta_{f}\f$ with \f$\delta_{f}\ll 1\f$
 * (small gradients, i.e. different focal spot positions create only small changes in extinction).
 */
void ArealFocalSpotExtension::enableLowExtinctionApproximation(bool enable)
{
    _lowExtinctionApprox = enable;
}

// Use AbstractProjector::toVariant() documentation.
QVariant ArealFocalSpotExtension::toVariant() const
{
    QVariantMap ret = ProjectorExtension::toVariant().toMap();

    ret.insert("#", "ArealFocalSpotExtension");

    return ret;
}

/*!
 * Returns the parameters of this instance as QVariant.
 *
 * This returns a QVariantMap with three key-value-pairs: The first to are
 * ("Discretization X", _discretizationSteps.width()) and ("Discretization Y",
 * _discretizationSteps.height()), which refer to the number of sampling points used to sub-sample
 * the focal spot in x and y direction (CT coordinates), respectively. The third key-value-pair is
 * ("Low extinction approx", _lowExtinctionApprox) and represents the information whether this
 * instance has the low extinction approximation enabled.
 *
 * This method is used within toVariant() to serialize the object's settings.
 */
QVariant ArealFocalSpotExtension::parameter() const
{
    QVariantMap ret = ProjectorExtension::parameter().toMap();

    ret.insert("Discretization X", _discretizationSteps.width());
    ret.insert("Discretization Y", _discretizationSteps.height());
    ret.insert("Low extinction approx", _lowExtinctionApprox);

    return ret;
}

// Use AbstractProjector::setParameter() documentation.
void ArealFocalSpotExtension::setParameter(const QVariant& parameter)
{
    QVariantMap map = parameter.toMap();
    _discretizationSteps.setWidth(map.value("Discretization X", 1).toInt());
    _discretizationSteps.setHeight(map.value("Discretization X", 1).toInt());
    enableLowExtinctionApproximation(map.value("Low extinction approx", false).toBool());
}

/*!
 * Computes and returns the grid of (relative) sampling points.
 *
 * This discretizes the area of the focal spot, i.e. \f$\left[-\frac{1}{2},\frac{1}{2}\right]\times
 * \left[-\frac{1}{2},\frac{1}{2}\right]\f$ into \c _discretizationSteps.width() x
 *  \c _discretizationSteps.height() steps.
 *
 * See detailed class description for a figure depicting the discretization pattern.
 */
QVector<QPointF> ArealFocalSpotExtension::discretizationGrid() const
{
    QVector<QPointF> ret;
    const auto width = _discretizationSteps.width();
    const auto height = _discretizationSteps.height();
    ret.reserve(width * height);

    double xStep = 0.0, yStep = 0.0; // step width from one sampling point to next
    double startX = 0.0, startY = 0.0; // starting position of sampling

    // compute step width of sampling and starting point only if more than one point is requested
    // (otherwise, keep initialization with zero unchanged)
    if(width > 1)
    {
        xStep = 1.0 / double(width - 1);
        startX = -0.5; // move starting position to left border
    }
    if(height > 1)
    {
        yStep = 1.0 / double(height - 1);
        startY = -0.5; // move starting position to upper border
    }

    // compute all sampling points
    QPointF gridPoint;
    for(int xDiscr = 0; xDiscr < width; ++xDiscr)
    {
        gridPoint.setX(startX + xDiscr * xStep);
        for(int yDiscr = 0; yDiscr < height; ++yDiscr)
        {
            gridPoint.setY(startY + yDiscr * yStep);
            ret.append(gridPoint);
        }
    }

    return ret;
}

/*!
 * Returns \c false (requires averaging operation in intensity domain) unless the low extinction
 * approximation is enabled.
 *
 * \sa enableLowExtinctionApproximation()
 */
bool ArealFocalSpotExtension::isLinear() const
{
    return _lowExtinctionApprox;
}

} // namespace CTL
