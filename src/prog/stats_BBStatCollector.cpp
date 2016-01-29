/*
 *	$Id$
 *	BBStatCollector class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2011, IRIT UPS.
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

#include <elm/util/BitVector.h>
#include <otawa/cfg/features.h>
#include <otawa/prog/WorkSpace.h>
#include <otawa/stats/BBStatCollector.h>

namespace otawa {





/**
 * @class BBStatCollector;
 * This class alleviates the work of building a statistics collector.
 * It ensures the traversal of all basic blocks and calls collect() to let
 * the inheriting class do specific work for a basic block.
 */


/**
 * Build the BB statistics collector.
 * @param ws	Current workspace.
 */
BBStatCollector::BBStatCollector(WorkSpace *ws): _ws(ws), _cfg(0), _total(-1) {
}


/**
 */
void BBStatCollector::collect(Collector& collector) {
	const CFGCollection *coll = INVOLVED_CFGS(ws());
	for(int i = 0; i < coll->count(); i++) {
		_cfg = coll->get(i);
		process(collector);
	}
}


/**
 * Process basic block of the current CFG.
 * @param collector		Collector to use.
 */
void BBStatCollector::process(Collector& collector) {
	const CFGCollection *coll = INVOLVED_CFGS(_ws);
	ASSERT(coll);
	for(int i = 0; i < coll->count(); i++) {
		_cfg = coll->get(i);
		processCFG(collector, coll->get(i));
	}
}

void BBStatCollector::processCFG(Collector& collector, CFG *cfg) {
	BitVector marks(_cfg->count());
	Vector<Pair<Block *, ContextualPath> > todo;

	// initialization
	todo.push(pair(cfg->entry(), *CONTEXT(cfg)));
	marks.set(cfg->entry()->index());

	// traverse until the end
	while(todo) {
		Pair<Block *, ContextualPath> cur = todo.pop();
		for(Block::EdgeIter e = cur.fst->outs(); e; e++) {

			// already processed sink?
			if(marks[e->sink()->index()])
				continue;
			marks.set(e->sink()->index());

			// update the context
			ContextualPath p = cur.snd;
			for(int i = 0; i < LEAVE(e); i++)
				p.pop();
			for(Identifier<ContextualStep>::Getter s(e, ENTER); s; s++)
				p.push(*s);

			// collect from the block
			if(e->sink()->isBasic())
			this->collect(collector, e->sink()->toBasic(), p);
		}
	}
}


/**
 * @fn void BBStatCollector::collect(Collector& collector, BasicBlock *bb);
 * This method is called for each basic block in the current workspace (except for
 * syntactic entry and exit blocks). It must specialized by the inheriting class
 * to provide specific work.
 *
 * The methods ws(), cfg() and path() allows to get the current information about
 * the processed blocks.
 *
 * @param collector		The invoker collector to pass statistics information.
 * @param bb			Current basic block.
 */


/**
 * This method is automatically called on each basic block
 * to compute the @ref total() result as the sum of total of each
 * basic block. This methodmust be overriden to provide a customized behaviour.
 * As a default, it returns 0.
 * @param bb	Current basic block.
 * @return		Total of the basic block.
 */
int BBStatCollector::total(BasicBlock *bb) {
	return 0;
}


/**
 * Default implementation of a total as the sum of the total
 * of each basic block (method total(BasicBlock *). It may be overriden
 * if the default behavior does not match.
 * @notice	As a default, entry and exit nodes are ignored.
 * @return	Total of the statistics.
 */
int BBStatCollector::total(void) {
	if(_total < 0) {
		_total = 0;
		const CFGCollection *coll = INVOLVED_CFGS(_ws);
		ASSERT(coll);
		for(int i = 0; i < coll->count(); i++) {
			_cfg = coll->get(i);
			for(CFG::BlockIter bb = _cfg->blocks(); bb; bb++)
				if(bb->isBasic())
					_total += total(bb->toBasic());
		}
	}
	return _total;
}

}	// otawa
