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
#include <otawa/prop/DynIdentifier.h>
#include <otawa/proc/ProcessorPlugin.h>

namespace otawa {

/**
 * @defgroup dyn	Dynamic Identifier, Feature and Processor
 *
 * As some identifier, feature and processors are located in plug-ins, they can
 * be retrieved at linking time. To use them, one has to load automatically
 * the matching plug-in and then to access the required object.
 *
 * This achieved automatically using the three classes:
 * @li @ref DynIdentifier
 * @li @ref DynFeature
 * @li @ref DynProcessor
 *
 * These classes works as proxies using the symbolic name of the looked object.
 * If not already resolved, they try tore load the plug-in from the name of object
 * (must be full qualified C++ name) and look the matching actual object.
 *
 * The example below show the use of these classes. First, we have an header file
 * provided by the plugin: MyPlugin.h.
 * @code
 * #define USE_MY_FEATURE		static DynFeature MY_FEATURE("mine::MyFeature");
 * #define USE_MY_IDENTIFIER	static Identifier<int> MY_IDENTIFIER("mine::MyIdentifier");
 * @endcode
 *
 * The MyPlugin.cpp file declare the feature and the identifier as usual:
 * @code
 * Feature<MyProcessor> MY_FEATURE("mine::MyFeature");
 * Identifier<int> MY_IDENTIFIER("mine::MyIdentifier", -1);
 * @endcode
 *
 * The user application includes the header file, uses the macro definition
 * and use identifiers as usual.
 * @code
 * #include <MyPlugin.h>
 *
 * USE_MY_FEATURE;
 * USE_MY_FEATURE;
 *
 * class AnotherProcessor: public Processor {
 * public:
 * 		AnotherProcessor(void) {
 * 			require(MY_FEATURE);
 * 		}
 *
 * 		void processWorkSpace(WorkSpace *ws) {
 * 			int v = MY_IDENTIFIER(ws);
 * 			...
 * 		}
 * @endcode
 */

/**
 * @class class FeatureNotFound
 * This exception is thrown when a feature can not be resolved.
 * @ingroup dyn
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
 * @ingroup dyn
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


/**
 * @class DynIdentifier
 * An identifier dynamically resolved.
 * Pass the full-qualified C++ name to the constructor and use it as a normal identifier.
 * @param T		Type of identifier value.
 * @ingroup dyn
 */


/**
 * @fn DynIdentifier::DynIdentifier(cstring name);
 * Build a dynamic identifier.
 * @param name	Full-qualified C++ name of the dynamic identifier.
 */


/**
 * @fn cstring DynIdentifier::name(void) const;
 * Get the full-qualified C++ name of the identifier.
 * @return		Identifier name.
 */

}	// otawa

