/*
 *	$Id$
 *	FeatureRequirer class implementation.
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2007, IRIT UPS.
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

#include <elm/assert.h>
#include <otawa/proc/FeatureRequirer.h>
#include <otawa/prog/WorkSpace.h>

namespace otawa {

/**
 * @class FeatureRequirer
 * This class provided feature requirement for non-program processor classes.
 * It provides two methods that takes as parameter an array of required features
 * ended by the @ref NULL_FEATURE.
 * 
 * The first one, the constructor, takes as parameter the workspace and 
 * the feature array and check that the features are available. If not, an
 * assertion failure is raised. If the constant NDEBUG is defined, the check
 * is not performed and no time is lost in this verification.
 * 
 * The second method is static and add to the current processor the array
 * of feature as requirements. To make it working, each requirer must provide
 * to the user processor a static method that calls this one in order to add
 * the feature of the requirer to the feature of the processor. The constructor
 * is only used to assert that the require method has been called.
 * 
 * Below, you may found an example. First, we provided a facility with
 * a requirement:
 * @code
 *	#include <otawa/proc/FeatureRequirer.h>
 * 
 *	class MyFacility: public FeatureRequirer {
 *	public:
 *		MyFacility(WorkSpace *ws): FeatureRequirer(ws, res) { }
 *		void doSomethingUseful(void) { ... }
 * 
 *		static void require(Processor& proc) { require(proc, reqs); }
 *	private:
 *		static AbstractFeature *reqs[];
 *	};
 * 
 *	AbstractFeature *MyFacility::req[] = {
 *		&CONTROL_DECODING_FEATURE,
 *		&BB_TIME_FEATURE,
 *		&NULL_FEATURE
 *	};
 * @endcode
 * 
 * Then, my processor uses my facility as below:
 * @code
 *	class MyProcessor: public Processor {
 *	public:
 *		MyProcessor(void): Processor("MyProcessor", Version(1, 0, 0)) {
 * 			provide(MY_FEATURE);
 * 			require(LOOP_INFOS_FEATURE);
 * 			MyFacility::require(*this);
 * 		}
 *
 * 		void process(WorkSpace *ws) {
 *			...
 *			MyFacility facility(ws);
 *			...
 *		}
 *	};
 * @endcode
 */


/**
 * Check that the features are available.
 * @param ws		Workspace to work on.
 * @param features	Array of feature to check (ended by @ref NULL_FEATURE).
 */
FeatureRequirer::FeatureRequirer(WorkSpace *ws, AbstractFeature *features[]) {
#	ifndef NDEBUG
		for(int i = 0; features[i] != &AbstractFeature::null; i++)
			ASSERTP(ws->isProvided(*features[i]),
				"require() method on a feature requirer class has not been called");
#	endif
}


/**
 * Added the given array of feature to the requirements of the processor.
 * @param proc		Processor to add feature requirements to.
 * @param features	Array of feature to a requirement for (ended by @ref
 * 					NULL_FEATURE).
 */
void FeatureRequirer::require(Processor& proc, AbstractFeature *features[]) {
	for(int i = 0; features[i] != &AbstractFeature::null; i++)
		proc.require(*features[i]);	
}


/**
 * Not a real feature, just a marker.
 */
Feature<NoProcessor> NULL_FEATURE("otawa::NULL");

}	// otawa

