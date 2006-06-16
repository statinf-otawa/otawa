	/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	otawa/include/otawa/util/DFA.h -- DFA class implementation.
 */

#include <assert.h>
#include <otawa/util/DFABitSet.h>
#include <otawa/cache/ccg/CCGDFA.h>
#include <otawa/cfg.h>
#include <otawa/instruction.h>
#include <otawa/cache/ccg/CCGConstraintBuilder.h>
#include <otawa/cache/LBlockSet.h>
#include <elm/genstruct/HashTable.h>
#include <otawa/cache/ccg/CCGBuilder.h>

using namespace otawa::ilp;
using namespace elm::genstruct;
using namespace otawa::ipet;

namespace otawa {

/**
 */
DFASet *CCGDFA::initial(void){
	int length = ccggraph->count();
	return new DFABitSet(length);
}


/**
 */
int CCGDFA::vars = 0;


/**
 */
DFASet *CCGDFA::generate(BasicBlock *bb) {
	int length = ccggraph->count();
	DFABitSet *dfabitset = new DFABitSet(length);
	if(bb->isEntry()){
		dfabitset->DFABitSet::add(0);
		return dfabitset;
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
		 	dfabitset->DFABitSet::add(identif);
		return dfabitset;
	}

}

/**
 */
DFASet *CCGDFA::kill(BasicBlock *bb) {
	Inst *inst;
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
								LBlock *ccgnode = ccggraph->lblock(identif1); 
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
	int length = ccggraph->count();
	bool ens = true;
	DFABitSet *kill;
	if(identif1 == 0) 
		kill = new DFABitSet(length);
	else
		kill = new DFABitSet(length, ens);
	return kill ;
}

/**
 */
void CCGDFA::clear(DFASet *set){
	DFABitSet *reset;
	reset = (DFABitSet *)set;
	reset->empty();
}

/**
 */
void CCGDFA::merge(DFASet *acc, DFASet *set) {
	DFABitSet *bitacc = (DFABitSet *)acc;
	bitacc->add(set);
}

}// otawa

