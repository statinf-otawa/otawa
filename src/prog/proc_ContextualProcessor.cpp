/*
 *	$Id$
 *	ContextualProcessor class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2008, IRIT UPS.
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

#include <elm/data/ListQueue.h>
#include <elm/util/SharedPtr.h>
#include <elm/genstruct/Vector.h>
#include <otawa/cfg.h>
#include <otawa/cfg/features.h>
#include <otawa/proc/ContextualProcessor.h>

namespace otawa {

using namespace elm::genstruct;

/**
 * @class ContextualProcessor
 * A contextual processor allows to process basic block in a call-aware way
 * even with a virtual CFG (where called CFG are inlined).
 *
 * First, the basic
 * blocks of the top are processed. When a virtual call is found, enteringCall()
 * method is called and the basic blocks of the called CFG are processed until
 * end of the call (leavingCall() is called at this moment). Then the traversal
 * of the caller CFG restarts. Note that thus behaviour is recursive and that
 * a call in the callee is processed in the same way.
 *
 * This processor may be used to build @ref ContextPath.
 *
 * As an example is often better than any discussion, let three functions :
 * <code>
 *	function f
 *		basic block f1
 *		basic block f2 calling h
 * 		basic block f3 calling g
 *	function g
 *		basic block g1
 *		basic block g2 calling h
 *		basic block g3
 *	function h
 *		basic block h1
 * </code>
 *
 * First the function f is traversed: enteringCall(f), processBB(f1) and processBB(f2).
 * As f2 calls h, we get enteringCall(h), processBB(h1), leavingCall(h).
 * We continue with f3: processBB(f3).
 * And we process the call to g: enteringCall(g), processBB(g1) and processBB(g2).
 * We call another time h: enteringCall(h), processBB(h1) and leavingCall(h).
 * Finally, we leaving g then g: processBB(g3), leavingCall(g), leavingCall(f).
 * @ingroup proc
 */

class Point {
public:

	Point(CFG *cfg) {
		path = CONTEXT(cfg);
		path.push(ContextualStep::FUNCTION, cfg->address());
	}

	SharedPtr<Point> enter(ContextualStep step) {
		return Point(*this, step);
	}

	SharedPtr<Point> leave(void) {
		return parent;
	}

	ContextualPath path;
	SharedPtr<Point> parent;

private:
	Point(const Point& p, ContextualStep s) {
		path = p.path;
		path.push(s);
		parent = p;
	}
};


p::declare ContextualProcessor::reg = p::init("otawa::ContextualProcessor", Version(2, 0, 0))
	.maker<ContextualProcessor>()
	.require(CHECKED_CFG_FEATURE);


/**
 */
ContextualProcessor::ContextualProcessor(p::declare& reg): CFGProcessor(reg) {
}

/**
 */
void ContextualProcessor::processCFG (WorkSpace *ws, CFG *cfg) {
	BitVector done(cfg->count());
	SharedPtr<Point> path = new Point(cfg);

	// define queue
	typedef Pair<Block *, SharedPtr<Point> > item_t;
	ListQueue<item_t> todo;

	// initialization
	for(BasicBlock::EdgeIter edge = cfg->entry()->outs(); edge; edge++)
		todo.put(pair(edge->sink(), path));

	// traverse until the end
	while(todo) {

		// process next block
		item_t i = todo.get();
		if(i.fst->isBasic())
			this->processBB(ws, cfg, i.fst->toBasic(), i.snd->path);
		done.set(i.fst->index());

		// put next blocks
		for(Block::EdgeIter e = i.fst->outs(); e; e++)
			if(!done[e->sink()->index()]) {
				SharedPtr<Point> p = i.snd;
				for(int i = 0; i < LEAVE(e); i++)
					p = p->leave();
				for(Identifier<ContextualStep>::Getter s(e, ENTER); s; s++)
					p = p->enter(s);
				todo.put(pair(e->sink(), i.snd));
			}
}



/**
 * @fn void ContextualProcessor::processBB(WorkSpace *ws, CFG *cfg, BasicBlock *bb, const ContextualPath& path);
 * This method is called each time a basic block is found.
 * @param ws		Current workspace.
 * @param cfg		Current top CFG.
 * @param bb		Current basic block.
 * @param path		Current context path.
 */

} // otawa
