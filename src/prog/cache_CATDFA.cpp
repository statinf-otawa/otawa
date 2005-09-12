	/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	otawa/include/otawa/cache/categorisation/CATDFA.h -- CATDFA class implementation.
 */

#include <assert.h>
#include <otawa/util/DFABitSet.h>
#include <otawa/cache/categorisation/CATDFA.h>
#include <otawa/cfg.h>
#include <otawa/instruction.h>
#include <otawa/cache/categorisation/CATConstraintBuilder.h>
#include <otawa/cache/LBlockSet.h>

using namespace otawa::ilp;

namespace otawa {


DFASet *CATDFA::initial(void){
	int length = lines->returnCOUNTER();
	return (new DFABitSet(length));
	}

	
DFASet *CATDFA::generate(BasicBlock *bb) {
	int length = lines->returnCOUNTER();
	DFABitSet *dfabitset = new DFABitSet(length);
	if (bb->isEntry()){
		dfabitset->DFABitSet::add(0);
		return dfabitset;
	}
	else {	
		address_t adlbloc;
	    PseudoInst *pseudo;
	    int identif=0;	    
	for(Iterator<Inst *> inst(bb->visit()); inst; inst++) {
		pseudo = inst->toPseudo();			
		if(!pseudo){
			adlbloc = inst->address();				
			for (Iterator<LBlock *> lbloc(lines->visitLBLOCK()); lbloc; lbloc++){
				address_t address = lbloc->addressLBLOCK();
				if ((adlbloc == address)&& (bb == lbloc->blockbasicLBLOCK()))
				identif = lbloc->identificateurLBLOCK();
				}
		}
		else if(pseudo->id() == bb->ID)
			break;
	}// end for
	
	 if (identif != 0)dfabitset->DFABitSet::add(identif);
	return dfabitset;
	}

}

DFASet *CATDFA::kill(BasicBlock *bb) {
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
				for (Iterator<LBlock *> lbloc(lines->visitLBLOCK()); lbloc; lbloc++){
				if ((adlbloc == (lbloc->addressLBLOCK()))&&(bb == (lbloc->blockbasicLBLOCK()))){
					//testnotconflit = true;
					identif1 = lbloc->identificateurLBLOCK();					
					unsigned long tag = ((unsigned long)adlbloc) >> dec;
					for (Iterator<LBlock *> lbloc1(lines->visitLBLOCK()); lbloc1; lbloc1++){
						unsigned long taglblock = ((unsigned long)lbloc1->addressLBLOCK()) >> dec;
						if (adlbloc != (lbloc1->addressLBLOCK())&&(tag == taglblock)){
						    	identnonconf = lbloc1->identificateurLBLOCK();
							// the state of the first lblock in BB become nonconflict
							   LBlock *ccgnode = lines->returnLBLOCK(identif1); 
								ccgnode->changeSTATENONCONF(true);
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
			for (Iterator<LBlock *> lbloc(lines->visitLBLOCK()); lbloc; lbloc++){
				if (adlbloc == (lbloc->addressLBLOCK())){
				identif2 = lbloc->identificateurLBLOCK();
				break ;
				}// end Sde if
			}// end Sde for
			}//end first if 
				
		}// end if
		else if(pseudo->id() == bb->ID)
			break;
	}// end for
	// the bit vector of kill
	int length = lines->returnCOUNTER();
	bool ens = true;
	DFABitSet *kill;
	if ((identif1 == 0) && (identif2 == 0)) 
	kill = new DFABitSet(length );
	else if ((identif1 != 0)&&(identif2 == 0)&&(identnonconf != 0)){
		kill = new DFABitSet(length , ens);
		kill->DFABitSet::remove(identnonconf);
	}
	else kill = new DFABitSet(length,ens);
   return kill ;
}
void CATDFA::clear(DFASet *set){
	DFABitSet *reset;
	reset = (DFABitSet *)set;
	reset->empty();
}
void CATDFA::merge(DFASet *acc, DFASet *set){
	DFABitSet *bitacc = (DFABitSet *)acc;
	bitacc->add(set);
}

}




