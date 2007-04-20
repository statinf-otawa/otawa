/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	src/ipet_WCETComputation.h -- WCETComputation class interface.
 */

#include <otawa/ilp.h>
#include <otawa/ipet/IPET.h>
#include <otawa/ipet/WCETComputation.h>
#include <otawa/cfg/CFG.h>
#include <otawa/proc/ProcessorException.h>
#include <otawa/ipet/BasicConstraintsBuilder.h>
#include <otawa/ipet/BasicObjectFunctionBuilder.h>
#include <otawa/ipet/FlowFactLoader.h>
#include <otawa/proc/Registry.h>

using namespace otawa::ilp;

namespace otawa { namespace ipet {

// Registration
static WCETComputation proc;
static Registration reg(proc, AUTODOC "/classotawa_1_1ets_1_1WCETComputation.html");


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
WCETComputation::WCETComputation(void)
: Processor("otawa::ipet::WCETComputation", Version(1, 0, 0)) {
	require(CONTROL_CONSTRAINTS_FEATURE);
	require(OBJECT_FUNCTION_FEATURE);
	require(FLOW_FACTS_CONSTRAINTS_FEATURE);
	provide(WCET_FEATURE);
}


/**
 */
void WCETComputation::processFrameWork(FrameWork *fw) {
	CFG *cfg = ENTRY_CFG(fw);
	System *system = SYSTEM(fw);
	if(!system)
		throw ProcessorException(*this, "no ILP system defined in this CFG");
	int wcet = -1;
	if(system->solve())
		wcet = (int)system->value();
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
