/*
 *	$Id$
 *	WCETComputation class implementation
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

#include <otawa/ilp.h>
#include <otawa/ipet/IPET.h>
#include <otawa/ipet/WCETComputation.h>
#include <otawa/cfg/CFG.h>
#include <otawa/proc/ProcessorException.h>
#include <otawa/ipet/BasicConstraintsBuilder.h>
#include <otawa/ipet/BasicObjectFunctionBuilder.h>
#include <otawa/ipet/FlowFactConstraintBuilder.h>
#include <otawa/proc/Registry.h>
#include <otawa/ipet/ILPSystemGetter.h>

using namespace otawa::ilp;

namespace otawa { namespace ipet {

// Registration
void WCETComputation::init(void) {
	_name("otawa::ipet::WCETComputation");
	_version(1, 0, 0);
	_require(CONTROL_CONSTRAINTS_FEATURE);
	_require(OBJECT_FUNCTION_FEATURE);
	_require(FLOW_FACTS_CONSTRAINTS_FEATURE);
	_provide(WCET_FEATURE);
}
	

/**
 * @class WCETComputation
 * This class is used for computing the WCET from the system found in the root
 * CFG.
 * 
 * @par Required Features
 * @li @ref ipet::CONTROL_CONSTRAINTS_FEATURE
 * @li @ref ipet::OBJECT_FUNCTION_FEATURE
 * @li @ref ipet::FLOW_FACTS_CONSTRAINTS_FEATURE
 * 
 * @par Provided Features
 * @li @re ipet::WCET_FEATURE
 */


/**
 * Build a new WCET computer.
 */
WCETComputation::WCETComputation(void) {
}


/**
 */
void WCETComputation::processWorkSpace(WorkSpace *fw) {
	//CFG *cfg = ENTRY_CFG(fw);
	System *system = SYSTEM(fw);
	if(!system)
		throw ProcessorException(*this, "no ILP system defined in this CFG");
	time_t wcet = -1;
	if(isVerbose())
		log << "\tlaunching ILP solver\n";
	if(system->solve()) {
		if(isVerbose())
			log << "\tobjective function = " << system->value() << io::endl;
		wcet = (time_t)system->value();
	}
	if(isVerbose())
		log << "\tWCET = " << wcet << io::endl; 
	WCET(fw) = wcet;
}


/**
 * This feature ensures that the WCET has been computed using IPET approach.
 * 
 * @par Properties
 * @li @ref ipet::WCET (FrameWork)
 */
Feature<WCETComputation> WCET_FEATURE("otawa::ipet::WCET");

} } // otawa::ipet
