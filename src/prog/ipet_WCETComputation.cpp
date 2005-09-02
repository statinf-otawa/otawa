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

using namespace otawa::ilp;

namespace otawa { namespace ipet {

/**
 * @class WCETComputation
 * This class is used for computing the WCET from the system found in the root
 * CFG.
 */


/**
 * Build a new WCET computer.
 * @param props		Configuration properties.
 */
WCETComputation::WCETComputation(const PropList& props)
: CFGProcessor("otawa::WCETComputation", Version(1, 0, 0), props) {
}


/**
 */
void WCETComputation::processCFG(FrameWork *fw, CFG *cfg) {
	System *system = cfg->use<System *>(IPET::ID_System);
	if(!system)
		throw ProcessorException(*this, "no ILP system defined in this CFG");
	int wcet = -1;
	if(system->solve())
		wcet = (int)system->value();
	cfg->set<int>(IPET::ID_WCET, wcet);
}

} } // otawa::ipet
