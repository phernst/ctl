#include "projectionpipeline.h"

namespace CTL {

DECLARE_SERIALIZABLE_TYPE(ProjectionPipeline)

/*!
 * \brief Sets the acquisition setup for the simulation to \a setup.
 *
 * Sets the acquisition setup for the simulation to \a setup. This needs to be done prior to calling project().
 */
void ProjectionPipeline::configure(const AcquisitionSetup& setup)
{
    _finalProjector->configure(setup);
}

/*!
 * \brief Creates projection data from \a volume.
 *
 * Creates projection data from \a volume using the current processing pipeline configuration of
 * this instance. Uses the last acquisition setup set by configure().
 */
ProjectionData ProjectionPipeline::project(const VolumeData& volume)
{
    return _finalProjector->project(volume);
}

/*!
 * \brief Creates projection data from the composite volume \a volume.
 *
 * Creates projection data from the composite volume \a volume using the current processing pipeline
 * configuration of this instance. Uses the last acquisition setup set by configure().
 */
ProjectionData ProjectionPipeline::projectComposite(const CompositeVolume& volume)
{
    return _finalProjector->projectComposite(volume);
}

/*!
 * Returns true if the application of the full processing pipeline is linear.
 */
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

ProjectorNotifier *ProjectionPipeline::notifier()
{
    return _finalProjector->notifier();
}

/*!
 * Constructs a ProjectionPipeline object and sets the projector to \a projector.
 *
 * This object takes ownership of \a projector.
 */
ProjectionPipeline::ProjectionPipeline(AbstractProjector* projector)
    : _finalProjector(makeExtension<ProjectorExtension>())
{
    setProjector(projector);
}

/*!
 * Convenience overload of ProjectionPipeline(AbstractProjector*) for unique_ptr arguments.
 */
ProjectionPipeline::ProjectionPipeline(ProjectorPtr projector)
    : ProjectionPipeline(projector.release())
{
}

/*!
 * Appends the extension \a extension to the end of the pipeline.
 *
 * This object takes ownership of \a extension.
 */
void ProjectionPipeline::appendExtension(ProjectorExtension* extension)
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
/*!
 * Inserts the extension \a extension at position \a pos into the pipeline. If
 * \a pos >= nbExtensions(), the extension is appended.
 *
 * Note that the position refers only to the extensions in the pipeline (i.e. the actual projector
 * is not does not count towards the current number of extension).
 *
 * This object takes ownership of \a extension.
 */
void ProjectionPipeline::insertExtension(uint pos, ProjectorExtension* extension)
{
    qDebug() << "ProjectionPipeline::insertExtension at pos" << pos;

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

/*!
 * Sets the projector to \a projector. Destroys any previous projector object managed by this
 * instance.
 *
 * This object takes ownership of \a projector.
 */
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

/*!
 * Removes the extension at position \a pos from the pipeline. Throws an std::domain_error if
 * \a pos >= nbExtensions().
 *
 * Note that the position refers only to the extensions in the pipeline (i.e. the actual projector
 * is not does not count towards the current number of extension).
 *
 * The ownership of the released object is transfered to the caller.
 */
ProjectorExtension* ProjectionPipeline::releaseExtension(uint pos)
{
    qDebug() << "ProjectionPipeline::releaseExtension at pos" << pos;

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

/*!
 * Removes the extension at position \a pos from the pipeline. The extension object is wrapped into
 * a unique pointer and returned to the caller.
 * Throws an std::domain_error if \a pos >= nbExtensions().
 *
 * Note that the position refers only to the extensions in the pipeline (i.e. the actual projector
 * is not does not count towards the current number of extension).
 */
ProjectionPipeline::ExtensionPtr ProjectionPipeline::takeExtension(uint pos)
{
    return ExtensionPtr(releaseExtension(pos));
}

/*!
 * Appends the extension \a extension to the end of the pipeline.
 *
 * This object takes ownership of \a extension.
 */
void ProjectionPipeline::appendExtension(ExtensionPtr extension)
{
    appendExtension(extension.release());
}

/*!
 * Inserts the extension \a extension at position \a pos into the pipeline. If
 * \a pos >= nbExtensions(), the extension is appended.
 *
 * Note that the position refers only to the extensions in the pipeline (i.e. the actual projector
 * is not does not count towards the current number of extension).
 *
 * This object takes ownership of \a extension.
 */
void ProjectionPipeline::insertExtension(uint pos, ExtensionPtr extension)
{
    insertExtension(pos, extension.release());
}

/*!
 * Removes the extension at position \a pos from the pipeline. Throws an std::domain_error if
 * \a pos >= nbExtensions().
 *
 * The removed extension object is destroyed.
 */
void ProjectionPipeline::removeExtension(uint pos)
{
    delete releaseExtension(pos);
}

/*!
 * Sets the projector to \a projector. Destroys any previous projector object managed by this
 * instance.
 *
 * This object takes ownership of \a projector.
 */
void ProjectionPipeline::setProjector(ProjectorPtr projector)
{
    setProjector(projector.release());
}

/*!
 * Returns a (base-class) pointer to the extension at position \a pos in the current pipeline.
 *
 * Note that the position refers only to the extensions in the pipeline (i.e. the actual projector
 * is not does not count towards the current number of extension).
 *
 * Ownership remains at this instance.
 */
ProjectorExtension* ProjectionPipeline::extension(uint pos) const
{
    if(pos >= nbExtensions())
        throw std::domain_error("Pipeline extension access out-of-range.");
    return _extensions[pos];
}

/*!
 * Returns a (base-class) pointer to the projector that is currently set in the pipeline.
 *
 * Ownership remains at this instance.
 */
AbstractProjector* ProjectionPipeline::projector() const
{
    return _projector;
}

/*!
 * Returns the number of extensions in the pipeline.
 *
 * Note that the actual projector does not count towards the number of extensions, i.e. for a
 * pipeline consisting solely of a projector, nbExtensions() is zero.
 */
uint ProjectionPipeline::nbExtensions() const
{
    return static_cast<uint>(_extensions.size());
}

/*!
 * Temporarily removes \a nbExt extensions from the end of the pipeline.
 *
 * The removed objects are not deleted and need to be restored later using restoreExtensions() to
 * avoid memory leaks.
 */
void ProjectionPipeline::stashExtensions(uint nbExt)
{
    ProjectorExtension* tmpProj = _finalProjector.release();

    // stash all extensions up to position 'pos'
    for(auto i = 0u; i < nbExt; ++i)
        tmpProj = static_cast<ProjectorExtension*>(tmpProj->release());

    // set final projector to finished object
    _finalProjector.reset(tmpProj);
}

/*!
 * Restores \a nbExt extensions at the end of the pipeline.
 *
 * Extensions must have been removed before by stashExtensions().
 */
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
