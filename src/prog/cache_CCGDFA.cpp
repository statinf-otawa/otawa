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

using namespace otawa::ilp;
using namespace elm::genstruct;

namespace otawa {


DFASet *CCGDFA::initial(void){
	int length = ccggraph->returnCOUNTER();
	return (new DFABitSet(length));
	}

int CCGDFA::vars = 0;
	
DFASet *CCGDFA::generate(BasicBlock *bb) {
	int length = ccggraph->returnCOUNTER();
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
			for (Iterator<LBlock *> lbloc(ccggraph->visitLBLOCK()); lbloc; lbloc++){
				address_t address = lbloc->addressLBLOCK();
				if ((adlbloc == address)&&(lbloc->blockbasicLBLOCK()== bb))
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

DFASet *CCGDFA::kill(BasicBlock *bb) {
	
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
				for (Iterator<LBlock *> lbloc(ccggraph->visitLBLOCK()); lbloc; lbloc++){
				if ((adlbloc == (lbloc->addressLBLOCK()))&&(bb == lbloc->blockbasicLBLOCK())){
					 testnotconflit = true; 
					identif1 = lbloc->identificateurLBLOCK();					
					unsigned long tag = ((unsigned long)adlbloc) >> dec;
					for (Iterator<LBlock *> lbloc1(ccggraph->visitLBLOCK()); lbloc1; lbloc1++){
						unsigned long taglblock = ((unsigned long)lbloc1->addressLBLOCK()) >> dec;
						address_t faddress = lbloc1->addressLBLOCK();
						if (adlbloc != (lbloc1->addressLBLOCK())&&(tag == taglblock)&&(bb != lbloc1->blockbasicLBLOCK())){
						//if ((adlbloc != faddress )&&(cach->tag(adlbloc) == cach->tag(faddress))&&(bb != lbloc1->blockbasicLBLOCK())){    	
						    	identnonconf = lbloc1->identificateurLBLOCK();
							// the state of the first lblock in BB become nonconflict
							   LBlock *ccgnode = ccggraph->returnLBLOCK(identif1); 
								//ccgnode->changeSTATENONCONF(true);
								break;
						}// end Sde if
					}//end Sde for of lbloc
					break;
				}//end first if
				
				}// end first for of lblock
				//testnotconflit = true;
				    visit = true; 
			}//end first if of testnonconf
			
							
			if (!visit){
			for (Iterator<LBlock *> lbloc(ccggraph->visitLBLOCK()); lbloc; lbloc++){
				if ((adlbloc == (lbloc->addressLBLOCK()))&&(bb != lbloc->blockbasicLBLOCK())){
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
	int length = ccggraph->returnCOUNTER();
	bool ens = true;
	DFABitSet *kill;
	if (identif1 == 0) 
		kill = new DFABitSet(length );
	else
	 kill = new DFABitSet(length,ens);
   return kill ;
}
void CCGDFA::clear(DFASet *set){
	DFABitSet *reset;
	reset = (DFABitSet *)set;
	reset->empty();
}
void CCGDFA::merge(DFASet *acc, DFASet *set){
	DFABitSet *bitacc = (DFABitSet *)acc;
	bitacc->add(set);
}
void CCGDFA::addCCGEDGES(CFG *cfg ,Identifier *in_id, Identifier *out_id){
		assert(cfg);
		System *system = cfg->get<System *>(IPET::ID_System, 0);
		assert (system);
		int length = ccggraph->returnCOUNTER();
		Inst *inst;
		address_t adinst;			
	    PseudoInst *pseudo;
	    LBlock *aux;
	    
		for(Iterator<BasicBlock *> bb(cfg->bbs()); bb; bb++) {
		  if ((!bb->isEntry())&&(!bb->isExit())){
			DFABitSet *info = bb->use<DFABitSet *>(in_id);
			assert(info);
			bool test = false;
			bool visit;
			for(Iterator<Inst *> inst(bb->visit()); inst; inst++) {
			visit = false;
			pseudo = inst->toPseudo();			
			if(!pseudo){
			adinst = inst->address();				
			for (Iterator<LBlock *> lbloc(ccggraph->visitLBLOCK()); lbloc; lbloc++){
				address_t address = lbloc->addressLBLOCK();
				// the first lblock in the BB it's a conflict
				//if ((adinst == address)&&(!lbloc->returnSTATENONCONF())&& (!test)){
				if ((adinst == address)&& (!test)&&(bb == lbloc->blockbasicLBLOCK())){		
					for (int i = 0; i< length; i++){
						if (info->contains(i)){
							LBlock *lblock = ccggraph->returnLBLOCK(i);
							// naming variables
							StringBuffer buf1;
							//buf1.print("eccg_%lx_%lx(%lx)", lblock->addressLBLOCK(), lbloc->addressLBLOCK(),vars);
							buf1 << "eccg_" << lblock->addressLBLOCK() << "_" << lbloc->addressLBLOCK() << "(" << vars << ")";
							vars = vars + 1;
							String name1 = buf1.toString();
							ilp::Var *arc = system->newVar(name1);
							CCGNode *node = lblock->ccgnode();
							new CCGEdge (node,lbloc->ccgnode(), arc);
						}// end if
					}//end for
					aux = lbloc;
					test = true;
					visit = true;
					// exit from the loop of lbloc
					break;
				}//end if
				
				if ((adinst == address)&&(!visit)&&(bb == lbloc->blockbasicLBLOCK())) {
					// naming variables
					StringBuffer buf3;
					//buf3.print("eccg_%lx_%lx(%lx)", aux->addressLBLOCK(), lbloc->addressLBLOCK(),vars);
					buf3 << "eccg_" << aux->addressLBLOCK() << "_" << lbloc->addressLBLOCK() << "(" << vars << ")";
					vars = vars +1;
					String name3 = buf3.toString();
					ilp::Var *arc = system->newVar(name3);
					new CCGEdge(aux->ccgnode(), lbloc->ccgnode(), arc);
					aux = lbloc;
					// exit from the loop Lbloc
					break;
				}//end if					
				
				}// end for first lbloc
		}//end if
		else if(pseudo->id() == bb->ID)
			break;		
		
		
			}// end for inst
			}// end if the BB its not entry
		}// end for BB
		
	// build edge to LBlock end
	BasicBlock *exit = cfglb->exit();
	LBlock *end = ccggraph->returnLBLOCK(length-1);
	DFABitSet *info = exit->use<DFABitSet *>(in_id);
	for (int i = 0; i< length; i++){
					if (info->contains(i)){
						LBlock *ccgnode1 = ccggraph->returnLBLOCK(i);
						//naming variables
						StringBuffer buf4;
						//buf4.print("eccg_%lx_END", ccgnode1->addressLBLOCK());
						buf4 << "eccg_" << ccgnode1->addressLBLOCK() << "_END";
						String name4 = buf4.toString();
						ilp::Var *arc = system->newVar(name4);
						new CCGEdge (ccgnode1->ccgnode(),end->ccgnode(), arc);
					}//end if
	}//end for
	
	// Build edge from 'S' till 'end'
	LBlock *s = ccggraph->returnLBLOCK(0);
	StringBuffer buf4;
	//buf4.print("eccg_S%lx_END",vars);
	buf4 << "eccg_S" << vars << "_END"; 
	vars = vars + 1;
	String name4 = buf4.toString();
	ilp::Var *arc = system->newVar(name4);
	new CCGEdge (s->ccgnode(),end->ccgnode(), arc);
} 
}// otawa

