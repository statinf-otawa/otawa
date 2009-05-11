/*
 *	$Id$
 *	DynFeature class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2009, IRIT UPS.
 *
 *	OTAWA is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	OTAWA is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for mo re details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with OTAWA; if not, write to the Free Software
 *	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <otawa/proc/DynFeature.h>
#include <otawa/proc/ProcessorPlugin.h>

namespace otawa {

/**
 * @class class FeatureNotFound
 * This exception is thrown when a feature can not be resolved.
 */

/**
 * Build the exception.
 * @param name	Name of the feature that can not be resolved.
 */
FeatureNotFound::FeatureNotFound(string name)
: otawa::Exception(_ << "feature " << name << " can not be found"), _name(name) {
}

/**
 * @fn const string& FeatureNotFound::name(void) const;
 * Get the name of the feature that has caused this exception.
 * @return	Unresolved feature name.
 */


/**
 * @class DynFeature
 * This class is used to resolve feature found in plugins using
 * ProcessorPlugin::getFeature() method. Notice that this class perform late binding:
 * the feature is only resolved when it is used.
 */

/**
 * Build the dynamic featue.
 * @param name	Name of the feature.
 */
DynFeature::DynFeature(string name)
: feature(0), _name(name) {
}

/**
 * Bind the feature.
 * @throw FeatureNotFound	Launched when the feature can not be resolved.
 */
void DynFeature::init(void) const throw(FeatureNotFound) {
	feature = ProcessorPlugin::getFeature(&_name);
	if(!feature)
		throw FeatureNotFound(_name);
}

/**
 * @fn DynFeature::operator AbstractFeature *(void) const throw(FeatureNotFound);
 * if not already done, bind the feature and return a pointer to.
 * @throw FeatureNotFound	Launched when the feature can not be resolved.
 */


/**
 * @fn AbstractFeature *DynFeature::operator*(void) const throw(FeatureNotFound);
 * if not already done, bind the feature and return a pointer to.
 * @throw FeatureNotFound	Launched when the feature can not be resolved.
 */

/**
 * @fn AbstractFeature::operator AbstractFeature&(void) const throw(FeatureNotFound);
 * if not already done, bind the feature and return a reference to.
 * @throw FeatureNotFound	Launched when the feature can not be resolved.
 */

}	// otawa

