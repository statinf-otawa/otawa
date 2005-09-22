/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	otawa/test/LBlock.cpp -- implementation of LBlock class.
 */

#include <assert.h>
#include <otawa/cache/LBlock.h>
#include <otawa/cache/LBlockSet.h>
#include <otawa/cfg/BasicBlock.h>

namespace otawa {

/**
 * Build a new LBlock.
 */
LBlock::LBlock(LBlockSet *graphe, address_t head, BasicBlock *bb, ilp::Var *hit1,
ilp::Var *miss1, ilp::Var * xi1, elm::String tp) {
	
	ident = graphe->LBlockSet::add(this);
	lblc = head;
	bblblock = bb;
	hit = hit1;
	miss = miss1;
	xi = xi1;
	nonconflit= false;
	if (tp == "ccg")
		ccgnod = new CCGNode(this);				
 	if (tp == "cat")
 		catnod = new CATNode(this);
}

/**
 */
int LBlock::countInsts(int cycle, const Cache *cach) {
	int cnt = 0;
	
	if (bblblock != 0){
		
		PseudoInst *pseudo;
		int m = cach->blockBits();
		unsigned long taglbloc = ((unsigned long )lblc) >> m;
		for(Iterator<Inst *> instr(bblblock->visit()); instr; instr++) {
			pseudo = instr->toPseudo();
			if(!pseudo){
				unsigned long taginst = ((unsigned long )instr->address()) >> m;
				if(taginst == taglbloc)
					cnt++;
			}
			else if(pseudo->id() == bblblock->ID)
				break;	
		}	
		
		chit = cnt * cycle;
		cmiss = chit + 10;
	}
	
	else{ 
		cnt = 0;
	 	cmiss = 0;
	  	chit =0;
	}
	
	return cnt;	
}

/**
 */
ilp::Var *LBlock::hitVar(void){
	return hit;
}

/**
 */
ilp::Var *LBlock::missVar(void){
	return miss;
}

/**
 */
ilp::Var *LBlock::bbVar(void){
	return xi;
}

/**
 */
int LBlock::hitCount(void){
		return chit;
}

/**
 */
int LBlock::missCount(void){
		return cmiss;	
}

/**
 */
void LBlock::setNonConflictState(bool set){
	nonconflit = set;
}

/**
 */
bool LBlock::getNonConflictState(void){
	return nonconflit;
}

/**
 */
int LBlock::id(void){
	return ident;
}

/**
 */
address_t LBlock::address(void){
	return lblc;
}

/**
 */
BasicBlock *LBlock::bb(void) {
	return bblblock;
}

} // otawa
