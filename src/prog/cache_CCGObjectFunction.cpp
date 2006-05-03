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
#include <otawa/cache/ccg/CCGBuilder.h>
#include <otawa/hardware/Platform.h>

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
				int hit_time = lbloc->countInsts(/*cach*/) * fw->platform()->pipelineDepth();
				int miss_time = hit_time + cach->missPenalty();
				// this fuction compute  chit & cmiss with 5 cycles and return the
				//number of inst in thr l-bloc with cache as parametre
  				int counter = lbloc->countInsts(/*cach*/);
  				system->addObjectFunction(
  					hit_time,
  					lbloc->use<ilp::Var *>(CCGBuilder::ID_HitVar));
  				system->addObjectFunction(
  					miss_time,
  					lbloc->use<ilp::Var *>(CCGBuilder::ID_MissVar));
					
			}
		
		}
		
	}
}

}//otawa

