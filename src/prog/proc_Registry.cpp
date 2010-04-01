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
#include <otawa/proc/Processor.h>

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

} // otawa
