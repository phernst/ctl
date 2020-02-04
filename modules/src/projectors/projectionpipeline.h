#ifndef PROJECTIONPIPELINE_H
#define PROJECTIONPIPELINE_H

#include "projectorextension.h"

namespace CTL {

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

    ProjectionPipeline();

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
    uint nbExtensions() const;

private:
    ExtensionPtr _finalProjector;

    AbstractProjector* _projector;
    std::vector<ProjectorExtension*> _extensions;

    void stashExtensions(uint nbExt);
    void restoreExtensions(uint nbExt);
};

}

#endif // PROJECTIONPIPELINE_H
