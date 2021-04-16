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
 * @class LoopManager;
 * Interface of EXTENDED_LOOP_FEATURE. Mainly provides access to CFG top-level
 * loop.
 *
 * @ingroup cfg
 */

///
LoopManager::~LoopManager() {
}

/**
 * @fn Loop *LoopManager::top(CFG *cfg);
 * Get the top-level loop for the given CFG. Each CFG has a top-level loop
 * that is not a loop but the parent of any loop in the CFG.
 * @return	Top-level loop.
 */


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
	ID(cfg) = this;
}


/**
 * Get an address to identify a loop.
 * This is usually the address of the first instruction of the header,
 * when the header is a basic block. Else, this functions look up in the header
 * successor for a valid basic block.
 * @return	Address of the loop.
 */
Address Loop::address() const {
	Block *v = _h;
	while(!v->isBasic()) {
		bool found = false;
		for(auto e: v->outEdges())
			if(Loop::of(e->sink()) == this) {
				v = e->sink();
				found = true;
				break;
			}
		if(!found)
			return Address::null;
	}
	return v->toBasic()->address();
}


/**
 * @fn bool Loop::isLeaf() const;
 * Test if the current loop is a leaf loop, that is, does not contain any
 * sub-loop.
 * @return	True if the loop does not contain any sub-loop , false else.
 */


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
	Loop *l = il;
	while(!l->isTop()) {
		if(l == this)
			return true;
		l = l->parent();
	}
	return l == this;
}

/**
 * @fn bool Loop::equals(Loop *l) const;
 * Test if the current loop is equal to the given loop.
 * Notice it's just enough to compare pointer to check equality.
 * @param l		Loop to compare with.
 * @return		True if both loops are equal, false else.
 */

/**
 * @fn const List<Loop *>& Loop::subLoops(void) const;
 * Get the set of the immediate sub-loops of the current loop.
 * @return	Immediate sub-loops iterator.
 */

/**
 * @fn const Vector<Edge *>& Loop::exitEdges(void) const;
 * Get the set of exit edges of the current loop.
 * @return	Iterator on exit edges.
 */

/**
 * @fn BlockRange Loop::blocks() const;
 * Get the set of blocks in the current loop (including headers of sub-loops).
 * @return	Set of loop blocks.
 */


/**
 * @class Loop::BlockIter: public PreIterator<BlockIter, Block *>;
 * Iterator on the block contained in the loop (including header of
 * sub-loops).
 */

/**
 */
Loop::BlockIter::BlockIter(): _loop(nullptr), _done(true) {
}

/**
 */
Loop::BlockIter::BlockIter(const Loop *loop): _loop(loop), _done(loop->cfg()->count()) {
	_todo.push(loop->header());
	_done.add(loop->header()->index());
}

/**
 */
void Loop::BlockIter::next(void) {
	Block *b = _todo.pop();
	if(isHeader(b) && b != _loop->header()) {
		for(auto e: Loop::of(b)->exitEdges())
			if(Loop::of(e->sink()) == _loop && !_done.contains(e->sink()->index())) {
				_todo.push(e->sink());
				_done.add(e->sink()->index());
			}
	}
	else
		for(auto e = b->outs(); e(); e++)
			if(!isExit(*e) && !isBack(*e) && !_done.contains(e->sink()->index())) {
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
 * Iterate on the current loop and its sub-loops and call f on each loop.
 * @param f	Function to apply on each loop.
 */
void Loop::forSubLoops(std::function<void(Loop *)> f) {
	f(this);
	for(auto sl: subLoops())
		sl->forSubLoops(f);
}


/**
 * @fn void Loop::forSubLoops(CFG *g, std::function<void(Loop *)> f);
 * Apply the given function f on all loops of the CFG g.
 * @param g		CFG for which loops has to be iterated.
 * @param f		Function to apply to each loop.
 */


/**
 * @fn void Loop::forSubLoops(WorkSpace *ws, std::function<void(Loop *)> f);
 * Apply the given function f on all loops of the workspace task.
 * @param ws	Current workspace.
 * @param f		Function to apply to each loop.
 */


///
void Loop::EntryIter::select() {
	while(!e.ended() && BACK_EDGE(*e))
		e.next();
}


/**
 * @fn Loop::EntryRange Loop::entries() const;
 * Get the collection of entry edges of the loop.
 * @return	Collection of entry edges.
 */


///
void Loop::BackIter::select() {
	while(!e.ended() && !BACK_EDGE(*e))
		e.next();
}


/**
 * @fn Loop::BackRange Loop::backs() const;
 * Get the collection of back edges of the loop.
 * @return	Collection of back edges.
 */


/**
 * Get the list of loop exit edges.
 * @return	List of vector exit edges.
 */
const Vector<Edge *>& Loop::exits() const {
	return **EXIT_LIST(_h);
}


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
class ExtendedLoopBuilder: public BBProcessor, public LoopManager {
public:
	static p::declare reg;
	ExtendedLoopBuilder(p::declare& r = reg): BBProcessor(r) {
	}

	void *interfaceFor(const AbstractFeature& f) override {
		return static_cast<LoopManager *>(this);
	}

	Loop *top(CFG *cfg) override {
		return Loop::top(cfg);
	}

	Loop *loop(Block *v) override {
		return Loop::of(v);
	}

protected:

	void processCFG(WorkSpace *ws, CFG *cfg) override {
		new Loop(cfg);
		BBProcessor::processCFG(ws, cfg);
	}

	void processBB(WorkSpace *ws, CFG *cfg, Block *b) override {
		if(Loop::isHeader(b))
			makeLoop(b);
	}

	void makeLoop(Block *b) {
		if(Loop::ID(b) != nullptr)
			return;
		if(ENCLOSING_LOOP_HEADER(b) != nullptr)
			makeLoop(ENCLOSING_LOOP_HEADER(b));
		new Loop(b);
	}

	void destroyBB(WorkSpace *ws, CFG *cfg, Block *b) override {
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
 * but an easier access is provided by @ref Loop::of(), @ref Loop::top() or by using
 * LoopManager feature interface.
 *
 * @par Properties
 * @li @ref Loop::ID
 *
 * @par Provided by:
 * @li @ref ExtendedLoopBuilder (default)
 *
 */
p::interfaced_feature<LoopManager *> EXTENDED_LOOP_FEATURE("otawa::EXTENDED_LOOP_FEATURE", p::make<ExtendedLoopBuilder>());

}	// otawa
