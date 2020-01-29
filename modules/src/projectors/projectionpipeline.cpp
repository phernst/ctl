#include "projectionpipeline.h"

namespace CTL {

DECLARE_SERIALIZABLE_TYPE(ProjectionPipeline)

void ProjectionPipeline::configure(const AcquisitionSetup &setup)
{
    _finalProjector->configure(setup);
}

ProjectionData ProjectionPipeline::project(const VolumeData &volume)
{
    return _finalProjector->project(volume);
}

bool ProjectionPipeline::isLinear() const
{
    return _finalProjector->isLinear();
}

void ProjectionPipeline::fromVariant(const QVariant& variant)
{
    AbstractProjector::fromVariant(variant);

    QVariantMap map = variant.toMap();

    QVariant projVar = map.value("projector");
    setProjector(projVar.isNull() ? nullptr : SerializationHelper::parseProjector(projVar));

    QVariantList extList = map.value("extensions").toList();
    for(const auto& ext : extList)
    {
        if(!ext.isNull())
            appendExtension(static_cast<ProjectorExtension*>(SerializationHelper::parseProjector(ext)));
    }
}

QVariant ProjectionPipeline::toVariant() const
{
    QVariantMap ret = AbstractProjector::toVariant().toMap();

    ret.insert("#", "ProjectionPipeline");
    ret.insert("projector",
               _projector ? _projector->toVariant() : QVariant());


    QVariantList extensionList;
    for(const auto& ext : _extensions)
        extensionList.append(ext ? ext->toVariant() : QVariant());

    ret.insert("extensions", extensionList);

    return ret;
}


ProjectionPipeline::ProjectionPipeline()
    : _finalProjector(makeExtension<ProjectorExtension>())
{
}

void ProjectionPipeline::appendExtension(ProjectorExtension *extension)
{
    // add pointer to extension list
    _extensions.push_back(extension);

    // extend projector with new extension
    _finalProjector |= extension;
}

void ProjectionPipeline::insertExtension(uint pos, ProjectorExtension* extension)
{
    qDebug() << "ProjectionPipeline::insertExtension at pos " << pos;

    uint oldNbExt = nbExtensions();
    // append case
    if(pos >= oldNbExt)
        pos = oldNbExt; // end position

    ProjectorExtension* tmpProj = _finalProjector.release();

    // temporarily remove all extensions up to position 'pos'
    for(uint ext = 0; ext < oldNbExt - pos; ++ext)
        tmpProj = static_cast<ProjectorExtension*>(tmpProj->release());

    // insert new extension
    _extensions.insert(_extensions.begin() + pos, extension);
    pipe(tmpProj, extension);

    // reinstantiate all extensions
    for(uint ext = pos + 1; ext < nbExtensions(); ++ext)
        pipe(tmpProj, _extensions[ext]);

    // set final projector to finished object
    _finalProjector.reset(tmpProj);
}

void ProjectionPipeline::setProjector(AbstractProjector* projector)
{
    qDebug() << "ProjectionPipeline::setProjector";

    ProjectorExtension* tmpProj = _finalProjector.get();

    // temporarily remove all extensions
    for(uint ext = 0; ext < nbExtensions(); ++ext)
        tmpProj = static_cast<ProjectorExtension*>(tmpProj->release());

    // replace projector step
    _projector = projector;
    tmpProj->use(projector);

    // reinstantiate all extensions
    for(uint ext = 0; ext < nbExtensions(); ++ext)
        pipe(tmpProj, _extensions[ext]);
}

ProjectorExtension* ProjectionPipeline::releaseExtension(uint pos)
{
    qDebug() << "ProjectionPipeline::releaseExtension at pos " << pos;

    uint oldNbExt = nbExtensions();
    if(pos >= oldNbExt)
        throw std::domain_error("ProjectionPipeline::releaseExtension: Trying to release extension "
                                "at an out-of-range position.");

    ProjectorExtension* tmpProj = _finalProjector.release();

    // temporarily remove all extensions up to position 'pos'
    for(uint ext = 0; ext < oldNbExt - pos; ++ext)
        tmpProj = static_cast<ProjectorExtension*>(tmpProj->release());

    // catch released extension pointer and remove extension from list
    ProjectorExtension* ret = _extensions[pos];
    _extensions.erase(_extensions.begin() + pos);

    // reinstantiate all remaining extensions
    for(uint ext = pos; ext < _extensions.size(); ++ext)
        pipe(tmpProj, _extensions[ext]);

    // set final projector to finished object
    _finalProjector.reset(tmpProj);

    return ret;
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
