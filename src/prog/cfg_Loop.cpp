/*
 *	Loop class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2017, IRIT UPS.
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

#include <otawa/cfg/Loop.h>
#include <otawa/proc/BBProcessor.h>

namespace otawa {

/**
 * @class Loop
 *
 * This class represents a loop in the CFG. It provides several facilities
 * to examine loops (output edges, sub-loops or parent loop).
 *
 * In order to have loop information, the feature @ref EXTENDED_LOOP_FEATURE
 * has to be required.
 *
 * To get the parent loop of a particular BB, just use @ref Loop::of() method.
 * Be aware that each CFG has a special top-level loop that (a) is parent of
 * any other loop and (b) contains any block that is not contained in a loop.
 * To get it, use the function @ref Loop::top().
 *
 * @ingroup cfg
 */

/**
 * Get the loop containing the given block.
 * @param b		Block to get the loop for.
 */
Loop *Loop::of(Block *b) {
	if(LOOP_HEADER(b))
		return ID(b);
	else if(ENCLOSING_LOOP_HEADER(b) == nullptr)
		return ID(b->cfg());
	else
		return ID(ENCLOSING_LOOP_HEADER(b));
}

/**
 * Get the special top-level loop.
 * @param cfg	CFG to get the loop for.
 * @return		Top-level loop.
 */
Loop *Loop::top(CFG *cfg) {
	return ID(cfg);
}

/**
 * @fn bool Loop::isBack(otawa::Edge *e);
 * Test if the given edge is a back edge.
 * @param e		Edge to test.
 * @return		True if e is a back edge, false else.
 */

/**
 * @fn bool Loop::isExit(otawa::Edge *e);
 * Test if the given edge is an exit edge.
 * @param e		Edge to test.
 * @return		True if e is an exit edge, false else.
 */

/**
 * @fn bool Loop::isHeader(Block *b);
 * Test if the given block is the header of a loop.
 * @param b		Tested block.
 * @return		True if b is a loop header, false else.
 */

/**
 * Build a loop headed by the given block.
 * @param h		Block head of the loop.
 */
Loop::Loop(Block *h): _h(h) {
	Block *p = ENCLOSING_LOOP_HEADER(_h);
	if(p == nullptr)
		_p = top(_h->cfg());
	else
		_p = of(p);
	_p->_c.add(this);
	_d = _p->_d + 1;
	ID(_h) = this;
}

/**
 * Build a top-level loop.
 * @param cfg	CFG of the top-level loop.
 */
Loop::Loop(CFG *cfg): _h(cfg->entry()), _p(nullptr), _d(0) {
	ID(_h) = this;
}

/**
 * @fn Block *Loop::header(void) const;
 * Get the header block of the loop. Notice that the top loop
 * has no header.
 * @return	Loop header block.
 */

/**
 * @fn bool Loop::isTop(void) const;
 * Test if the loop is top-level (the special loop associated with
 * the CFG).
 * @return	True if the loop is top-level, false else.
 */

/**
 * @fn Loop Loop::parent(void) const;
 * Get the parent loop of the current loop. If the current loop is top-level,
 * return null.
 * @return	Parent loop.
 */

/**
 * @fn CFG *Loop::cfg(void) const;
 * Get the CFG containing the loop.
 * @return	CFG containing the loop.
 */

/**
 * @fn int Loop::depth(void);
 * Get the depth of the loop, that is, the distance of the current loop
 * from the top-level loop. The top-level loop has a 0 depth.
 * @return	Loop depth.
 */

/**
 * Test if the current loop includes the given one.
 * @param il	Loop to test for inclusion.
 */
bool Loop::includes(Loop *il) const {
	for(Loop *l = il; !l->isTop(); l = l->parent())
		if(l == this)
			return true;
	return false;
}

/**
 * @fn Loop::ChildIter Loop::subLoops(void) const;
 * Get the iterator on the immediate sub-loops of the current loop.
 * @return	Immediate sub-loops iterator.
 */

/**
 * @fn Loop::ChildIter Loop::endSubLoops(void) const;
 * Get the end tag to iterate on immediate sub-loops.
 * @return	End tag to iterate on immediate sub-loops.
 */

/**
 * @fn Loop::ExitIter Loop::exitEdges(void) const;
 * Get the iterator on the exit edges of the current loop.
 * @return	Iterator on exit edges.
 */

/**
 * @fn Loop::ExitIter endExitEdges(void) const;
 * Get the end tag to iterate on exit edges.
 * @return	End tag to iterate on exit edges.
 */


/**
 * @class Loop::BlockIter: public PreIterator<BlockIter, Block *>;
 * Iterator on the block contained in the loop (including header of
 * sub-loops).
 */

/**
 */
Loop::BlockIter::BlockIter(Loop *loop): _loop(loop), _done(loop->cfg()->count()) {
	_todo.push(loop->header());
	_done.add(loop->header()->index());
}

/**
 */
void Loop::BlockIter::next(void) {
	Block *b = _todo.pop();
	if(isHeader(b) && b != _loop->header())
		for(auto e = Loop::of(b)->exitEdges(); e; e++)
			if(Loop::of(e->sink()) == _loop && !_done.contains(e->sink()->index())) {
				_todo.push(e->sink());
				_done.add(e->sink()->index());
			}
	else
		for(auto e = b->outs(); e; e++)
			if(!isExit(e) && !isBack(e) && !_done.contains(e->sink()->index())) {
				_todo.push(e->sink());
				_done.add(e->sink()->index());
			}
}


/**
 */
io::Output& operator<<(io::Output& out, const Loop *l) {
	if(!l->header())
		out << "top";
	else
		out << "loop(" << l->header() << ")";
	return out;
}

/**
 * Property attaching a @ref Loop to a block or a CFG (for the top-level loop).
 *
 * @par Features
 * @li @ref EXTENDED_LOOP_FEATURE
 *
 */
p::id<Loop *> Loop::ID("");


/**
 * Default implementation for feature @ref EXTENDED_LOOP_FEATURE.
 *
 * @par Provide
 * @li @ref EXTENDED_LOOP_FEATURE
 *
 * @par Require
 * @li @ref LOOP_INFO_FEATURE
 *
 * @ingroup cfg
 */
class ExtendedLoopBuilder: public BBProcessor {
public:
	static p::declare reg;
	ExtendedLoopBuilder(p::declare& r = reg) {
	}

protected:

	void processCFG(WorkSpace *ws, CFG *cfg) override {
		new Loop(cfg);
		BBProcessor::processCFG(ws, cfg);
	}

	void processBB(WorkSpace *ws, CFG *cfg, Block *b) override {
		new Loop(cfg);
		if(Loop::isHeader(b))
			makeLoop(b);
	}

	void makeLoop(Block *b) {
		if(Loop::of(b) != nullptr)
			return;
		Block *p = ENCLOSING_LOOP_HEADER(b);
		if(p != nullptr)
			makeLoop(p);
	}

	void destroy(WorkSpace *ws, CFG *cfg, Block *b) override {
		if(Loop::isHeader(b) || b->isEntry()) {
			delete Loop::ID(b);
			Loop::ID(b).remove();
		}
	}

};


/**
 */
p::declare ExtendedLoopBuilder::reg = p::init("otawa::ExtendedLoopBuilder", Version(1, 0, 0))
	.extend<BBProcessor>()
	.make<ExtendedLoopBuilder>()
	.provide(EXTENDED_LOOP_FEATURE)
	.require(LOOP_INFO_FEATURE);


/**
 * This feature build @ref Loop objects representing the hierarchy of loops and attach it
 * to the CFG and its blocks. The loop can be accessing using properties (described below)
 * but an easier access is provided by @ref Loop::of() or @ref Loop::top().
 *
 * @par Properties
 * @li @ref Loop::ID
 *
 * @par Provided by:
 * @li @ref ExtendedLoopBuilder (default)
 *
 */
p::feature EXTENDED_LOOP_FEATURE("otawa::EXTENDED_LOOP_FEATURE", p::make<ExtendedLoopBuilder>());

}	// otawa
