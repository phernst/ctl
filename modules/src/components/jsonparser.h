#ifndef JSONPARSER_H
#define JSONPARSER_H

#include "allcomponents.h"

namespace CTL {

namespace {

SystemComponent* parseComponentFromJson(const QJsonObject& object)
{
    SystemComponent* ret;
    auto typeID = static_cast<int>(object.value("type-id").toDouble(-1.0));

    switch(typeID)
    {
    // detectors
    case GenericDetector::Type:
        ret = new GenericDetector(object);
        break;
    case CylindricalDetector::Type:
        ret = new CylindricalDetector(object);
        break;
    case FlatPanelDetector::Type:
        ret = new FlatPanelDetector(object);
        break;
    // sources
    case GenericSource::Type:
        ret = new GenericSource(object);
        break;
    case XrayTube::Type:
        ret = new XrayTube(object);
        break;
    case XrayLaser::Type:
        ret = new XrayLaser(object);
        break;
    // gantries
    case GenericGantry::Type:
        ret = new GenericGantry(object);
        break;
    case TubularGantry::Type:
        ret = new TubularGantry(object);
        break;
    case CarmGantry::Type:
        ret = new CarmGantry(object);
        break;
    // beam modifiers
    case GenericBeamModifier::Type:
        ret = new GenericBeamModifier(object);
        break;
    // unspecific system component
    case SystemComponent::Type:
        ret = new SystemComponent(object);
        break;
    // unknown type
    default:
        ret = nullptr;
        break;
    }

    return ret;
}

SystemComponent* parseGenericComponentFromJson(const QJsonObject& object)
{
    SystemComponent* ret;
    auto genericTypeID = static_cast<int>(object.value("generic type-id").toDouble());

    switch(genericTypeID)
    {
    // Elemental Types
    case AbstractDetector::Type:
        ret = new GenericDetector(object);
        break;
    case AbstractSource::Type:
        ret = new GenericSource(object);
        break;
    case AbstractGantry::Type:
        ret = new GenericGantry(object);
        break;
    case AbstractBeamModifier::Type:
        ret = new GenericBeamModifier(object);
        break;
    // unknown type
    default:
        ret = nullptr;
        break;
    }

    return ret;
}

} // unnamed namespace

} // namespace CTL

/*! \file */
///@{
/*!
 * \fn SystemComponent* CTL::parseComponentFromJson(const QJsonObject& object)
 * \relates SystemComponent
 *
 * Parses a SystemComponent from a JSON \a object. Reads the "type-id" of the JSON \a object and
 * creates a new component with a class according to this "type-id". The object creation only
 * works if the corresponding component type has an entry in the switch-case list of this function.
 *
 * This switch-case list is intended to be extended with new types.
 *
 * If no known "type-id" is found, a `nullptr` is returned.
 */

/*!
 * \fn SystemComponent* CTL::parseGenericComponentFromJson(const QJsonObject& object)
 * \relates SystemComponent
 *
 * Checks for an elemental type and, if found, tries to restore (partly)
 * a corresponding Generic<Type>.
 *
 * If no known "generic type-id" is found, a `nullptr` is returned.
 */
///@}

#endif // JSONPARSER_H
