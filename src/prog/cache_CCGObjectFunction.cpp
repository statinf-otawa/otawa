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
#include <otawa/hard/CacheConfiguration.h>
#include <otawa/cache/ccg/CCGBuilder.h>
#include <otawa/hard/Platform.h>

using namespace otawa::ilp;
using namespace otawa;
using namespace otawa::ipet;


namespace otawa {

/**
 */	

void CCGObjectFunction::processFrameWork(FrameWork *fw) {
	CFG* entry_cfg = ENTRY_CFG(fw);
	System *system = getSystem(fw, entry_cfg);
	ASSERT (system);
	LBlockSet **lbsets = LBLOCKS(fw);
	const hard::Cache *cach = hard::CACHE_CONFIGURATION(fw)->instCache();
	
	for(int i = 0; i < cach->lineCount(); i++) {
		LBlockSet *idg = lbsets[i];
		
		// Building the object function which used by S. Malik 
		/* 
		 * OLD VERSION (CCGObjectFunction Builder)
		 * For each Lblock (i,j) we create two terms:
		 *  hittime_ij*xhit_ij + misstime_ij*xmiss_ij
		 * (that is equivalent to: hittime_ij*(x_i) + penalty_ij*xmiss_ij)
		 *
		 * NEW VERISON (CCGObjectFunction Modifier)
		 * t_i*x_i + [sum penalty_ij*xmiss_ij]  (added parts between [])
		 * 
		 */
		for(Iterator<LBlock *> lbloc(idg->visit()); lbloc; lbloc++) {
			if(lbloc->id() != 0 && lbloc->id() != idg->count()- 1) {
				int hit_time = lbloc->countInsts(/*cach*/) * fw->platform()->pipelineDepth();
				int miss_time = hit_time + cach->missPenalty();
				// this fuction compute  chit & cmiss with 5 cycles and return the
				//number of inst in thr l-bloc with cache as parametre
  				int counter = lbloc->countInsts(/*cach*/);
  				system->addObjectFunction(
  					cach->missPenalty(),
  					lbloc->use<ilp::Var *>(CCGBuilder::ID_MissVar));
  					/*
  				system->addObjectFunction(
  					miss_time,
  					lbloc->use<ilp::Var *>(CCGBuilder::ID_MissVar));
  					*/
					
			}
		
		}
		
	}
}


}//otawa

