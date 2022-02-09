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

#include <otawa/proc/DynFeature.h>
#include <otawa/proc/Feature.h>
#include <otawa/proc/FeatureDependency.h>
#include <otawa/proc/ProcessorPlugin.h>

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
class RegistrationMaker: public AbstractMaker {
public:
	RegistrationMaker(p::declare& reg): _reg(reg) { }
	virtual Processor *make(void) const { return _reg.make(); }
private:
	p::declare& _reg;
};

/**
 * Build a feature from processor registration.
 * @param name	Feature name.
 */
feature::feature(cstring name, p::declare& reg): AbstractFeature(name), _maker(new RegistrationMaker(reg)) {
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
	if(p == nullptr)
		throw MessageException(_
			<< "no processor for require feature: "
			<< name());
	ws->run(p, props, true);
}


/**
 * For internal use only. Work-around the non-definition of WorkSpace
 * at this point.
 */
void *get_impl(WorkSpace *ws, const AbstractFeature& feature) {
	Processor *p = ws->getImpl(feature);
	if(p == nullptr)
		return nullptr;
	else
		return p->interfaceFor(feature);
}


/**
 * Test if the given identifier is a feature identifier.
 * @param id	Identifier to test.
 * @return		True if it is a feature, false else.
 * @ingroup proc
 */
bool is_feature(const AbstractIdentifier *id) {
	return IS_FEATURE(id);
}


/**
 * Retrieve a feature by its name, possibly loading corresponding plug-in.
 * @param name	Full-qualified "::"-separated feature name.
 * @return		Found feature or null.
 * @ingroup proc
 */
AbstractFeature *find_feature(cstring name) {
	return ProcessorPlugin::getFeature(name);
}


/**
 * Find an identifierby its name.
 * @param name	Name of the looked identifier.
 * @return		Found identifier or null.
 * @ingroup proc
 */
AbstractIdentifier *find_id(cstring name) {
	return ProcessorPlugin::getIdentifier(name);
}


/**
 * @fn p::id<T>& get_id(cstring name);
 * Find and return an identifier for the given and which value type corresponds
 * to the given type T. If the identifier cannot be found, perform an
 * assertion failure.
 *
 * @param T		Type of identifier values.
 * @param name	Name of the identifier.
 * @return		Corresponding identifier.
 * @ingroup proc
 */


/**
 * @class interfaced_feature
 *
 * Objects of this class declare a feature supporting an interface.
 *
 * @param I		Class of the interface.
 *
 * @ingroup proc
 */


/**
 * @fn const I *interfaced_feature::give(const P *p) const;
 * This function may used by a processor implementing an interfaced feature
 * in order to return the interface in its function Processor::interfaceFor().
 * Calling this function is only convenient if the processor class implements
 * also the feature interface.
 * @param p		Current processor.
 * @return		Current processor converted to the interface.
 */


/**
 * @fn const I *interfaced_feature::get(WorkSpace *ws) const;
 * This function is called to obtain the interface of an interfaced feature.
 * @param ws	Workspace to look for the feature interface.
 * @return		Feature interface in the given workspace (or null if the feature is not provided).
 */

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
	for(list_t::Iter parent(parents); parent(); parent++)
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
