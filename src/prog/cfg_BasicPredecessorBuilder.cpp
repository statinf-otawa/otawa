/*
 *	CFG class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2003-08, IRIT UPS.
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

#include <elm/types.h>
#include <otawa/cfg.h>
#include <otawa/proc/BBProcessor.h>

namespace otawa {

class BasicPredecessorBuilder: public BBProcessor {
public:
	static p::declare reg;
	BasicPredecessorBuilder(void): BBProcessor(reg) { }

protected:

	virtual void processBB(WorkSpace *ws, CFG *cfg, Block *b) {
		if(!b->isBasic())
			return;
		BasicBlock *bb = b->toBasic();

		// any basic predecessor?
		bool doit = false;
		for(Block::EdgeIter e = b->ins(); e; e++)
			if(!e->source()->isBasic()) {
				doit = true;
				break;
			}
		if(!doit)
			return;

		// build the list
		genstruct::Vector<BasicBlock::BasicEdge> es;
		genstruct::Vector<Pair<Edge *, Edge *> > wl;
		for(Block::EdgeIter e = b->ins(); e; e++)
			wl.add(pair(*e, *e));
		while(wl) {
			Pair<Edge *, Edge *> ep = wl.pop();

			// basic block case
			if(ep.fst->source()->isBasic()) {
				if(logFor(LOG_BLOCK))
					log << "\t\t\tbasic edge " << ep.fst->source()->toBasic() << ", " << ep.snd << ", " << bb << io::endl;
				es.add(BasicBlock::BasicEdge(ep.fst->source()->toBasic(), ep.snd, bb));
			}

			// synthetic block case
			else if(ep.fst->source()->isSynth()) {
				SynthBlock *sb = ep.fst->source()->toSynth();
				if(!sb->callee()) {
					if(logFor(LOG_BLOCK))
						log << "\t\t\tstandalone basic edge " << ep.fst << ", " << bb << io::endl;
					es.add(BasicBlock::BasicEdge(0, ep.fst, bb));
				}
				else
					for(Block::EdgeIter e = ep.fst->source()->toSynth()->callee()->exit()->ins(); e; e++)
						wl.add(pair(*e, ep.fst));
			}

			// entry block case
			else {
				CFG::CallerIter c = ep.fst->source()->cfg()->callers();
				if(!c) {
					if(logFor(LOG_BLOCK))
						log << "\t\t\tstandalone basic edge " << ep.fst << ", " << bb << io::endl;
					es.add(BasicBlock::BasicEdge(0, ep.fst, bb));
				}
				else
					for(; c; c++)
						for(Block::EdgeIter e = c->ins(); e; e++)
							wl.add(pair(*e, *e));
			}
		}

		// store the list
		*BASIC_PREDECESSORS(b) << es;
		track(BASIC_PREDECESSOR_FEATURE, BASIC_PREDECESSORS(b));
	}
};

p::declare BasicPredecessorBuilder::reg = p::init("otawa::BasicPredecessorBuilder", Version(1, 0, 0))
	.provide(otawa::BASIC_PREDECESSOR_FEATURE)
	.extend<BBProcessor>()
	.make<BasicPredecessorBuilder>();


/**
 * This features ensures that basic predecessors has been computed
 * for each concerned basic block (blocks preceded by an entry or
 * by a function call).
 *
 * A basic predecessor v is a predecessor of a block w (a) that is a basic
 * block and (b) there exists a path to v that contains no other basic block,
 * that is, that only contains end and synthetic blocks.
 *
 * Getting the basic predecessors is useful for analysis working at the very
 * low-level because they represents the actual chaining of instruction blocks.
 *
 * This enables the use Block::primaryIns() in the Block class.
 *
 * @par Properties
 * @li @ref BASIC_PREDECESSORS
 *
 * @par Default Implementation
 * @li @ref PrimaryPredecessorBuilder
 */
p::feature BASIC_PREDECESSOR_FEATURE("otawa::BASIC_PREDECESSOR_FEATURE", p::make<BasicPredecessorBuilder>());

/**
 * Provides, for blocks needing this, the list of primary predecessors
 */
p::id<Bag<BasicBlock::BasicEdge> > BASIC_PREDECESSORS("otawa::BASIC_PREDECESSORS");

}	// otawa
