	/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	otawa/include/otawa/util/DFA.h -- DFA class implementation.
 */

#include <assert.h>
#include <otawa/cache/categorisation/CATDFA.h>
#include <otawa/cfg.h>
#include <otawa/instruction.h>
#include <otawa/cache/categorisation/CATConstraintBuilder.h>
#include <otawa/cache/LBlockSet.h>
#include <elm/genstruct/HashTable.h>
#include <otawa/cache/categorisation/CATBuilder.h>

using namespace otawa::ilp;
using namespace elm::genstruct;
using namespace otawa::ipet;

namespace otawa {


/**
 */
int CATProblem::vars = 0;


/**
 */
CATDomain *CATProblem::gen(CFG *cfg, BasicBlock *bb) {
	int length = lines->count();
	CATDomain *bitset = new CATDomain(length);
	if (bb->isEntry()){
		bitset->add(0);
		return bitset;
	}
	else {	
		address_t adlbloc;
	    PseudoInst *pseudo;
	    int identif=0;	    
	for(Iterator<Inst *> inst(bb->visit()); inst; inst++) {
		pseudo = inst->toPseudo();			
		if(!pseudo){
			adlbloc = inst->address();				
			for (Iterator<LBlock *> lbloc(lines->visit()); lbloc; lbloc++){
				address_t address = lbloc->address();
				if ((adlbloc == address)&& (bb == lbloc->bb()))
					identif = lbloc->id();
				}
		}
		else if(pseudo->id() == &bb->ID)
			break;
	}// end for
	
	 if (identif != 0)bitset->add(identif);
	return bitset;
	}

}

/**
 */
CATDomain *CATProblem::preserve(CFG *cfg, BasicBlock *bb) {
		Inst *inst;
		bool testnotconflit = false;
		bool visit = false;
		address_t adlbloc;
	    PseudoInst *pseudo;
	    int identif1 = 0 , identnonconf = 0 , identif2 = 0;
	for(Iterator<Inst *> inst(bb->visit()); inst; inst++) {
		visit = false;
		// decallage x where each block containts 2^x ocets
		int dec = cach->blockBits();
		pseudo = inst->toPseudo();
		if(!pseudo){
			adlbloc = inst->address();
			if (!testnotconflit){
				// lblocks iteration
				for (Iterator<LBlock *> lbloc(lines->visit()); lbloc; lbloc++){
				if ((adlbloc == (lbloc->address()))&&(bb == (lbloc->bb()))){
					//testnotconflit = true;
					identif1 = lbloc->id();					
					unsigned long tag = ((unsigned long)adlbloc) >> dec;
					for (Iterator<LBlock *> lbloc1(lines->visit()); lbloc1; lbloc1++){
						unsigned long taglblock = ((unsigned long)lbloc1->address()) >> dec;
						if (adlbloc != (lbloc1->address())&&(tag == taglblock)){
						    	identnonconf = lbloc1->id();
							// the state of the first lblock in BB become nonconflict
							   LBlock *ccgnode = lines->lblock(identif1); 
//								ccgnode->setNonConflictState(true);
								CATBuilder::NON_CONFLICT(ccgnode) = true;
								break;
						}// end Sde if
					}//end Sde for of lbloc
					break;
				}//end first if
				
				}// end first for of lblock
				testnotconflit = true;
				    visit = true; 
			}//end first if of testnonconf
			
							
			if (!visit){
			for (Iterator<LBlock *> lbloc(lines->visit()); lbloc; lbloc++){
				if (adlbloc == (lbloc->address())){
				identif2 = lbloc->id();
				break ;
				}// end Sde if
			}// end Sde for
			}//end first if 
				
		}// end if
		else if(pseudo->id() == &bb->ID)
			break;
	}// end for
	// the bit vector of kill
	int length = lines->count();
	
	CATDomain *kill;
	kill = new CATDomain(length);
	
	if (!((identif1 == 0) && (identif2 == 0))) {
	        kill->fill();
	        if ((identif1 != 0)&&(identif2 == 0)&&(identnonconf != 0))
		        kill->remove(identnonconf);
        }
   kill->complement();
   return kill ;
}



}// otawa
