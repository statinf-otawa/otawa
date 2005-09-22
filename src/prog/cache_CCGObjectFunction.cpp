/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	otawa/cache/ccg/CCGObjectFunction.h -- CCGConstraintsBuilder class implementation.
 */
#include<stdio.h>
#include <elm/io.h>
#include <otawa/cache/ccg/CCGObjectFunction.h>
#include <otawa/cfg.h>
#include <otawa/hardware/CacheConfiguration.h>

using namespace otawa::ilp;
using namespace otawa;
using namespace otawa::ipet;


namespace otawa {

/**
 */	
void CCGObjectFunction::processCFG(FrameWork *fw, CFG *cfg ) {
	assert(cfg);
	System *system = cfg->get<System *>(IPET::ID_System, 0);
	assert (system);
	LBlockSet **lbsets = cfg->use<LBlockSet **>(LBlockSet::ID_LBlockSet);
	const Cache *cach = fw->platform()->cache().instCache();
	
	for(int i = 0; i < cach->lineCount(); i++) {
		LBlockSet *idg = lbsets[i];
		
		// Building the object function which used by S. Malik 
		for(Iterator<LBlock *> lbloc(idg->visit()); lbloc; lbloc++) {
			if(lbloc->id() != 0 && lbloc->id() != idg->count()- 1) {
				// this fuction compute  chit & cmiss with 5 cycles and return the
				//number of inst in thr l-bloc with cache as parametre
  				int counter = lbloc->countInsts(5, cach);
  				system->addObjectFunction(lbloc->hitCount(), lbloc->hitVar());
  				system->addObjectFunction(lbloc->missCount(),lbloc->missVar());
					
			}
		
		}
		
	}
}

}//otawa

