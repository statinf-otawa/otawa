	/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	otawa/include/otawa/util/DFA.h -- DFA class implementation.
 */

#include <assert.h>
#include <otawa/cache/ccg/CCGDFA.h>
#include <otawa/cfg.h>
#include <otawa/instruction.h>
#include <otawa/cache/ccg/CCGConstraintBuilder.h>
#include <otawa/cache/LBlockSet.h>
#include <elm/genstruct/HashTable.h>
#include <otawa/cache/ccg/CCGBuilder.h>
#include <otawa/util/ContextTree.h>
#include <otawa/proc/CFGProcessor.h>
#include <otawa/prog/WorkSpace.h>
#include <otawa/proc/ProcessorException.h>

using namespace otawa::ilp;
using namespace elm::genstruct;
using namespace otawa::ipet;

namespace otawa {


/**
 */
int CCGProblem::vars = 0;


/**
 */
CCGDomain *CCGProblem::gen(CFG *cfg, BasicBlock *bb) {
	/*int length =*/ ccggraph->count();
	CCGDomain *bitset = empty();
	if(bb->isEntry() && (cfg == ENTRY_CFG(fw))) {
		bitset->add(0);
		return bitset;
	}
	else {	
		address_t adlbloc;
	    PseudoInst *pseudo;
	    int identif = 0;	    
		for(Iterator<Inst *> inst(bb->visit()); inst; inst++) {
			pseudo = inst->toPseudo();			
			if(!pseudo){
				adlbloc = inst->address();				
				for (Iterator<LBlock *> lbloc(ccggraph->visit()); lbloc; lbloc++) {
					address_t address = lbloc->address();
					if(adlbloc == address && lbloc->bb()== bb)
						identif = lbloc->id();
				}
			}
			else if(pseudo->id() == &bb->ID)
				break;
		}
	
		if(identif != 0)
		 	bitset->add(identif);
		return bitset;
	}

}

/**
 */
CCGDomain *CCGProblem::preserve(CFG *cfg, BasicBlock *bb) {
	//Inst *inst;
	bool testnotconflit = false;
	bool visit = false;
	address_t adlbloc;
   PseudoInst *pseudo;
    int identif1 = 0 , identnonconf = 0 , identif2 = 0;
    
	for(Iterator<Inst *> inst(bb->visit()); inst; inst++) {
		visit = false;
		int dec = cach->blockBits();
		
		pseudo = inst->toPseudo();
		if(!pseudo) {
			adlbloc = inst->address();
			if (!testnotconflit) {
				
				// lblocks iteration
				for (Iterator<LBlock *> lbloc(ccggraph->visit()); lbloc; lbloc++) {
					if (adlbloc == lbloc->address() && bb == lbloc->bb()) {
						testnotconflit = true; 
						identif1 = lbloc->id();					
						unsigned long tag = ((unsigned long)adlbloc) >> dec;
						for(Iterator<LBlock *> lbloc1(ccggraph->visit()); lbloc1; lbloc1++) {
							unsigned long taglblock = ((unsigned long)lbloc1->address()) >> dec;
							address_t faddress = lbloc1->address();
							if(adlbloc != lbloc1->address() && tag == taglblock
							&& bb != lbloc1->bb()) {
								identnonconf = lbloc1->id();
								/*LBlock *ccgnode =*/ ccggraph->lblock(identif1); 
								break;
							}
						}
						break;
					}
				
				}
				visit = true; 
			}
							
			if (!visit) {
				for (Iterator<LBlock *> lbloc(ccggraph->visit()); lbloc; lbloc++) {
					if(adlbloc == lbloc->address() && bb != lbloc->bb()) {
						identif2 = lbloc->id();
						break ;
					}
				}
			}
				
		}
		
		else if(pseudo->id() == &bb->ID)
			break;
	}
	
	// the bit vector of kill
	/*int length =*/ ccggraph->count();
	CCGDomain *kill;
	kill = empty();
	if(identif1 == 0)  {
		kill->fill();
	}
	return kill;
}



}// otawa

