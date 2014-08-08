/*
 *	CCGBuilder class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2006, IRIT UPS.
 *
 *	OTAWA is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	OTAWA is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with OTAWA; if not, write to the Free Software
 *	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */
#include <elm/io.h>
#include <otawa/ccg/Builder.h>
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
#include <otawa/prop/DeletableProperty.h>
#include <otawa/ccg/DFA.h>

using namespace otawa::ilp;
using namespace otawa;
using namespace elm::genstruct;

namespace otawa { namespace ccg {

// DFA Properties
static Identifier<dfa::BitSet *> IN("", 0);


/**
 * @class Builder
 * This processor builds the Cache Conflict Graph of the current task.
 * An instruction cache is required to make it work.
 *
 * @par Provided Feature
 * @ref @li FEATURE
 *
 * @par Required Feature
 * @ref @li COLLECTED_LBLOCKS_FEATURE
 *
 * @par Plugin
 * @li @ref ccg
 *
 * @ingroup ccg
 */

p::declare Builder::reg = p::init("Builder", Version(1, 1, 0))
	.base(CFGProcessor::reg)
	.maker<Builder>()
	.provide(FEATURE)
	.require(COLLECTED_LBLOCKS_FEATURE)
	.require(COLLECTED_CFG_FEATURE);


/**
 * Create a new Builder.
 */
Builder::Builder(p::declare& r): Processor(r) {
}


/**
 * This property stores the list of CCG for the current task.
 *
 * @par Hooks
 * @li @ref FrameWork
 */
Identifier<Collection *> Graph::GRAPHS("otawa::ccg:Graph::GRAPHS", 0);


/**
 * This property stores the CCG node matching the L-Block it is hooked to.
 *
 * @par Hooks
 * @li @ref LBlock
 */
Identifier<Node *> Graph::NODE("otawa::ccg::Graph::NODE", 0);


/**
 */
Identifier<bool> Builder::NON_CONFLICT("otawa::ccg::Builder::NON_CONFLICT", false);


/**
 */
void Builder::processLBlockSet(WorkSpace *fw, LBlockSet *lbset) {
	ASSERT(fw);
	ASSERT(lbset);
	const hard::Cache *cache = hard::CACHE_CONFIGURATION(fw)->instCache();

	// Create the CCG
	Collection *ccgs = Graph::GRAPHS(fw);
	if(!ccgs) {
		ccgs = new Collection(cache->rowCount());
		fw->addProp(new DeletableProperty<Collection *>(Graph::GRAPHS, ccgs));
	}
	Graph *ccg = new Graph;
	ccgs->ccgs[lbset->line()] = ccg;

	// Initialization
	for(LBlockSet::Iterator lblock(*lbset); lblock; lblock++) {
		Node *node = new Node(lblock);
		ccg->add(node);
		Graph::NODE(lblock) = node;
	}

	// Run the DFA
	Problem prob(lbset, lbset->count(), cache, fw);
	const CFGCollection *coll = INVOLVED_CFGS(fw);
	dfa::XCFGVisitor<Problem> visitor(*coll, prob);
	dfa::XIterativeDFA<dfa::XCFGVisitor<Problem> > engine(visitor);
	engine.process();

	// Add the annotations from the DFA result
	for (CFGCollection::Iterator cfg(coll); cfg; cfg++) {
		for (CFG::BBIterator block(*cfg); block; block++) {
			dfa::XCFGVisitor<Problem>::key_t pair(*cfg, *block);
			dfa::BitSet *bitset = engine.in(pair);
			block->addProp(new DeletableProperty<dfa::BitSet *>(IN, new dfa::BitSet(*bitset)));
		}
	}

	// Detecting the non conflict state of each lblock
	BasicBlock *BB;
	LBlock *line;
	int length = lbset->count();
	for(LBlockSet::Iterator lbloc(*lbset); lbloc; lbloc++)
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
	address_t adinst;
	LBlock *aux;

	for (CFGCollection::Iterator cfg(coll); cfg; cfg++) {
		for (CFG::BBIterator bb(*cfg); bb; bb++) {
			if ((cfg != ENTRY_CFG(fw)) || (!bb->isEntry() && !bb->isExit())) {
				dfa::BitSet *info = IN(bb);
				ASSERT(info);
				bool test = false;
				bool visit;
				for(BasicBlock::InstIter inst(bb); inst; inst++) {
					visit = false;
					adinst = inst->address();
					for (LBlockSet::Iterator lbloc(*lbset); lbloc; lbloc++){
						address_t address = lbloc->address();
						// the first lblock in the BB it's a conflict
						if(adinst == address && !test && bb == lbloc->bb()) {
							for (int i = 0; i< length; i++)
								if (info->contains(i)) {
									LBlock *lblock = lbset->lblock(i);
									Node *node = Graph::NODE(lblock);
									new Edge (node, Graph::NODE(lbloc));
								}
							aux = lbloc;
							test = true;
							visit = true;
							break;
						}

						if(adinst == address && !visit && bb == lbloc->bb()) {
							new Edge(Graph::NODE(aux), Graph::NODE(lbloc));
							aux = lbloc;
							break;
						}
					}
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
			new Edge(Graph::NODE(ccgnode1), Graph::NODE(end));
		}

	// Build edge from 'S' till 'end'
	LBlock *s = lbset->lblock(0);
	new Edge(Graph::NODE(s), Graph::NODE(end));

	// Cleanup the DFA annotations
	for (CFGCollection::Iterator cfg(coll); cfg; cfg++)
		for (CFG::BBIterator block(cfg); block; block++)
			block->removeProp(&IN);
}


/**
 */
void Builder::processWorkSpace(WorkSpace *fw) {
	ASSERT(fw);

	// Check the cache
	const hard::Cache *cache = hard::CACHE_CONFIGURATION(fw)->instCache();
	if(!cache)
		out << "WARNING: no instruction cache !\n";

	// Process the l-block sets
	LBlockSet **lbsets = LBLOCKS(fw);
	ASSERT(lbsets);
	for(int i = 0; i < cache->rowCount(); i++)
		processLBlockSet(fw, lbsets[i]);
}


/**
 * This feature ensures that Cache Conflict Graphs has been built. They may
 * accessed by @ref CCG::GRAPHS put on the framework.
 *
 * @par Properties
 * @li @ref Graph::GRAPHS (Framework)
 * @li @ref Graph::NODE (LBlock)
 *
 * @ingroup ccg
 */
p::feature FEATURE("otawa::ccg::FEATURE", new Maker<Builder>());

} }	//otawa::ccg
