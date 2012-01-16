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

#include <otawa/cfg/features.h>
#include <otawa/stats/BBStatCollector.h>
#include <elm/util/BitVector.h>

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
	BitVector marks(_cfg->countBB());
	typedef Pair<CFG *, Edge *> call_t;
	Vector<Edge *> todo;
	Vector<call_t> calls;

	// initialization
	for(BasicBlock::OutIterator edge(cfg->entry()); edge; edge++)
		todo.push(edge);

	// traverse until the end
	while(todo) {
		Edge *edge = todo.pop();
		BasicBlock *bb;

		// null edge -> leaving function
		if(!edge) {
			edge = calls.top().snd;
			calls.pop();
			collector.leave();
			if(calls) {
				collector.leave();
				bb = edge->target();
			}
		}

		// a virtual call ?
		else switch(edge->kind()) {

		case Edge::NONE:
			ASSERT(false);
			break;

		case Edge::VIRTUAL_RETURN:
		case Edge::CALL:
			bb = 0;
			break;

		case Edge::TAKEN:
		case Edge::NOT_TAKEN:
			if(!marks.bit(edge->target()->number()))
				bb = edge->target();
			else
				bb = 0;
			break;

		case Edge::VIRTUAL:
			if(edge->target() == cfg->exit())
				bb = 0;
			else {
				bb = edge->target();
				collector.enter(ContextualStep(ContextualStep::FUNCTION, edge->target()->address()));
				calls.push(call_t(cfg, BasicBlock::InIterator(cfg->exit())));
				todo.push(0);
			}
			break;


		case Edge::VIRTUAL_CALL: {
				bb = edge->target();

				// recursive call
				if(marks.bit(bb->number()))
					bb = 0;

				// simple call
				else {
					if(!edge->source()->isEntry())
						collector.enter(ContextualStep(ContextualStep::CALL, edge->source()->controlInst()->address()));
					collector.enter(ContextualStep(ContextualStep::FUNCTION, edge->target()->address()));
					BasicBlock *ret = VIRTUAL_RETURN_BLOCK(edge->source());
					if(!ret)
						ret = cfg->exit();
					CFG *called_cfg = CALLED_CFG(edge);
					if(!called_cfg)
						called_cfg = cfg;
					calls.push(call_t(called_cfg, BasicBlock::InIterator(ret)));
					todo.push(0);
				}
			}
			break;
		}

		// process basic block
		if(bb) {
			collect(collector, bb);
			for(BasicBlock::OutIterator edge(bb); edge; edge++)
				todo.push(edge);
			marks.set(bb->number());
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
			for(CFG::BBIterator bb(_cfg); bb; bb++)
				if(!bb->isEnd())
					_total += total(bb);
		}
	}
	return _total;
}

}	// otawa
