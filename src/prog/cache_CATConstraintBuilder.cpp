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
#include <otawa/cfg/CFGCollector.h>
#include <otawa/cache/categorisation/CATDFA.h>
#include <otawa/cache/LBlockSet.h>
#include <otawa/ilp.h>
#include <otawa/ipet/IPET.h>
#include <elm/Collection.h>
#include <otawa/util/ContextTree.h>
#include <elm/genstruct/HashTable.h>
#include <otawa/util/Dominance.h>
#include <otawa/cfg.h>
#include <otawa/hard/CacheConfiguration.h>
#include <otawa/cache/categorisation/CATBuilder.h>
#include <otawa/hard/Platform.h>
#include <otawa/dfa/XIterativeDFA.h>
#include <otawa/dfa/XCFGVisitor.h>

using namespace otawa;
using namespace otawa::ilp;
using namespace elm::genstruct;
using namespace otawa::ipet;


namespace otawa {

Identifier CATConstraintBuilder::ID_In("ipet.cat.dfain");

Identifier CATConstraintBuilder::ID_Out("ipet.cat.dfaout");

Identifier CATConstraintBuilder::ID_Set("ipet.cat.lbset");

Identifier CATConstraintBuilder::ID_Cat("ipet.cat.categorization");

int CATConstraintBuilder::counter = 0;


/**
 */
void CATConstraintBuilder::processLBlockSet(FrameWork *fw, LBlockSet *id ) {

	ilp::System *system = getSystem(fw, ENTRY_CFG(fw));
	assert (system);
	
	// cache configuration
		const hard::Cache *cach = fw->platform()->cache().instCache();
	
	// decallage x where each block containts 2^x ocets
		int dec = cach->blockBits();
	


        CATProblem prob(id, id->count(), cach, fw);
        CFGCollection *coll = INVOLVED_CFGS(fw);
        dfa::XCFGVisitor<CATProblem> visitor(*coll, prob);
        dfa::XIterativeDFA<dfa::XCFGVisitor<CATProblem> > engine(visitor);
        engine.process();
	
	
	//cout<<" starting DFA \n";
	// DFA prossecing
	//cout << " DFA has constructed \n";
	// Add the annotations from the DFA result
	
	for (CFGCollection::Iterator cfg(coll); cfg; cfg++) {
		for (CFG::BBIterator block(*cfg); block; block++) {
			dfa::XCFGVisitor<CATProblem>::key_t pair(*cfg, *block);
			BitSet *bitset = engine.in(pair);
			block->addDeletable<BitSet *>(ID_In, new BitSet(*bitset));
		}
	}
	
	

	
	ContextTree *ct = new ContextTree(ENTRY_CFG(fw));
	//cout << " Context Tree has constructed " <<"\n";
	//cout << "buit the l-block sets u of the loops \n";
	BitSet *virtuel = buildLBLOCKSET(id,system,ct);
	//cout << "the l-block sets have constructed " <<"\n";
	// set the categorisation of each lblock based on C. Healy
	setCATEGORISATION(id,ct,dec);
	//cout << "the catégorizations have constructed " <<"\n";

	
	int length = id->count();	
	/*
	 * Process each l-block, creating constraints using the l-block's categorisation
	 */
	for (Iterator<LBlock *> bloc(id->visit()); bloc; bloc++){
		int test = bloc->id();
		
		/* Avoid first/last l-block */
		if ((test != 0)&&(test != (length-1))) {	
			Categorization_t categorie = bloc->use<Categorization_t>(ID_Cat);
			Constraint *cons;
		
			/* If ALWAYSHIT, then x_miss(i,j) = 0 */
			if (categorie == ALWAYSHIT){
				cons = system->newConstraint(Constraint::EQ,0);
				cons->add(1, bloc->use<ilp::Var *>(CATBuilder::ID_MissVar));
			}
			/* If FIRSTHIT,
			 * x_hit(i,j) = sum x_egde (for all incoming non-back-edges)
			 * and
			 * xhit(i,j) + xmiss(i,j) = x(i)
			 */
			if (categorie == FIRSTHIT) {
				BasicBlock *bb = bloc->bb();
				cons = system->newConstraint(Constraint::EQ);
				cons->addLeft(1, bloc->use<ilp::Var *>(CATBuilder::ID_HitVar));
				bool used = false;
				for(Iterator<Edge *> edge(bb->inEdges()); edge; edge++) {
					if (!Dominance::dominates(bb, edge->source())){
						cons->addRight(1, edge->use<ilp::Var *>(VAR));
						used = true;
					}
				}
			if(!used)
				delete cons;
		
		
				Constraint *cons2 = system->newConstraint(Constraint::EQ);
				cons2->addLeft(1, bloc->use<ilp::Var *>(CATBuilder::ID_BBVar));
				cons2->addRight(1, bloc->use<ilp::Var *>(CATBuilder::ID_HitVar));
				cons2->addRight(1, bloc->use<ilp::Var *>(CATBuilder::ID_MissVar));		
			}
		
			/*
			 * If FIRSTMISS,
			 * xmiss(i,j) == 1  (?!??)
			 */
			 
			if (categorie == FIRSTMISS){
				cons = system->newConstraint(Constraint::EQ,1);
				cons->add(1, bloc->use<ilp::Var *>(CATBuilder::ID_MissVar));
			}
			if (categorie == ALWAYSMISS){
				if (bloc->use<CATNode *>(CATBuilder::ID_Node)->INLOOP()){
					if (bloc->use<CATNode *>(CATBuilder::ID_Node)->HASHEADEREVOLUTION()){
						/* If lblock in loop / lblock has HEADEREVOLUTION:
						 * x_miss(i,j) <= x_loop_header (??)
						 * x_miss(i,j) <= sum x_edge (for all incoming non-back-edges of the header of the loop containing the current lblock)
						 */
						Constraint *cons32 = system->newConstraint(Constraint::LE);
						cons32->addLeft(1, bloc->use<ilp::Var *>(CATBuilder::ID_MissVar));
						ilp::Var *x = (ilp::Var *)bloc->use<CATNode *>(CATBuilder::ID_Node)->HEADEREVOLUTION()->use<Var *>(VAR);
					//cout << bloc->use<CATNode *>(CATBuilder::ID_Node)->HEADEREVOLUTION();
						cons32->addRight(1, x);
				//}
						Constraint * boundingmiss = system->newConstraint(Constraint::LE);
						boundingmiss->addLeft(1, bloc->use<ilp::Var *>(CATBuilder::ID_MissVar));
						for(Iterator<Edge *> entry(bloc->use<CATNode *>(CATBuilder::ID_Node)->HEADERLBLOCK()->inEdges());
							entry; entry++) {
							if (!Dominance::dominates(bloc->use<CATNode *>(CATBuilder::ID_Node)->HEADERLBLOCK(), entry->source())){
								boundingmiss->addRight(1, entry->use<ilp::Var *>(VAR));
							
							}
					
						}
					}
					else {
						/* xmiss(i,j) = x(i) */
						cons = system->newConstraint(Constraint::EQ);
						cons->addLeft(1, bloc->use<ilp::Var *>(CATBuilder::ID_MissVar));
						cons->addRight(1, bloc->use<ilp::Var *>(CATBuilder::ID_BBVar));
								
					}
				}
				else {
					/* xmiss(i,j) == x(i) */
					cons = system->newConstraint(Constraint::EQ);
					cons->addLeft(1, bloc->use<ilp::Var *>(CATBuilder::ID_MissVar));
					cons->addRight(1, bloc->use<ilp::Var *>(CATBuilder::ID_BBVar));
				}
			}
			/*
			this fuction compute  chit & cmiss with 5 cycles trivial execition
		 	and return the number of instructions in the l-block with cache as parametre
		 	*/
			int counter = bloc->countInsts(/*cach*/);
//			int latence = bloc->missCount() - bloc->hitCount();
			int latence = cach->missPenalty();
  			system->addObjectFunction(latence,bloc->use<ilp::Var *>(CATBuilder::ID_MissVar));
		} /* of if (not a first/last lblock) */
	} /* of for(all lblocks) */
}
	


void CATConstraintBuilder::processFrameWork(FrameWork *fw) {
	assert(fw);
	LBlockSet **lbsets = LBLOCKS(fw);
	const hard::Cache *cache = fw->platform()->cache().instCache();
	
	for(int i = 0; i < cache->lineCount(); i++)
		processLBlockSet(fw, lbsets[i]);
}


/**
 * Annotate all the loop headers with the set of the l-blocks contained in the loop.
 * Sets the categorisation to "INVALID" for all l-blocks processed by this function.
 * Annotate all processed l-blocks with the the header of the most inner loop.
 * (or with the root of the CT. if there isn't any loop containing the l-block)
 *
 * @param lcache The lblock set.
 * @param system The constraints system
 * @param root The root ContextTree
 * @return The set of all lblocks contained in the "root" ContextTree and its children (that is, the set of all processed l-blocks)
 * 
 */
BitSet *CATConstraintBuilder::buildLBLOCKSET(LBlockSet *lcache , System *system ,ContextTree *root){
		int lcount = lcache->count();	
		BitSet *set = new BitSet(lcount);
		BitSet *v = new BitSet(lcount);
		bool inloop = false;
		
		if (root->kind()== ContextTree::LOOP )
				inloop = true;
		int ident;
	
		/*
		 * Call recursively buildLBLOCKSET for each ContextTree children
		 * Merge result with current set
		 */
		for(Iterator<ContextTree *> son(root->children()); son; son++){
			 v = buildLBLOCKSET(lcache,system,son);
			 set->add(*v);
		}
		
		/*
		 * For each lblock that is part of any non-(entry|exit) BB of the current ContextTree:
		 *   - Set the lblock's categorization to INVALID
		 *   - Add this lblock to the current set.
		 */
		for(Iterator<BasicBlock *> bb(root->bbs()); bb; bb++){
			if ((!bb->isEntry())&&(!bb->isExit())){ /* XXX */
			for(Iterator<Inst *> inst(bb->visit()); inst; inst++) {
				 PseudoInst *pseudo = inst->toPseudo();
				if(!pseudo){
					address_t adlbloc = inst->address();
					for (Iterator<LBlock *> lbloc(lcache->visit()); lbloc; lbloc++){
						if ((adlbloc == (lbloc->address()))&&(bb == lbloc->bb())){
							ident = lbloc->id();
							lbloc->add<Categorization_t >(ID_Cat,CATConstraintBuilder::INVALID);
							lbloc->use<CATNode *>(CATBuilder::ID_Node)->setHEADERLBLOCK(root->bb(),inloop);
							set->BitSet::add(ident);
							
						}
					}
				}
				else if(pseudo->id() == &bb->ID)
					break;
			}
		}
		}

	/* 
	 * For loops, annotate the loop-header with the set of all l-blocks in the loop
	 */
	if(root->kind()== ContextTree::LOOP){
		root->add<BitSet *>(ID_Set, set); 
	}
	return set;
}


/*
 * This function categorize the l-blocks in the ContextTree S (and its children)
 *
 * @param lineset The l-block list
 * @param S The root context tree
 * @param dec In an address, the number of bits representing the offset in a cache block.
 */
void CATConstraintBuilder::setCATEGORISATION(LBlockSet *lineset ,ContextTree *S ,int dec){
	int size = lineset->count();	
	int ident;
	BitSet *u = new BitSet(size);
	LBlock *cachelin;
	
	/*
	 * Categorize first all the l-blocks in the children ContextTree
	 */
	for(Iterator<ContextTree *> fils(S->children()); fils; fils++){
		setCATEGORISATION(lineset,fils,dec);		
	}
	
	/* Now categorize the l-blocks in this ContextTree */
	if(S->kind() == ContextTree::LOOP){
		/* 
		 * Call worst() on each l-block of the loop.
		 */
		u = S->use<BitSet *>(ID_Set);
		for (int a = 0; a < size; a++){
			if (u->contains(a)){
				cachelin = lineset->lblock(a);
				worst(cachelin ,S,lineset,dec);
			}
		}
	}
	else {
		/* 
		 * Call worst() on each l-block of this ContextTree.
		 */
		for(Iterator<BasicBlock *> bk(S->bbs()); bk; bk++){
			for(Iterator<Inst *> inst(bk->visit()); inst; inst++) {
				 PseudoInst *pseudo = inst->toPseudo();
				if(!pseudo){
					address_t adlbloc = inst->address();
					for (Iterator<LBlock *> lbloc(lineset->visit()); lbloc; lbloc++){
						if ((adlbloc == (lbloc->address()))&&(bk == lbloc->bb())){
							ident = lbloc->id();
							cachelin = lineset->lblock(ident);
							worst(cachelin ,S , lineset,dec);
						}
					}
				}
				else if(pseudo->id() == &bk->ID)
					break;
			}
		}
		
	}
}

/*
 * ??? This function really categorizes the lblock "line"... ???
 * Annotates the "line" with ALWAYSMISS/ALWAYSHIT/FIRSTMISS/FIRSTHIT
 * In the case of FIRSTMISS, also annotate with the loop-header of the most inner loop.
 */
void CATConstraintBuilder::worst(LBlock *line , ContextTree *node , LBlockSet *idset, int dec){
	int number = idset->count();	
	BasicBlock *bb = line->bb();
	LBlock *cacheline;
	BitSet *in = new BitSet(number);
	
	
	in = bb->use<BitSet *>(ID_In);

	int count = 0;
	bool nonconflitdetected = false;
	bool continu = false;
	unsigned long tagcachline,tagline;

	//test if it's the lbloc which find in the same memory block
	
	/*
	 * If the IN(line) = {LB} and cacheblock(line)==cacheblock(LB), then
	 * nonconflict (Always Hit)
	 */
	if (in->count() == 1){
		for (int i=0;i < number;i++){
		if (in->contains(i)){
			cacheline = idset->lblock(i);
			tagcachline = ((unsigned long)cacheline->address()) >> dec;
			unsigned long tagline = ((unsigned long)line->address()) >> dec;
				if (tagcachline == tagline )
					nonconflitdetected = true;
			}
		}
	}
	
	
	
	//test the virtual non-conflit state
	bool nonconflict = false;
	for (int i=0;i < number;i++) {
			if (in->contains(i)) {
				cacheline = idset->lblock(i);
				tagcachline = ((unsigned long)cacheline->address()) >> dec;
			    tagline = ((unsigned long)line->address()) >> dec;
				if(cacheline->address() == line->address()
				&& line->bb() != cacheline->bb())
					nonconflict = true;
			}
	}
	
	
	bool exist = false;
	// test if the first lblock in the header is Always miss
	if (in->count() == 2){
		bool test = false;
		for (int i=0;i < number;i++){
			if (in->contains(i)) {
				cacheline = idset->lblock(i);
				tagcachline = ((unsigned long)cacheline->address()) >> dec;
				unsigned long tagline = ((unsigned long)line->address()) >> dec;
				if(tagcachline == tagline && line->bb() != cacheline->bb())
					test = true;
				if(tagcachline != tagline )
					exist = true;
				if(test && line->address() == cacheline->address())
					continu = true;
			}
		}
	}
	
	//the last categorisation of the l-block
	Categorization_t lastcateg = line->use<Categorization_t>(ID_Cat);
	if(node->kind()!=ContextTree::LOOP){
		//if ((line->returnSTATENONCONF())&&( nonconflitdetected)){
		if ( nonconflitdetected || continu){	
			line->set<Categorization_t>(ID_Cat,CATConstraintBuilder::ALWAYSHIT);
		}			
		else
			line->set<Categorization_t>(ID_Cat,CATConstraintBuilder::ALWAYSMISS);	
	}
	bool dfm = false;
	if(node->kind()== ContextTree::LOOP){
		BitSet *w = new BitSet(number);
		w = node->use<BitSet *>(ID_Set);
		if (w->count()== 2){
			int U[2];
			int m = 0;
			for (BitSet::Iterator iter(*w); iter; iter++) {
				U[m] = *iter;
				m = m + 1;
			}
			//cacheline = lineset->returnLBLOCK(i);
			tagcachline = ((unsigned long)idset->lblock(U[0])->address()) >> dec;
		    tagline = ((unsigned long)idset->lblock(U[1])->address()) >> dec;
			if(tagcachline == tagline)
					dfm = true;
		}
		
		BitSet inter = *w;
		BitSet dif = *in;
	
		// intersection
		inter.mask(*in);
	
		//difference (IN - U)
		dif.remove(*w);
		
		int identif = line->id();
		
		//basic bock of the header
		BasicBlock * blockheader = node->bb();
		//if ((line->returnSTATENONCONF())&&(nonconflitdetected || continu)){
		if ((nonconflitdetected )|| (continu && !exist)){
			line->set<Categorization_t>(ID_Cat,CATConstraintBuilder::ALWAYSHIT);
		}
		else 
			if(lastcateg == CATConstraintBuilder::FIRSTHIT
//			|| ((line->getNonConflictState() || nonconflict)
			|| ((line->get<bool>(CATBuilder::ID_NonConflict, false) || nonconflict)
				&& inter.count() > 0
				&& dif.count()== 1
				&& blockheader == bb))
					line->set<Categorization_t>(ID_Cat,CATConstraintBuilder::FIRSTHIT);
			else if ((((lastcateg == CATConstraintBuilder::FIRSTMISS) || (lastcateg == CATConstraintBuilder::INVALID))
				&&(in->contains(identif)&&(inter.count()== 1)&&(inter.contains(identif))&& (dif.count() >= 0)&&(w->count()== 1)))
				||((inter.count() == 2)&&(dfm)&&(inter.contains(identif))))
					line->set<Categorization_t>(ID_Cat,CATConstraintBuilder::FIRSTMISS);
				else {
					line->set<Categorization_t>(ID_Cat,CATConstraintBuilder::ALWAYSMISS);
					 if (lastcateg == CATConstraintBuilder::FIRSTMISS){
					 				line->use<CATNode *>(CATBuilder::ID_Node)->setHEADEREVOLUTION(node->bb(),true);
					 }
					 
				}
		}
		
}


}//otawa






