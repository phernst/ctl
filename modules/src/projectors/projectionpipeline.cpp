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

    const QVariantList extList = map.value("extensions").toList();
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
    try {
        // extend projector with new extension
        _finalProjector |= extension;

        // add pointer to extension list
        _extensions.push_back(extension);
    } catch (...) {
        delete extension;
        qCritical() << "Appending extension failed. Deleted extension object.";
        throw;
    }
}

void ProjectionPipeline::insertExtension(uint pos, ProjectorExtension* extension)
{
    qDebug() << "ProjectionPipeline::insertExtension at pos " << pos;

    const auto oldNbExt = nbExtensions();

    if(pos >= oldNbExt) // append case
    {
        appendExtension(extension);
        return;
    }

    // temporarily remove all extensions (from the back) up to position 'pos'
    stashExtensions(oldNbExt - pos);

    // insert new extension
    try {
        _finalProjector |= extension;
        _extensions.insert(_extensions.begin() + pos, extension);
    } catch (...) {
        delete extension;
        restoreExtensions(oldNbExt - pos);
        qCritical() << "Insertion of extension failed. Deleted extension object.";
        throw;
    }

    // restore all extensions
    restoreExtensions(oldNbExt - pos);
}

void ProjectionPipeline::setProjector(AbstractProjector* projector)
{
    qDebug() << "ProjectionPipeline::setProjector";

    const auto nbExt = nbExtensions();
    stashExtensions(nbExt);

    // replace projector step
    _projector = projector;
    _finalProjector->use(projector);

    restoreExtensions(nbExt);
}

ProjectorExtension* ProjectionPipeline::releaseExtension(uint pos)
{
    qDebug() << "ProjectionPipeline::releaseExtension at pos " << pos;

    const auto oldNbExt = nbExtensions();
    if(pos >= oldNbExt)
        throw std::domain_error("ProjectionPipeline::releaseExtension: Trying to release extension "
                                "at an out-of-range position.");

    stashExtensions(oldNbExt - pos);

    // catch released extension pointer and remove extension from list
    ProjectorExtension* ret = _extensions[pos];
    _extensions.erase(_extensions.begin() + pos);

    restoreExtensions(oldNbExt - pos - 1);

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

void ProjectionPipeline::stashExtensions(uint nbExt)
{
    ProjectorExtension* tmpProj = _finalProjector.release();

    // stash all extensions up to position 'pos'
    const auto fullNbExt = nbExtensions();
    for(auto i = 0u; i < nbExt; ++i)
        tmpProj = static_cast<ProjectorExtension*>(tmpProj->release());

    // set final projector to finished object
    _finalProjector.reset(tmpProj);
}

void ProjectionPipeline::restoreExtensions(uint nbExt)
{
    ProjectorExtension* tmpProj = _finalProjector.release();

    // restore 'nbExt' extensions
    const auto fullNbExt = nbExtensions();
    const auto startPos = fullNbExt - nbExt;
    for(auto ext = startPos; ext < fullNbExt; ++ext)
        pipe(tmpProj, _extensions[ext]);

    // set final projector to finished object
    _finalProjector.reset(tmpProj);
}

} // namespace CTL
