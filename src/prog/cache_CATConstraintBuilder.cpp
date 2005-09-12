/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	otawa/src/prog/cache_CATConstraintsBuilder.cpp -- CATConstraintsBuilder class implementation.
 */
#include <stdio.h>
#include <elm/io.h>
#include <otawa/cache/categorisation/CATConstraintBuilder.h>
#include <otawa/instruction.h>
#include <otawa/cache/categorisation/CATNode.h>
#include <otawa/cache/categorisation/CATDFA.h>
#include <otawa/cache/LBlockSet.h>
#include <otawa/ilp.h>
#include <otawa/ipet/IPET.h>
#include <elm/Collection.h>
#include <otawa/util/ContextTree.h>
#include <elm/genstruct/HashTable.h>
#include <otawa/util/Dominance.h>
#include <otawa/cfg.h>

using namespace otawa;
using namespace otawa::ilp;
using namespace elm::genstruct;


namespace otawa {


Identifier CATConstraintBuilder::ID_In("ipet.dfain");

Identifier CATConstraintBuilder::ID_Out("ipet.dfaout");

Identifier CATConstraintBuilder::ID_Set("ipet.lbset");

Identifier CATConstraintBuilder::ID_Cat("ipet.categorization");

int CATConstraintBuilder::counter = 0;

void CATConstraintBuilder::processCFG(FrameWork *fw, CFG *cfg ) {
	assert(cfg);
	ilp::System *system = cfg->get<System *>(IPET::ID_System, 0);
	assert (system);
	LBlockSet *id = cfg->use<LBlockSet *>(LBlockSet::ID_LBlockSet);
	
	// cache configuration
		const Cache *cach = fw->caches().get(0);
	
	// decallage x where each block containts 2^x ocets
		int dec = cach->blockBits();
	
	CATDFA dfa(id,cfg, cach);
	
	cout<<" starting DFA \n";
	// DFA prossecing
	dfa.DFA::resolve(cfg,&ID_In,&ID_Out);
	cout << " DFA has constructed \n";

	ContextTree *ct = new ContextTree(cfg);
	cout << " Context Tree has constructed " <<"\n";
	cout << "buit the l-block sets u of the loops \n";
	DFABitSet *virtuel = buildLBLOCKSET(id,system,ct);
	cout << "the l-block sets have constructed " <<"\n";
	// set the categorisation of each lblock based on C. Healy
	setCATEGORISATION(id,ct,dec);
	cout << "the catégorizations have constructed " <<"\n";

	int length = id->returnCOUNTER();	
	for (Iterator<LBlock *> bloc(id->visitLBLOCK()); bloc; bloc++){
		int test = bloc->identificateurLBLOCK();
		if ((test != 0)&&(test != (length-1))) {	
		Categorization_t categorie = bloc->use<Categorization_t>(ID_Cat);
		Constraint *cons;
		
		if (categorie == ALWAYSHIT){
			cons = system->newConstraint(Constraint::EQ,0);
			cons->add(1, bloc->varMISS());
		}
		if (categorie == FIRSTHIT){
			BasicBlock *bb = bloc->blockbasicLBLOCK();
			cons = system->newConstraint(Constraint::EQ);
			cons->addLeft(1, bloc->varHIT());
			bool used = false;
			for(Iterator<Edge *> edge(bb->inEdges()); edge; edge++) {
				if (!Dominance::dominates(bb, edge->source())){
					cons->addRight(1, edge->use<ilp::Var *>(IPET::ID_Var));
					used = true;
				}
			}
			if(!used)
				delete cons;
		
		
		Constraint *cons2 = system->newConstraint(Constraint::EQ);
		cons2->addLeft(1, bloc->varBB());
		cons2->addRight(1, bloc->varHIT());
		cons2->addRight(1, bloc->varMISS());		
		}
		
		if (categorie == FIRSTMISS){
			cons = system->newConstraint(Constraint::EQ,1);
			cons->add(1, bloc->varMISS());
		}
		if (categorie == ALWAYSMISS){
			if (bloc->catnode()->INLOOP()){
				if (bloc->catnode()->HASHEADEREVOLUTION()){
					Constraint *cons32 = system->newConstraint(Constraint::LE);
					cons32->addLeft(1, bloc->varMISS());
					ilp::Var *x = (ilp::Var *)bloc->catnode()->HEADEREVOLUTION()->use<Var *>(IPET::ID_Var);
					cout << bloc->catnode()->HEADEREVOLUTION();
					cons32->addRight(1, x);
				//}
				Constraint * boundingmiss = system->newConstraint(Constraint::LE);
				boundingmiss->addLeft(1, bloc->varMISS());
				for(Iterator<Edge *> entry(bloc->catnode()->HEADERLBLOCK()->inEdges()); entry; entry++) {
					if (!Dominance::dominates(bloc->catnode()->HEADERLBLOCK(), entry->source())){
							boundingmiss->addRight(1, entry->use<ilp::Var *>(IPET::ID_Var));
							
					}
					
					}
				}
				else {
				cons = system->newConstraint(Constraint::EQ);
				cons->addLeft(1, bloc->varMISS());
				cons->addRight(1, bloc->varBB());
								
				}
			}
			else{
				cons = system->newConstraint(Constraint::EQ);
				cons->addLeft(1, bloc->varMISS());
				cons->addRight(1, bloc->varBB());
			}
		}
		/*
		 this fuction compute  chit & cmiss with 5 cycles trivial execition
		 and return the number of instructions in the l-block with cache as parametre
		*/
		int counter = bloc->countLBINTRUCTION(5,cach);
		int latence = bloc->constCMISS() - bloc->constCHIT();
  		system->addObjectFunction(latence,bloc->varMISS());
			}
		}
}
	


DFABitSet *CATConstraintBuilder::buildLBLOCKSET(LBlockSet *lcache , System *system ,ContextTree *root){
		int lcount = lcache->returnCOUNTER();	
		DFABitSet *set = new DFABitSet(lcount);
		DFABitSet *v = new DFABitSet(lcount);
		bool inloop = false;
		if (root->kind()!= ContextTree::ROOT )
				inloop = true;
		int ident;
	
		for(Iterator<ContextTree *> son(root->children()); son; son++){
			 v = buildLBLOCKSET(lcache,system,son);
			 set->add(v);
		}
		
		for(Iterator<BasicBlock *> bb(root->bbs()); bb; bb++){
			if ((!bb->isEntry())&&(!bb->isExit())){
			for(Iterator<Inst *> inst(bb->visit()); inst; inst++) {
				 PseudoInst *pseudo = inst->toPseudo();
				if(!pseudo){
					address_t adlbloc = inst->address();
					for (Iterator<LBlock *> lbloc(lcache->visitLBLOCK()); lbloc; lbloc++){
						if ((adlbloc == (lbloc->addressLBLOCK()))&&(bb == lbloc->blockbasicLBLOCK())){
							ident = lbloc->identificateurLBLOCK();
							lbloc->add<Categorization_t >(ID_Cat,CATConstraintBuilder::INVALID);
							lbloc->catnode()->setHEADERLBLOCK(root->bb(),inloop);
							set->DFABitSet::add(ident);
							
						}
					}
				}
				else if(pseudo->id() == bb->ID)
					break;
			}
		}
		}

	if(root->kind()!= ContextTree::ROOT){
		root->add<DFABitSet *>(ID_Set, set); 
	}
	return set;
}

void CATConstraintBuilder::setCATEGORISATION(LBlockSet *lineset ,ContextTree *S ,int dec){
	int size = lineset->returnCOUNTER();	
	int ident;
	DFABitSet *u = new DFABitSet(size);
	LBlock *cachelin;
	for(Iterator<ContextTree *> fils(S->children()); fils; fils++){
		setCATEGORISATION(lineset,fils,dec);		
	}
	if(S->kind() == ContextTree::LOOP){
		u = S->use<DFABitSet *>(ID_Set);
		for (int a = 0; a < size; a++){
			if (u->contains(a)){
				cachelin = lineset->returnLBLOCK(a);
				worst(cachelin ,S,lineset,dec);
			}
		}
	}
	else {
		for(Iterator<BasicBlock *> bk(S->bbs()); bk; bk++){
			for(Iterator<Inst *> inst(bk->visit()); inst; inst++) {
				 PseudoInst *pseudo = inst->toPseudo();
				if(!pseudo){
					address_t adlbloc = inst->address();
					for (Iterator<LBlock *> lbloc(lineset->visitLBLOCK()); lbloc; lbloc++){
						if ((adlbloc == (lbloc->addressLBLOCK()))&&(bk == lbloc->blockbasicLBLOCK())){
							ident = lbloc->identificateurLBLOCK();
							cachelin = lineset->returnLBLOCK(ident);
							worst(cachelin ,S , lineset,dec);
						}
					}
				}
				else if(pseudo->id() == bk->ID)
					break;
			}
		}
		
	}
}
void CATConstraintBuilder::worst(LBlock *line , ContextTree *node , LBlockSet *idset, int dec){
	int number = idset->returnCOUNTER();	
	BasicBlock *bb = line->blockbasicLBLOCK();
	LBlock *cacheline;
	DFABitSet *in = new DFABitSet(number);
	in = bb->use<DFABitSet *>(ID_In);

	int count = 0;
	bool nonconflitdetected = false;
	bool continu = false;
	unsigned long tagcachline,tagline;

	//test if it's the lbloc which find in the same memory block
	if (in->counttruebit() == 1){
		for (int i=0;i < number;i++){
		if (in->contains(i)){
			cacheline = idset->returnLBLOCK(i);
			tagcachline = ((unsigned long)cacheline->addressLBLOCK()) >> dec;
			unsigned long tagline = ((unsigned long)line->addressLBLOCK()) >> dec;
				if (tagcachline == tagline )
					nonconflitdetected = true;
			}
		}
	}
	
	
	
	//test the virtual non-conflit state
	bool nonconflict = false;
	for (int i=0;i < number;i++){
			if (in->contains(i)){
			cacheline = idset->returnLBLOCK(i);
			tagcachline = ((unsigned long)cacheline->addressLBLOCK()) >> dec;
		    tagline = ((unsigned long)line->addressLBLOCK()) >> dec;
			if ((cacheline->addressLBLOCK() == line->addressLBLOCK())&&(line->blockbasicLBLOCK()!=cacheline->blockbasicLBLOCK()))
											nonconflict = true;
			}
	}
	
	
	bool exist = false;
	// test if the first lblock in the header is Always miss
	if (in->counttruebit()== 2){
		bool test = false;
		for (int i=0;i < number;i++){
			if (in->contains(i)){
			cacheline = idset->returnLBLOCK(i);
			tagcachline = ((unsigned long)cacheline->addressLBLOCK()) >> dec;
			unsigned long tagline = ((unsigned long)line->addressLBLOCK()) >> dec;
			//if ((tagcachline == tagline )&&(line->addressLBLOCK() != cacheline->addressLBLOCK()))
			if ((tagcachline == tagline )&&(line->blockbasicLBLOCK() != cacheline->blockbasicLBLOCK()))
				test = true;
			if (tagcachline != tagline )
							exist = true;
			if ((test)&&(line->addressLBLOCK() == cacheline->addressLBLOCK()))
			//if ((test)&&(line->blockbasicLBLOCK() == cacheline->blockbasicLBLOCK()))
					continu = true;
			
			}
		}
	}
	
	//the last categorisation of the l-block
	Categorization_t lastcateg = line->use<Categorization_t>(ID_Cat);
	if(node->kind()==ContextTree::ROOT){
		//if ((line->returnSTATENONCONF())&&( nonconflitdetected)){
		if ( nonconflitdetected || continu){	
			line->set<Categorization_t>(ID_Cat,CATConstraintBuilder::ALWAYSHIT);
		}			
		else
			line->set<Categorization_t>(ID_Cat,CATConstraintBuilder::ALWAYSMISS);	
	}
	bool dfm = false;
	if(node->kind()!= ContextTree::ROOT){
		DFABitSet *w = new DFABitSet(number);
		w = node->use<DFABitSet *>(ID_Set);
		if (w->counttruebit()== 2){
			int U[2];
			int m = 0;
			for (int i=0;i < w->size();i++){
				if (w->contains(i)){
					U[m] = i;
					m = m+1; 
				}
			}
			//cacheline = lineset->returnLBLOCK(i);
			tagcachline = ((unsigned long)idset->returnLBLOCK(U[0])->addressLBLOCK()) >> dec;
		    tagline = ((unsigned long)idset->returnLBLOCK(U[1])->addressLBLOCK()) >> dec;
			if (tagcachline == tagline )
					dfm = true;
			}
		
		DFABitSet inter = *w;
		DFABitSet dif = *in;
	
		// intersection
		inter.mask(in);
	
		//difference (IN - U)
		dif.dif(w);
		
		int identif = line->identificateurLBLOCK();
		
		//basic bock of the header
		BasicBlock * blockheader = node->bb();
		//if ((line->returnSTATENONCONF())&&(nonconflitdetected || continu)){
		if ((nonconflitdetected )|| (continu && !exist)){
			line->set<Categorization_t>(ID_Cat,CATConstraintBuilder::ALWAYSHIT);
		}
		else 
			if ((lastcateg == CATConstraintBuilder::FIRSTHIT)
			||(((line->returnSTATENONCONF())|| (nonconflict))&&(inter.counttruebit() > 0)&&(dif.counttruebit()== 1)&&(blockheader == bb)))
				line->set<Categorization_t>(ID_Cat,CATConstraintBuilder::FIRSTHIT);
			else if ((((lastcateg == CATConstraintBuilder::FIRSTMISS) || (lastcateg == CATConstraintBuilder::INVALID))
				&&(in->contains(identif)&&(inter.counttruebit()== 1)&&(inter.contains(identif))&& (dif.counttruebit() >= 0)&&(w->counttruebit()== 1)))
				||((inter.counttruebit() == 2)&&(dfm)&&(inter.contains(identif))))
					line->set<Categorization_t>(ID_Cat,CATConstraintBuilder::FIRSTMISS);
				else {
					line->set<Categorization_t>(ID_Cat,CATConstraintBuilder::ALWAYSMISS);
					 if (lastcateg == CATConstraintBuilder::FIRSTMISS){
					 				line->catnode()->setHEADEREVOLUTION(node->bb(),true);
					 }
					 
				}
		}
		
}


}//otawa






