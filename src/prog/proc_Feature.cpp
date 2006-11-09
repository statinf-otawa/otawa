/*
 *	$Id$
 *	Copyright (c) 2005-06, IRIT UPS.
 *
 *	Feature and GenFeature class implementation.
 */

#include <otawa/proc/Feature.h>
#include <otawa/prop/NameSpace.h>

namespace otawa {


/**
 * @class AbstractFeature
 * See @ref Feature.
 */

/**
 * Basically, the features are identifier owned by this name space.
 */
NameSpace AbstractFeature::NS("features", OTAWA_NS);


/**
 * Build a simple feature.
 * @param name Name of the feature (only for information).
 */
AbstractFeature::AbstractFeature(CString name)
: GenericIdentifier<Processor *>(name, 0, NS) {
}


/**
 * @fn void AbstractFeature::process(FrameWork *fw, const PropList& props) const;
 * This method is called, when a feature is not available, to provided a
 * default implementation of the feature.
 * @param fw	Framework to work on.
 * @param props	Property list configuration.
 */


/**
 * @fn void AbstractFeature::check(FrameWork *fw) const;
 * Check if the framework really implement the current feature. If not, it
 * throws a @ref ProcessorException. This method is only usually called
 * for debugging purpose as its execution is often very large.
 */


/**
 * @class Feature
 * A feature is a set of facilities, usually provided using properties,
 * available on a framework. If a feature is present on a framework, it ensures
 * that the matching properties all also set.
 * @p
 * Most of the time, the features are provided and required by the code
 * processor. If a feature is not available, a processor matching the feature
 * is retrieved from the processor configuration property list if any or
 * from a default processor tied to the @ref Feature class. This processor
 * gives a default implementation computing the lacking feature. 
 * 
 * @param T	Default processor to compute the feature.
 * @param C Feature checker. This type (class or structure) must provide
 * a function called "check" as provided by the @ref Feature class.
 */


/**
 * @fn Feature::GenFeature(CString name);
 * Build a feature.
 * @param name	Feature name.
 */

} // otawa
