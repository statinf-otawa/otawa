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


using namespace otawa::ilp;
using namespace otawa;



namespace otawa {
	
class LBlockSet;

void CCGObjectFunction::processCFG(FrameWork *fw, CFG *cfg ) {
	assert(cfg);
	System *system = cfg->get<System *>(IPET::ID_System, 0);
	assert (system);
	LBlockSet *idg = cfg->use<LBlockSet *>(LBlockSet::ID_LBlockSet);
	const Cache *cach = fw->caches().get(0);
	
	// Building the object function which used by S. Malik 
	for (Iterator<LBlock *> lbloc(idg->visitLBLOCK()); lbloc; lbloc++){
		if((lbloc->identificateurLBLOCK()!=0) &&(lbloc->identificateurLBLOCK()!= (idg->returnCOUNTER()- 1))){
			// this fuction compute  chit & cmiss with 5 cycles and return the
			//number of inst in thr l-bloc with cache as parametre
  			int counter = lbloc->countLBINTRUCTION(5,cach);
  			system->addObjectFunction(lbloc->constCHIT(),lbloc->varHIT());
  			system->addObjectFunction(lbloc->constCMISS(),lbloc->varMISS());
					
		}// end if (without S end end)	
		
		}
	}
}//otawa

