/*
 *	$Id$
 *	feature module interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2005-08, IRIT UPS.
 *
 *	OTAWA is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	OTAWA is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with OTAWA; if not, write to the Free Software
 *	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <otawa/proc/Feature.h>
#include <otawa/proc/SilentFeature.h>
#include <otawa/proc/FeatureDependency.h>

namespace otawa {

// null feature
static Feature<NullProcessor> _null;


// feature property
Identifier<bool> IS_FEATURE("otawa::IS_FEATURE", false);


/**
 * @class AbstractFeature
 * See @ref Feature.
 * @ingroup proc
 */

/**
 * Build a simple feature.
 * @param name Name of the feature (only for information).
 */
AbstractFeature::AbstractFeature(cstring name)
: Identifier<Processor *>(name, 0) {
	IS_FEATURE(this) = true;
}


/**
 */
AbstractFeature::~AbstractFeature(void) {
}


/**
 * Null value for features.
 */
AbstractFeature& AbstractFeature::null = _null;


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
 * @fn void AbstractFeature::clean(WorkSpace *ws) const;
 * This method is called each time a feature is invalidated. In this case, the
 * feature must removed all properties put on the workspace.
 */


namespace p {

/**
 * @class feature
 * Shortcut to create a feature with a maker (without the mess of @ref SilentFeature).
 *
 * Such a feature must be declared in a header file by
 * @code
 * extern p::feature MY_FEATURE;
 * @endcode
 *
 * And defined by:
 * @code
 * p::feature MY_FEATURE("MY_FEATURE", new Maker<MyDefaultProcessor>());
 * @endcode
 */


/**
 * Build the feature.
 * @param name		Feature name.
 * @param maker		Maker to build the default processor.
 * @notice			This class is responsible for freeing the maker passed in parameter.
 */
feature::feature(cstring name, AbstractMaker *maker): AbstractFeature(name), _maker(maker) {
}


/**
 */
feature::~feature(void) {
	if(_maker != null_maker && _maker != no_maker)
		delete _maker;
}


/**
 */
void feature::process(WorkSpace *ws, const PropList& props) const {
	Processor *p = _maker->make();
	p->process(ws, props);
	delete p;
}


/**
 */
void feature::check(WorkSpace *fw) const {
}


/**
 */
void feature::clean(WorkSpace *ws) const {
}

}	// p


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
 * @ingroup proc
 */


/**
 * @fn Feature::GenFeature(CString name);
 * Build a feature.
 * @param name	Feature name.
 */


/**
 * @class SilentFeature;
 * The usual @ref Feature class has as drawback to exhibit completely the processing of the feature
 * and therefore, in C++, to require too much header file inclusion (like the default processor
 * or the default handler structure).
 *
 * @par
 *
 * This class allow to fix this using default processor builder that does not need to be included
 * in the feature declaration. The constructor of this class requires as parameter the name of
 * the feature and a factory class for the processor, @ref SilentFeature::AbstractMaker.
 * Usually, the factory object is implemented used the @ref SilenfFeature::Maker, that is a template
 * but does only need to be declared in the source file. Consequently, the header file declaring
 * the feature is no more overloaded with the inclusion of the default processor.
 *
 * Here is an example of use, first the header file "my_feature.h":
 * @code
 * #include <otawa/proc/SilentFeature.h>
 *
 * extern SilentFeature MyFeature;
 * @endcode
 *
 * Then, the source is defined as:
 * @code
 * #include <path_to/MyFeature.h>
 * #include <path_to/MyDefaultProcessor.h>
 *
 * static SilentFeature::Maker<MyDefaultProcessor> maker;
 * SilentFeature MyFeature("MyFeature", maker);
 * @endcode
 * @ingroup proc
 */


/**
 * @fn SilentFeature::SilentFeature(cstring name, const AbstractMaker& maker);
 * Build a silent feature.
 * @param name	Name of the feature.
 * @param maker	Factory to build the default processor.
 */


/**
 */
void SilentFeature::process(WorkSpace *fw, const PropList& props) const {
	Processor *processor = _maker.make();
	processor->process(fw, props);
}


/**
 * @class SilentFeature::AbstractMaker;
 * Interface for the processor maker of @ref SilentFeature.
 */


/**
 * @fn Processor *SilentFeature::make(void) const;
 * Build the linked processor.
 * @return	Built processor.
 */


/**
 * @class Maker
 * Template to make a processor from its class passed as template parameter.
 * @param C	Type of the processor.
 * @seealso SilentFeature
 */


/**
 * Identifier for identifier property providing ownerness of an identifier.
 * The arguments describes the feature owning the current
 */
Identifier<const AbstractFeature *> DEF_BY("otawa::DEF_BY", 0);


/**
 * @class FeatureDependency
 * A feature dependency represents the dependencies used to implement a feature and is a node
 * of the dependency graph.
 */


/**
 * Build a dependency for the given feature.
 * @param _feature	Feature this dependency represents.
 */
FeatureDependency::FeatureDependency(const AbstractFeature *_feature)
: feature(_feature) {
}


/**
 */
FeatureDependency::~FeatureDependency(void) {
	ASSERT(children.isEmpty());
	for(list_t::Iterator parent(parents); parent; parent++)
		parent->children.remove(this);
}


/**
 * Create a dependency from the current feature to the given one.
 * @param to	Feature which the current feature depends to.
 */
void FeatureDependency::add(FeatureDependency *to) {
	to->children.add(this);
	parents.add(to);
}


/**
 * Remove the dependency of the current feature from the given one.
 * @param from	Feature to remove depedency from.
 */
void FeatureDependency::remove(FeatureDependency *from)  {
	from->children.remove(this);
	parents.remove(from);
}


/**
 * @fn const AbstractFeature *FeatureDependency::getFeature(void) const;
 * Get the feature supported by this dependency.
 * @return	Feature ported by this dependency.
 */

} // otawa
