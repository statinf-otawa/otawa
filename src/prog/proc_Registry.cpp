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
#include <otawa/proc/Registration.h>
#include <otawa/proc/Processor.h>
#include <otawa/proc/Feature.h>

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
 * @ingroup proc
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
 * @ingroup proc
 */


/**
 * Build the registration.
 */
AbstractRegistration::AbstractRegistration(void): _base(&Processor::reg) {
}


/**
 * Build of a custom registration.
 * @param base		Base registration.
 */
AbstractRegistration::AbstractRegistration(AbstractRegistration *base) {
	_base = base;
	_name = base->_name;
	_version = base->_version;
}


/**
 * Build a new registration.
 * @param name		Processor name.
 * @param version	Processor version.
 * @param base		Base processor.
 */
AbstractRegistration::AbstractRegistration(string name, Version version, AbstractRegistration *base)
: _name(name), _version(version), _base(base) {
}


/**
 * Set the features.
 * @param coll		Features to set.
 */
void AbstractRegistration::setFeatures(const SLList<FeatureUsage>& coll) {
	features.clear();
	for(genstruct::SLList<FeatureUsage>::Iterator use(coll); use; use++)
		features.add(use);
}


/**
 * Set the configurations.
 * @param coll	Configures to set.
 */
void AbstractRegistration::setConfigs(const SLList<AbstractIdentifier *>& coll) {
	configs.clear();
	for(genstruct::SLList<AbstractIdentifier *>::Iterator use(coll); use; use++)
		configs.add(use);
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
 * Test if the current processor uses or requires the given feature.
 * @param feature	Tested feature.
 * @return			True if the feature is used or required, false else.
 */
bool AbstractRegistration::uses(const AbstractFeature& feature) {
	for(FeatureIter fuse(*this); fuse; fuse++)
		if(&(fuse->feature()) == &feature
		&& (fuse->kind() == FeatureUsage::require || fuse->kind() == FeatureUsage::use))
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
 * Initialize a registration from an argument list.
 * @param name		Processor name.
 * @param version	Processor version.
 * @param tag		First tag.
 * @param args		List of pair (tag, value) ended by a @ref otawa::p::end value.
 */
void AbstractRegistration::init(cstring name, const Version& version, int tag, VarArg& args) {
	_name = name;
	_version = version;
	while(tag != p::end) {
		switch(tag) {
		case p::require:
			features.add(FeatureUsage(FeatureUsage::require, *args.next<const AbstractFeature *>()));
			break;
		case p::provide:
			features.add(FeatureUsage(FeatureUsage::provide, *args.next<const AbstractFeature *>()));
			break;
		case p::invalidate:
			features.add(FeatureUsage(FeatureUsage::invalidate, *args.next<const AbstractFeature *>()));
			break;
		case p::use:
			features.add(FeatureUsage(FeatureUsage::use, *args.next<const AbstractFeature *>()));
			break;
		case p::base:
			_base = args.next<AbstractRegistration *>();
			break;
		case p::config:
			configs.add(args.next<AbstractIdentifier *>());
			break;
		default:
			ASSERTP(0, "bad registration argument for processor " << name << " (" << version << ")");
		}
		tag = args.next<int>();
	}
	record();
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
 * @fn Registration::Registration(void);
 * Default constructor.
 */


/**
 * @fn Registration::Registration(cstring name, const Version& version, int tag, ...);
 * @param name		Name of the processor.
 * @param version	Version of the processor.
 * @param tag		First tag.
 * @param ...		List of pairs (tag, value) ended by @ref otawa::p::end .
 */


/**
 * @fn Registration::Registration(cstring name, const Version& version, int tag, VarArgs& args);
 * @param name		Name of the processor.
 * @param version	Version of the processor.
 * @param tag		First tag.
 * @param args		List of pairs (tag, value) ended by @ref otawa::p::end .
 */

namespace p {

/**
 * @class make
 * Used internally for processor declaration.
 */

/**
 * @class declare
 * Class to declare simple a processor.
 * Each processor must have a static member of this type giving information about:
 * @li configuration identifier,
 * @li required features,
 * @li provided features,
 * @li invalidated features,
 * @li used features,
 * @li maker for the default processor.
 *
 * Below an example of processor registration using declaration:
 * @code
 * class MyProcessor: public BBProcessor {
 * public:
 * 	static p::declare reg;
 * 	...
 * };
 *
 * p::declare MyProcessor::reg =
 * 		p::init("MyProcessor", Version(1, 0, 0), BBProcessor::reg)
 * 		.require(OTHER_FEATURE)
 * 		.provide(MY_FEATURE)
 * 		.config(MY_CONFIG)
 * 		.make<MyProcessor>());
 * @endcode
 *
 * @ingroup proc
 */


/**
 */
declare::declare(otawa::p::init& maker)
	: AbstractRegistration(maker._name, maker._version, maker._base ? maker._base : &Processor::reg)
{
	setFeatures(maker.features);
	setConfigs(maker.configs);
	_maker = maker._maker;
	record();
}


declare::~declare(void) {
	if(_maker)
		delete _maker;
}

/**
 */
Processor *declare::make(void) const {
	if(_maker)
		return _maker->make();
	else
		return 0;
}

/**
 */
bool declare::isFinal(void) const {
	return true;
}

}	// p

static Maker<NullProcessor> null_inst;
static Maker<NoProcessor> no_inst;

/**
 * Special maker for a null processor.
 */
AbstractMaker *null_maker = &null_inst;


/**
 * Special maker for "no processor available".
 */
AbstractMaker *no_maker = &no_inst;

} // otawa
