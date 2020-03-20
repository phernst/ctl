#ifndef CTL_PROJECTIONPIPELINE_H
#define CTL_PROJECTIONPIPELINE_H

#include "projectorextension.h"

namespace CTL {

/*!
 * \class ProjectionPipeline
 *
 * \brief The ProjectionPipeline class is a convenience class to manage a composition of a projector
 * and additional extensions in a simple manner.
 *
 * This class provides a simple means to manage a projector along with an arbitrary number of
 * ProjectorExtension objects. It allows for manipulations of the processing pipeline in a list-like
 * fashion.
 *
 * Use appendExtension() to add another extension to the end of the current pipeline. Extensions can
 * also be inserted at arbitrary positions within the pipeline with insertExtension() as well as
 * removed with removeExtension(). The actual projector to be used can be set using setProjector()
 * or directly in the constructor.
 * All methods take ownership of the objects passed to them and destroy any previous object, in case
 * the replace or remove it. To take extensions out of the pipeline without destroying the object,
 * use releaseExtension() or takeExtension().
 *
 * All manipulations on the pipeline are executed immediately. That means each call to either of the
 * above mentioned methods can throw an exeption in case the pipeline that would result from the
 * manipulation is not possible (i.e. if it internally throws an exception during use()).
 *
 * To modify any settings of individual extensions (or the actual projector), pointers to the
 * corresponding objects can be accessed with extension() and projector(). Note that the ownership
 * remains at the ProjectionPipeline object and you need to cast the pointer to the correct type of
 * extension or projector in order to access its full interface. Best practice is to fully prepare
 * all settings of the extensions before adding them to the pipeline.
 *
 * The ProjectorPipeline object itself can be used in the same way as any projector; use configure()
 * to pass the AcquisitionSetup for the simulation and then call project() (or projectComposite())
 * with the volume dataset that shall be projected to create the simulated projections using the
 * full processing pipeline that is managed your ProjectorPipeline object.
 *
 * The following code example demonstrates the usage of a ProjectionPipeline for creating
 * projections of a bone ball phantom. The simulation shall be done using the
 * OCL::RayCasterProjector class as forward projector and the processing chain shall consider
 * spectral effects followed by addition of Poisson noise.
 * \code
 * // create ball phantom made of cortical bone
 * auto volume = SpectralVolumeData::ball(50.0f, 0.5f, 1.0f,
 *                                        database::attenuationModel(database::Composite::Bone_Cortical));
 *
 * // create a C-arm CT system and a short scan protocol with 10 views
 * auto system = CTsystemBuilder::createFromBlueprint(blueprints::GenericCarmCT());
 * auto setup = AcquisitionSetup(system, 10);
 * setup.applyPreparationProtocol(protocols::ShortScanTrajectory(750.0));
 *
 * // create the pipeline with a ray caster projector
 * ProjectionPipeline pipe(new OCL::RayCasterProjector);
 * // alternatively:
 * //   ProjectionPipeline pipe;
 * //   pipe.setProjector(new OCL::RayCasterProjector);
 *
 * // create a SpectralEffectsExtension and set the energy resolution to 7.5 keV
 * auto spectralExt = new SpectralEffectsExtension;
 * spectralExt->setSpectralSamplingResolution(7.5f);
 *
 * // add the spectral effects extension and a Poisson noise extension to the pipeline
 * pipe.appendExtension(spectralExt);
 * pipe.appendExtension(new PoissonNoiseExtension);
 *
 * // pass the acquisition setup and run the simulation
 * pipe.configure(setup);
 * auto projections = pipe.project(volume);
 * \endcode
 */
class ProjectionPipeline : public AbstractProjector
{
    CTL_TYPE_ID(200)

    // abstract interface
    public: void configure(const AcquisitionSetup& setup) override;
    public: ProjectionData project(const VolumeData& volume) override;

public:
    using ProjectorPtr = std::unique_ptr<AbstractProjector>;
    using ExtensionPtr = std::unique_ptr<ProjectorExtension>;

    ProjectionData projectComposite(const CompositeVolume &volume) override;
    bool isLinear() const override;

    // SerializationInterface interface
    void fromVariant(const QVariant &variant) override;
    QVariant toVariant() const override;

    ProjectorNotifier* notifier() override;

    ProjectionPipeline(AbstractProjector* projector = nullptr);

    void appendExtension(ExtensionPtr extension);
    void appendExtension(ProjectorExtension* extension);
    void insertExtension(uint pos, ExtensionPtr extension);
    void insertExtension(uint pos, ProjectorExtension* extension);
    ProjectorExtension* releaseExtension(uint pos);
    void removeExtension(uint pos);
    void setProjector(ProjectorPtr projector);
    void setProjector(AbstractProjector* projector);
    ExtensionPtr takeExtension(uint pos);

    ProjectorExtension* extension(uint pos) const;
    AbstractProjector* projector() const;
    uint nbExtensions() const;

private:
    void stashExtensions(uint nbExt);
    void restoreExtensions(uint nbExt);

    std::vector<ProjectorExtension*> _extensions; //!< (Ordered) list of pointers to all extensions.
    ExtensionPtr _finalProjector; //!< The fully-assembled projector (incl. all extensions).
    AbstractProjector* _projector; //!< Pointer to the actual projector object.
};

}

/*! \file */
///@{
/*!
* \typedef CTL::ProjectionPipeline::ExtensionPtr
*
* \brief Alias name for std::unique_ptr<ProjectorExtension>.
*/

/*!
* \typedef CTL::ProjectionPipeline::ProjectorPtr
*
* \brief Alias name for std::unique_ptr<AbstractProjector>.
*/
///@}

#endif // CTL_PROJECTIONPIPELINE_H
