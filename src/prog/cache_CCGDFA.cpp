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
			else if(pseudo->id() == bb->ID)
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
		
		else if(pseudo->id() == bb->ID)
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

/**
 */
void CCGDFA::addCCGEDGES(CFG *cfg ,Identifier *in_id, Identifier *out_id){
	assert(cfg);
	System *system = cfg->get<System *>(IPET::ID_System, 0);
	assert (system);
	int length = ccggraph->count();
	Inst *inst;
	address_t adinst;			
    PseudoInst *pseudo;
    LBlock *aux;
	    
	for(Iterator<BasicBlock *> bb(cfg->bbs()); bb; bb++) {
		if(!bb->isEntry() && !bb->isExit()) {
			DFABitSet *info = bb->use<DFABitSet *>(in_id);
			assert(info);
			bool test = false;
			bool visit;
			for(Iterator<Inst *> inst(bb->visit()); inst; inst++) {
				visit = false;
				pseudo = inst->toPseudo();			
				if(!pseudo){
					adinst = inst->address();				
					for (Iterator<LBlock *> lbloc(ccggraph->visit()); lbloc; lbloc++){
						address_t address = lbloc->address();
						// the first lblock in the BB it's a conflict
						//if ((adinst == address)&&(!lbloc->returnSTATENONCONF())&& (!test)){
						if(adinst == address && !test && bb == lbloc->bb()) {		
							for (int i = 0; i< length; i++) {
								if (info->contains(i)) {
									LBlock *lblock = ccggraph->lblock(i);
									// naming variables
									StringBuffer buf1;
									buf1 << "eccg_" << lblock->address() << "_" << lbloc->address() << "(" << vars << ")";
									vars = vars + 1;
									String name1 = buf1.toString();
									ilp::Var *arc = system->newVar(name1);
									CCGNode *node = lblock->ccgNode();
									new CCGEdge (node, lbloc->ccgNode(), arc);
								}
							}
							aux = lbloc;
							test = true;
							visit = true;
							break;
						}
				
						if(adinst == address && !visit && bb == lbloc->bb()) {
							// naming variables
							StringBuffer buf3;
							buf3 << "eccg_" << aux->address() << "_" << lbloc->address() << "(" << vars << ")";
							vars = vars +1;
							String name3 = buf3.toString();
							ilp::Var *arc = system->newVar(name3);
							new CCGEdge(aux->ccgNode(), lbloc->ccgNode(), arc);
							aux = lbloc;
							break;
						}
					}
				}
				else if(pseudo->id() == bb->ID)
					break;		
			}
		}
	}
		
	// build edge to LBlock end
	BasicBlock *exit = cfglb->exit();
	LBlock *end = ccggraph->lblock(length-1);
	DFABitSet *info = exit->use<DFABitSet *>(in_id);
	for (int i = 0; i< length; i++){
		if (info->contains(i)){
			LBlock *ccgnode1 = ccggraph->lblock(i);
			//naming variables
			StringBuffer buf4;
			buf4 << "eccg_" << ccgnode1->address() << "_END";
			String name4 = buf4.toString();
			ilp::Var *arc = system->newVar(name4);
			new CCGEdge (ccgnode1->ccgNode(),end->ccgNode(), arc);
		}
	}
	
	// Build edge from 'S' till 'end'
	LBlock *s = ccggraph->lblock(0);
	StringBuffer buf4;
	buf4 << "eccg_S" << vars << "_END"; 
	vars = vars + 1;
	String name4 = buf4.toString();
	ilp::Var *arc = system->newVar(name4);
	new CCGEdge (s->ccgNode(),end->ccgNode(), arc);
}

}// otawa

