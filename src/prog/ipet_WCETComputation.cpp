/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	src/ipet_WCETComputation.h -- WCETComputation class interface.
 */

#include <otawa/ilp.h>
#include <otawa/ipet/IPET.h>
#include <otawa/ipet/WCETComputation.h>

using namespace otawa::ilp;

namespace otawa {

/**
 * @class WCETComputation
 * This class is used for computing the WCET from the system found in the root
 * CFG.
 */


/**
 */
void WCETComputation::processCFG(CFG *cfg) {
	System *system = cfg->use<System *>(IPET::ID_System);
	int wcet = -1;
	if(system->solve())
		wcet = (int)system->value();
	cfg->set<int>(IPET::ID_WCET, wcet);
}

} // otawa
