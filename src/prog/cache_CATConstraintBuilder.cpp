/*
 *	$Id$
 *	Copyright (c) 2005-07, IRIT UPS.
 *
 *	CATConstraintsBuilder class implementation
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
#include <otawa/ipet.h>
#include <otawa/ipet/IPET.h>
#include <otawa/util/ContextTree.h>
#include <elm/genstruct/HashTable.h>
#include <otawa/util/Dominance.h>
#include <otawa/cfg.h>
#include <otawa/hard/CacheConfiguration.h>
#include <otawa/cache/categorisation/CATBuilder.h>
#include <otawa/hard/Platform.h>
#include <otawa/dfa/XIterativeDFA.h>
#include <otawa/dfa/XCFGVisitor.h>
#include <otawa/util/LBlockBuilder.h>
#include <otawa/ipet/TrivialInstCacheManager.h>

using namespace otawa;
using namespace otawa::ilp;
using namespace elm::genstruct;
using namespace otawa::ipet;


namespace otawa { namespace ipet {

using namespace otawa::cache;

/**
 */
Identifier<ilp::Var *> HIT_VAR("otawa::ipet::HIT_VAR_", 0);


/**
 */
Identifier<ilp::Var *> MISS_VAR("otawa::ipet::MISS_VAR_");


/**
 */
Identifier<ilp::Var *> BB_VAR("otawa::ipet::BB_VAR_");


/**
 */
Identifier<CATNode *> NODE("otawa::ipet::NODE", 0);


/**
 * @class CATConstraintBuilder
 * This processor uses categories assigned to L-block of the instruction cache
 * to add contraints and to modify maximized function to support the instruction
 * cache.
 *
 * @par Provided Features
 * @li @ref ICACHE_SUPPORT_FEATURE
 *
 * @par Required Features
 * @li @ref CONTEXT_TREE_FEATURE
 * @li @ref ICACHE_CATEGORY_FEATURE
 * @li @ref COLLECTED_LBLOCKS_FEATURE
 * @li @ref ILP_SYSTEM_FEATURE
 */

/**
 * Build a builder of constraints based on instruction cache access categories.
 */
CATConstraintBuilder::CATConstraintBuilder(void)
:	Processor("otawa.ipet.CATConstraintBuilder", Version(1, 0, 0)),
	_explicit(false)
{
	require(CONTEXT_TREE_FEATURE);
	require(COLLECTED_LBLOCKS_FEATURE);
	require(ICACHE_CATEGORY_FEATURE);
	require(ASSIGNED_VARS_FEATURE);
	require(ILP_SYSTEM_FEATURE);
	provide(INST_CACHE_SUPPORT_FEATURE);
}


/**
 */
void CATConstraintBuilder::processLBlockSet(WorkSpace *fw, LBlockSet *id ) {

	ilp::System *system = SYSTEM(fw);
	ASSERT (system);

	// cache configuration
	const hard::Cache *cach = hard::CACHE_CONFIGURATION(fw)->instCache();

	// LBlock initialization
	ContextTree *ct = CONTEXT_TREE(fw);
	ASSERT(ct);
	for(LBlockSet::Iterator lblock(*id); lblock; lblock++)
		NODE(lblock).add(new CATNode(lblock));
	buildLBLOCKSET(id, ct);

	// Set variables
	for(LBlockSet::Iterator lblock(*id); lblock; lblock++) {
		BasicBlock *bb = lblock->bb();
		if(!bb)
			continue;

		// Link BB variable
		ilp::Var *bbvar = VAR(bb);
		BB_VAR(lblock).add(bbvar);

		// Create x_hit variable
		ilp::Var *vhit;
		if(!_explicit)
			vhit = system->newVar();
		else {
			StringBuffer buf;
			buf << "xhit_" << lblock->address();
			String namex = buf.toString();
			vhit = system->newVar(namex);
		}
		HIT_VAR(lblock).add(vhit);

		// Create x_miss variable
		ilp::Var *miss;
		if(!_explicit)
			miss = system->newVar();
		else {
			StringBuffer buf1;
			buf1 << "xmiss_" << lblock->address();
			String name1 = buf1.toString();
			miss = system->newVar(name1);
		}
		MISS_VAR(lblock).add(miss);
	}

	int length = id->count();
	/*
	 * Process each l-block, creating constraints using the l-block's categorisation
	 */
	for (LBlockSet::Iterator bloc(*id); bloc; bloc++){
		int test = bloc->id();

		/* Avoid first/last l-block */
		if ((test != 0)&&(test != (length-1))) {
			cache::category_t categorie = cache::CATEGORY(bloc);
			Constraint *cons;

			/* If ALWAYSHIT, then x_miss(i,j) = 0 */
			if (categorie == cache::ALWAYS_HIT){
				cons = system->newConstraint(Constraint::EQ,0);
				cons->add(1, MISS_VAR(bloc));
			}
			/* If FIRSTHIT,
			 * x_hit(i,j) = sum x_egde (for all incoming non-back-edges)
			 * and
			 * xhit(i,j) + xmiss(i,j) = x(i)
			 */
			if (categorie == cache::FIRST_HIT) {
				BasicBlock *bb = bloc->bb();
				cons = system->newConstraint(Constraint::EQ);
				cons->addLeft(1, HIT_VAR(bloc));
				bool used = false;
				for(BasicBlock::InIterator edge(bb); edge; edge++) {
					if (!Dominance::dominates(bb, edge->source())){
						cons->addRight(1, VAR(edge));
						used = true;
					}
				}
			if(!used)
				delete cons;


				Constraint *cons2 = system->newConstraint(Constraint::EQ);
				cons2->addLeft(1, BB_VAR(bloc));
				cons2->addRight(1, HIT_VAR(bloc));
				cons2->addRight(1, MISS_VAR(bloc));
			}

			/*
			 * If FIRSTMISS,
			 * xmiss(i,j) == 1  (?!??)
			 */

			if (categorie == cache::FIRST_MISS){
				cons = system->newConstraint(Constraint::EQ,1);
				cons->add(1, MISS_VAR(bloc));
			}
			if (categorie == cache::ALWAYS_MISS){
				if (NODE(bloc)->INLOOP()){
					//if (CATBuilder::NODE(bloc)->HASHEADEREVOLUTION()){
					if(LOWERED_CATEGORY(bloc)) {
						/* If lblock in loop / lblock has HEADEREVOLUTION:
						 * x_miss(i,j) <= x_loop_header (??)
						 * x_miss(i,j) <= sum x_edge (for all incoming non-back-edges of the header of the loop containing the current lblock)
						 */
						Constraint *cons32 = system->newConstraint(Constraint::LE);
						cons32->addLeft(1, MISS_VAR(bloc));
						//ilp::Var *x = (ilp::Var *)CATBuilder::NODE(bloc)->HEADEREVOLUTION()->use<Var *>(VAR);
						ilp::Var *x = VAR(LOWERED_CATEGORY(bloc));
					//cout << bloc->use<CATNode *>(CATBuilder::ID_Node)->HEADEREVOLUTION();
						cons32->addRight(1, x);
				//}
						Constraint * boundingmiss = system->newConstraint(Constraint::LE);
						boundingmiss->addLeft(1, MISS_VAR(bloc));
						for(BasicBlock::InIterator entry(NODE(bloc)->HEADERLBLOCK());
							entry; entry++) {
							if (!Dominance::dominates(NODE(bloc)->HEADERLBLOCK(), entry->source())){
								boundingmiss->addRight(1, VAR(entry));

							}

						}
					}
					else {
						/* xmiss(i,j) = x(i) */
						cons = system->newConstraint(Constraint::EQ);
						cons->addLeft(1, MISS_VAR(bloc));
						cons->addRight(1, BB_VAR(bloc));

					}
				}
				else {
					/* xmiss(i,j) == x(i) */
					cons = system->newConstraint(Constraint::EQ);
					cons->addLeft(1, MISS_VAR(bloc));
					cons->addRight(1, BB_VAR(bloc));
				}
			}
			/*
			this fuction compute  chit & cmiss with 5 cycles trivial execition
		 	and return the number of instructions in the l-block with cache as parametre
		 	*/
			/*int counter =*/ bloc->countInsts(/*cach*/);
//			int latence = bloc->missCount() - bloc->hitCount();
			int latence = cach->missPenalty();
  			system->addObjectFunction(latence, MISS_VAR(bloc));
		} /* of if (not a first/last lblock) */
	} /* of for(all lblocks) */
}



void CATConstraintBuilder::processWorkSpace(WorkSpace *fw) {
	ASSERT(fw);
	LBlockSet **lbsets = LBLOCKS(fw);
	const hard::Cache *cache = hard::CACHE_CONFIGURATION(fw)->instCache();

	for(int i = 0; i < cache->rowCount(); i++)
		processLBlockSet(fw, lbsets[i]);
}


/**
 * Annotate all the loop headers with the set of the l-blocks contained in the loop.
 * Sets the categorisation to "INVALID" for all l-blocks processed by this function.
 * Annotate all processed l-blocks with the the header of the most inner loop.
 * (or with the root of the CT. if there isn't any loop containing the l-block)
 *
 * @param lcache The lblock set.
 * @param root The root ContextTree
 * @return The set of all lblocks contained in the "root" ContextTree and its children (that is, the set of all processed l-blocks)
 *
 */
void CATConstraintBuilder::buildLBLOCKSET(LBlockSet *lcache , ContextTree *root){

		// Cuurently in loop ?
		bool inloop = false;
		if (root->kind()== ContextTree::LOOP )
				inloop = true;
		int ident;

		/*
		 * Call recursively buildLBLOCKSET for each ContextTree children
		 * Merge result with current set
		 */
		for(ContextTree::ChildrenIterator son(root); son; son++)
			buildLBLOCKSET(lcache, son);

		/*
		 * For each lblock that is part of any non-(entry|exit) BB of the current ContextTree:
		 *   - Set the lblock's categorization to INVALID
		 *   - Add this lblock to the current set.
		 */
		for(ContextTree::BBIterator bb(root); bb; bb++){
			if ((!bb->isEntry())&&(!bb->isExit())){ /* XXX */
			for(BasicBlock::InstIter inst(bb); inst; inst++) {
				address_t adlbloc = inst->address();
				for (LBlockSet::Iterator lbloc(*lcache); lbloc; lbloc++){
					if ((adlbloc == (lbloc->address()))&&(bb == lbloc->bb())){
						ident = lbloc->id();
						NODE(lbloc)->setHEADERLBLOCK(root->bb(),inloop);
					}
				}
			}
		}
	}
}


/**
 */
void CATConstraintBuilder::configure(const PropList& props) {
	Processor::configure(props);
	_explicit = EXPLICIT(props);
}

} } // otawa::ipet





