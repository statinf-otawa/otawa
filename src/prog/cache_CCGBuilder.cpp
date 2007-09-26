/*
 *	$Id$
 *	Copyright (c) 2005-06, IRIT UPS.
 *
 *	CCGConstraintsBuilder class implementation
 */
#include <elm/io.h>
#include <otawa/cache/ccg/CCGBuilder.h>
#include <otawa/cfg/CFGCollector.h>
#include <otawa/dfa/XIterativeDFA.h>
#include <otawa/dfa/XCFGVisitor.h>
#include <otawa/dfa/BitSet.h>
#include <otawa/util/GenGraph.h>
#include <otawa/hard/Cache.h>
#include <otawa/hard/CacheConfiguration.h>
#include <otawa/hard/Platform.h>
#include <otawa/util/LBlockBuilder.h>
#include <otawa/prog/WorkSpace.h>

using namespace otawa::ilp;
using namespace otawa;
using namespace elm::genstruct;
using namespace otawa::ipet;

namespace otawa {

// DFA Properties
static Identifier<dfa::BitSet *> IN("", 0);


/**
 * @class CCGBuilder
 * This processor builds the Cache Conflict Graph of the current task.
 * An instruction cache is required to make it work.
 * 
 * @par Provided Feature
 * @ref @li CCG_FEATURE
 * 
 * @par Required Feature
 * @ref @li COLLECTED_LBLOCKS_FEATURE
 */


/**
 * Create a new CCGBuilder.
 */
CCGBuilder::CCGBuilder(void):
	Processor("CCGBuilder", Version(1, 0, 0))
{
	provide(CCG_FEATURE);
	require(COLLECTED_LBLOCKS_FEATURE);
	require(COLLECTED_CFG_FEATURE);
}


/**
 * This property stores the list of CCG for the current task.
 * 
 * @par Hooks
 * @li @ref FrameWork
 */
Identifier<CCGCollection *> CCG::GRAPHS("otawa::CCG::graphs", 0);


/**
 * This property stores the CCG node matching the L-Block it is hooked to.
 * 
 * @par Hooks
 * @li @ref LBlock
 */
Identifier<CCGNode *> CCG::NODE("otawa::CCG::node", 0);


/**
 */
Identifier<bool> CCGBuilder::NON_CONFLICT("otawa::CCG::non_conflict", false);


/**
 */
void CCGBuilder::processLBlockSet(WorkSpace *fw, LBlockSet *lbset) {
	assert(fw);
	assert(lbset);
	const hard::Cache *cache = fw->platform()->cache().instCache();

	// Create the CCG
	CCGCollection *ccgs = CCG::GRAPHS(fw);
	if(!ccgs) {
		ccgs = new CCGCollection(cache->rowCount());
		fw->addDeletable(CCG::GRAPHS, ccgs);
	}
	CCG *ccg = new CCG;
	ccgs->ccgs[lbset->line()] = ccg;

	// Initialization
	for(LBlockSet::Iterator lblock(*lbset); lblock; lblock++) {
		CCGNode *node = new CCGNode(lblock);
		ccg->add(node);
		CCG::NODE(lblock) = node;
	}

	// Run the DFA
	CCGProblem prob(lbset, lbset->count(), cache, fw);
	CFGCollection *coll = INVOLVED_CFGS(fw);
	dfa::XCFGVisitor<CCGProblem> visitor(*coll, prob);
	dfa::XIterativeDFA<dfa::XCFGVisitor<CCGProblem> > engine(visitor);
	engine.process();
	
	// Add the annotations from the DFA result 
	for (CFGCollection::Iterator cfg(coll); cfg; cfg++) {
		for (CFG::BBIterator block(*cfg); block; block++) {
			dfa::XCFGVisitor<CCGProblem>::key_t pair(*cfg, *block);
			dfa::BitSet *bitset = engine.in(pair);
			block->addDeletable<dfa::BitSet *>(IN, new dfa::BitSet(*bitset));
		}
	}
	
	// Detecting the non conflict state of each lblock
	BasicBlock *BB;
	LBlock *line;
	int length = lbset->count();		
	for(Iterator<LBlock *> lbloc(lbset->visit()); lbloc; lbloc++)
		if(lbloc->id() != 0 && lbloc->id() != length - 1) {
			BB = lbloc->bb();
			dfa::BitSet *inid = IN(BB);
			for(dfa::BitSet::Iterator bit(*inid); bit; bit++)
				line = lbset->lblock(*bit);
				if(cache->block(line->address()) == cache->block(lbloc->address())
					&& BB != line->bb())
					NON_CONFLICT(lbloc) = true;
				
		}
	
	// Building the ccg edges using DFA
	length = lbset->count();
	Inst *inst;
	address_t adinst;			
	PseudoInst *pseudo;
	LBlock *aux;

	for (CFGCollection::Iterator cfg(coll); cfg; cfg++) {
		for (CFG::BBIterator bb(*cfg); bb; bb++) {
			if ((cfg != ENTRY_CFG(fw)) || (!bb->isEntry() && !bb->isExit())) {
				dfa::BitSet *info = IN(bb);
				assert(info);
				bool test = false;
				bool visit;
				for(Iterator<Inst *> inst(bb->visit()); inst; inst++) {
					visit = false;
					pseudo = inst->toPseudo();			
					if(!pseudo){
						adinst = inst->address();				
						for (Iterator<LBlock *> lbloc(lbset->visit()); lbloc; lbloc++){
							address_t address = lbloc->address();
							// the first lblock in the BB it's a conflict
							if(adinst == address && !test && bb == lbloc->bb()) {		
								for (int i = 0; i< length; i++)
									if (info->contains(i)) {
										LBlock *lblock = lbset->lblock(i);
										CCGNode *node = CCG::NODE(lblock);
										new CCGEdge (node, CCG::NODE(lbloc));
									}
								aux = lbloc;
								test = true;
								visit = true;
								break;
							}
					
							if(adinst == address && !visit && bb == lbloc->bb()) {
								new CCGEdge(CCG::NODE(aux), CCG::NODE(lbloc));
								aux = lbloc;
								break;
							}
						}
					}
					else if(pseudo->id() == &bb->ID)
						break;		
				}
			}
		}
	}
	
	// build edge to LBlock end
	BasicBlock *exit = ENTRY_CFG(fw)->exit();
	LBlock *end = lbset->lblock(length-1);
	dfa::BitSet *info = IN(exit);
	for (int i = 0; i< length; i++)
		if (info->contains(i)) {
			LBlock *ccgnode1 = lbset->lblock(i);
			new CCGEdge(CCG::NODE(ccgnode1), CCG::NODE(end));
		}
		
	// Build edge from 'S' till 'end'
	LBlock *s = lbset->lblock(0);
	new CCGEdge(CCG::NODE(s), CCG::NODE(end));
			
	// Cleanup the DFA annotations
	for (CFGCollection::Iterator cfg(coll); cfg; cfg++)
		for (CFG::BBIterator block(cfg); block; block++)
			block->removeProp(&IN);	
}
			
			
/**
 */
void CCGBuilder::processWorkSpace(WorkSpace *fw) {
	assert(fw);
	
	// Check the cache
	const hard::Cache *cache = fw->platform()->cache().instCache();
	if(!cache)
		out << "WARNING: no instruction cache !\n";
	
	// Process the l-block sets
	LBlockSet **lbsets = LBLOCKS(fw);
	assert(lbsets);
	for(int i = 0; i < cache->rowCount(); i++)
		processLBlockSet(fw, lbsets[i]);
}


/**
 * This feature ensures that Cache Conflict Graphs has been built. They may
 * accessed by @ref CCG::GRAPHS put on the framework.
 * 
 * @par Properties
 * @li @ref CCG::GRAPHS (Framework)
 * @li @ref CCG::NODE (LBlock)
 */
Feature<CCGBuilder> CCG_FEATURE("otawa::ccg");

} //otawa
