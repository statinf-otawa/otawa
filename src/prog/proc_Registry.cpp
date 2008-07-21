/*
 *	$Id$
 *	Registration class implementation
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

#include <otawa/proc/Registry.h>

namespace otawa {


/**
 * @class Registry
 * Class dedicated to the registering of the processors.
 * A registration contains:
 * @li name and version of the processor,
 * @li list of configuration identifiers,
 * @li list of the used features (provided, required, invalidated).
 * 
 * The registration may be used to write processor scripts or for GUI to provide
 * list of processors to the user.
 */


/**
 * Default registry.
 */
Registry Registry::_;


/**
 * Look for a processor matching the given name.
 * @param name	Name of the processor to look for.
 * @return		Found processor or null.
 */
const AbstractRegistration *Registry::find(CString name) {
	return _.procs.get(name, 0);
}


/**
 * To ensure that no other registry is built.
 */
Registry::Registry(void): Initializer<AbstractRegistration>(false) {
	startup();
}


/**
 * @class AbstractRegistration
 * Abstract class to represent the registered processors.
 */


/**
 * Build the registration.
 */
AbstractRegistration::AbstractRegistration(void): _base(0) {
}


/**
 */
void AbstractRegistration::record(void) {
	Registry::_.record(this);	
}


/**
 * Test if the current processor provides the given feature.
 * @param feature	Tested feature.
 * @return			True if the feature is provided, false else.
 */
bool AbstractRegistration::provides(const AbstractFeature& feature) {
	for(FeatureIter fuse(*this); fuse; fuse++)
		if(&(fuse->feature()) == &feature
		&& fuse->kind() == FeatureUsage::provide)
			return true;
	return false;
}
 

/**
 * Test if the current processor requires the given feature.
 * @param feature	Tested feature.
 * @return			True if the feature is required, false else.
 */
bool AbstractRegistration::requires(const AbstractFeature& feature) {
	for(FeatureIter fuse(*this); fuse; fuse++)
		if(&(fuse->feature()) == &feature
		&& fuse->kind() == FeatureUsage::require)
			return true;
	return false;
}
 

/**
 * Test if the current processor invalidates the given feature.
 * @param feature	Tested feature.
 * @return			True if the feature is invalidated, false else.
 */
bool AbstractRegistration::invalidates(const AbstractFeature& feature) {
	for(FeatureIter fuse(*this); fuse; fuse++)
		if(&(fuse->feature()) == &feature
		&& fuse->kind() == FeatureUsage::invalidate)
			return true;
	return false;
}
 

/**
 * @fn const string& AbstractRegistration::name(void) const;
 * Get the name of the processor.
 * @return	Processor name.
 */


/**
 * @fn const Version& AbstractRegistration::version(void) const;
 * Get the version of the processor.
 * @return	Processor version.
 */


/**
 * @fn Processor *AbstractRegistration::make(void) const;
 * Build the registered processor.
 * @return	Built processor.
 */


/**
 * Internal.
 */
void AbstractRegistration::initialize(void) {
	Registry::_.procs.put(name(), this);
}


/**
 * @class ConfigIter
 * Iterator on the configurations of a processor.
 * @param T	Processor class.
 */


/**
 */
void ConfigIter::step(void) {
	while(_all && reg && iter.ended()) {
		if((reg = &reg->base()))
			iter = SLList<AbstractIdentifier *>::Iterator(reg->configs);
	}
}


/**
 * @class FeatureIter
 * Iterator on the features used by the processor.
 * @param T		Type of the processor.
 */


/**
 */
void FeatureIter::step(void) {
	while(reg && iter.ended()) {
		if((reg = &reg->base()))
			iter = SLList<FeatureUsage>::Iterator(reg->features);
	}
}


/*
 * @class Registration
 * A registration for a processor of type T.
 * @param T		Type of the registered processor.
 */


/**
 * @class Registered
 * A registered class is used to build automatically a processor with a
 * registration.
 * @param T	Type of the processor to register.
 * @param B	Base type of the parent processor.
 *
 * To make a processor registrable, it must inherit from Registered and
 * defines a static function init() responsbible to initialize the registration.
 * This function invokes the initialization functions: name(), version(),
 * require(), provide(), invalidate().
 * 
 * An example is given below:
 * @code
 *	class MyProcessor: public Registered<MyProcessor, BBProcessor> {
 *	public:
 *		static void init(void) {
 *			name("my_namespace::MyProcessor");
 *			version(1, 0, 1);
 *			require(SOME_FEATURE);
 *			require(ANOTHER_FEATURE);
 *			provide(MY_FEATURE);
 *		}
 *	
 *	protected:
 *		void processBB(WorkSpace *ws, CFG *cfg, BB *bb) {
 *			// something useful
 *		}
 *	};
 * @endcode
 */

} // otawa
