/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	otawa/test/LBlock.cpp -- implementation of LBlock class.
 */
#include <otawa/cache/LBlock.h>
#include <otawa/cache/LBlockSet.h>
#include <otawa/cfg/BasicBlock.h>


namespace otawa {



LBlock::LBlock(LBlockSet *graphe, address_t head, BasicBlock *bb, ilp::Var *hit1,
				ilp::Var *miss1, ilp::Var * xi1, string tp){
		ident = graphe->LBlockSet::addLBLOCK(this);
		lblc = head;
		bblblock = bb;
		hit = hit1;
		miss = miss1;
		xi = xi1;
		nonconflit= false;
		if (tp == "ccg")ccgnod = new CCGNode(this);				
 		if (tp == "cat")catnod = new CATNode(this);
 }
int LBlock::countLBINTRUCTION( int cycle, const Cache *cach){
	
	int cnt = 0;
	if (bblblock != 0){
		PseudoInst *pseudo;
		int m = cach->blockBits();
		unsigned long taglbloc = ((unsigned long )lblc) >> m;
		for(Iterator<Inst *> instr(bblblock->visit()); instr; instr++) {
			pseudo = instr->toPseudo();
			if(!pseudo){
				unsigned long taginst = ((unsigned long )instr->address()) >> m;
				if(taginst == taglbloc) cnt++;
				//if ((cach->line(lblc)) == (cach->line(instr->address()))) cnt++;
			}
			else if(pseudo->id() == bblblock->ID)
				break;	
		}	
			chit = cnt * cycle;
			 //10 cycles the caches default
			cmiss = chit +10;
		}
		else{ 
			cnt = 0;
		 	cmiss = 0;
		  	chit =0;
		}
	return cnt;	
 	
 }
ilp::Var *LBlock::varHIT(void){
	return hit;
}

ilp::Var *LBlock::varMISS(void){
	return miss;
}

ilp::Var *LBlock::varBB(void){
	return xi;
}

int LBlock::constCHIT(void){
		return chit;
}
int LBlock::constCMISS(void){
		return cmiss;	
}
void LBlock::changeSTATENONCONF(bool set){
	nonconflit = set;
}
bool LBlock::returnSTATENONCONF(void){
	return nonconflit;
}


int LBlock::identificateurLBLOCK (void){
		return ident;
	}
address_t LBlock::addressLBLOCK(void){
		return lblc;
	}
BasicBlock *LBlock::blockbasicLBLOCK (void) {
	return bblblock;
}

		
} // otawa
