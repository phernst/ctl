#include "projectionpipeline.h"

namespace CTL {

void ProjectionPipeline::configure(const AcquisitionSetup &setup, const AbstractProjectorConfig &config)
{
    _finalProjector->configure(setup, config);
}

ProjectionData ProjectionPipeline::project(const VolumeData &volume)
{
    return _finalProjector->project(volume);
}

bool ProjectionPipeline::isLinear() const
{
    return _finalProjector->isLinear();
}

ProjectionPipeline::ProjectionPipeline()
    : _finalProjector(makeExtension<ProjectorExtension>())
{
}

void ProjectionPipeline::appendExtension(ProjectorExtension *extension)
{
    // add pointer to extension list
    _extensions.push_back(extension);
    // set current final projection object to be used in new extension
    extension->use(_finalProjector.release());
    // set final projection object to new extended projector
    _finalProjector.reset(extension);
}

void ProjectionPipeline::insertExtension(uint pos, ProjectorExtension* extension)
{
    qInfo() << "ProjectionPipeline::insertExtension at pos " << pos;

    // append case
    if(pos >= nbExtensions())
    {
        qInfo() << "redirect to append";
        appendExtension(extension);
        return;
    }

    // (real) insert case
    uint oldNbExt = nbExtensions();
    ProjectorExtension* tmpProj = _finalProjector.get();

    // temporarily remove all extensions up to position 'pos'
    for(uint ext = 0; ext < oldNbExt - pos; ++ext)
        tmpProj = static_cast<ProjectorExtension*>(tmpProj->release());

    // insert new extension
    _extensions.insert(_extensions.begin() + pos, extension);
    extension->use(tmpProj);
    tmpProj = extension;

    // reinstantiate all extensions
    for(uint ext = pos + 1; ext < nbExtensions(); ++ext)
    {
        _extensions[ext]->use(tmpProj);
        tmpProj = _extensions[ext];
    }
}

void ProjectionPipeline::setProjector(AbstractProjector* projector)
{
    qInfo() << "ProjectionPipeline::setProjector";
    ProjectorExtension* tmpProj = _finalProjector.get();
    // temporarily remove all extensions
    for(uint ext = 0; ext < nbExtensions(); ++ext)
        tmpProj = static_cast<ProjectorExtension*>(tmpProj->release());

    // replace projector step
    _projector = projector;
    tmpProj->use(projector);

    // reinstantiate all extensions
    for(uint ext = 0; ext < nbExtensions(); ++ext)
    {
        _extensions[ext]->use(tmpProj);
        tmpProj = _extensions[ext];
    }
}

ProjectionPipeline::ExtensionPtr ProjectionPipeline::takeExtension(uint pos)
{
    return ExtensionPtr(releaseExtension(pos));
}

void ProjectionPipeline::appendExtension(ExtensionPtr extension)
{
    appendExtension(extension.release());
}

void ProjectionPipeline::insertExtension(uint pos, ExtensionPtr extension)
{
    insertExtension(pos, extension.release());
}

ProjectorExtension* ProjectionPipeline::releaseExtension(uint pos)
{
    qInfo() << "ProjectionPipeline::releaseExtension at pos " << pos;

    uint oldNbExt = nbExtensions();
    if(pos >= oldNbExt)
        throw std::domain_error("ProjectionPipeline::releaseExtension: Trying to release extension "
                                "at out-of-range position.");

    if(pos == oldNbExt-1) // remove last extension
    {
         ProjectorExtension* tmpProj = static_cast<ProjectorExtension*>(_finalProjector->release());
         ProjectorExtension* ret = _extensions.back();
         _extensions.pop_back();
         _finalProjector.release();
         _finalProjector.reset(tmpProj);
         return ret;
    }

    ProjectorExtension* tmpProj = _finalProjector.get();

    // temporarily remove all extensions up to position 'pos'
    for(uint ext = 0; ext < oldNbExt - pos; ++ext)
        tmpProj = static_cast<ProjectorExtension*>(tmpProj->release());

    // catch released extension pointer and remove extension from list
    //ProjectorExtension* ret = tmpProj;
    ProjectorExtension* ret = _extensions[pos];
    _extensions.erase(_extensions.begin() + pos);

    // reinstantiate all remaining extensions
    for(uint ext = pos; ext < _extensions.size(); ++ext)
    {
        _extensions[ext]->use(tmpProj);
        tmpProj = _extensions[ext];
    }

    return ret;
}

void ProjectionPipeline::removeExtension(uint pos)
{
    delete releaseExtension(pos);
}

void ProjectionPipeline::setProjector(ProjectorPtr projector)
{
    setProjector(projector.release());
}


ProjectorExtension* ProjectionPipeline::extension(uint pos) const
{
    if(pos >= nbExtensions())
        throw std::domain_error("Pipeline extension access out-of-range.");
    return _extensions[pos];
}


uint ProjectionPipeline::nbExtensions() const
{
    return _extensions.size();
}

}
