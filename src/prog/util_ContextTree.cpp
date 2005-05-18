/*
 * $Id$
 * Copyright (c) 2005 IRIT-UPS
 *
 * src/prog/ContextTree.h -- ContextTree class implementation.
 */

#include <otawa/util/ContextTree.h>
#include <otawa/util/Dominance.h>
#include <otawa/cfg.h>

namespace otawa {
	
/**
 * @class ContextTree
 * Representation of a context tree.
 */


/**
 * Build a new context tree for the given CFG.
 * @param cfg	CFG to build the context tree for.
 */
ContextTree::ContextTree(CFG *cfg): _kind(ROOT), _bb(cfg->entry()),
_parent(0), next(0), _children(0), _cfg(cfg) {
	assert(cfg);

	// Compute dominators and loop headers
	if(!cfg->entry()->getProp(&Dominance::ID_RevDom)) {
		Dominance dom;
		dom.processCFG(0, cfg);
	}
	
	// Look children
	scan(cfg->exit());
}


/**
 * Build the context tree of a loop.
 * @param bb	Header of the loop.
 */
ContextTree::ContextTree(BasicBlock *bb)
: _bb(bb), _kind(LOOP), _children(0), next(0), _parent(0), _cfg(0) {
	assert(bb);
	_bbs.add(bb);
	for(Iterator<Edge *> edge(bb->inEdges()); edge; edge++)
		if(Dominance::dominates(bb, edge->source()))
			scan(edge->source(), 1);
}


/**
 * Free the entire tree.
 */
ContextTree::~ContextTree(void) {
	for(ContextTree *cur = _children, *next; cur; cur++) {
		next = cur->next;
		delete cur;
	}
};


/**
 * Scan back the loop or the function from the given header for finding the
 * full body.
 * @param bb	Header of loop or function.
 * @param start	Start index in the BB vector.
 */
void ContextTree::scan(BasicBlock *bb, int start) {
	_bbs.add(bb);
	
	// Find the body
	for(int i =  start; i < _bbs.length(); i++) {
		
		// Look forward for calls
		for(Iterator<Edge *> edge(_bbs[i]->outEdges()); edge; edge++)
			if(edge->kind() == Edge::CALL && edge->calledCFG())
				addChild(new ContextTree(edge->calledCFG()));

		// Look backward
		bool header = false;
		for(Iterator<Edge *> edge(_bbs[i]->inEdges()); edge; edge++) {
			assert(edge->source());
			if(edge->source() != _bb) {
				if(Dominance::dominates(_bbs[i], edge->source()))
						header = true;
				else if(!_bbs.contains(edge->source()))
					_bbs.add(edge->source());
			}
		}
		
		// Add the loop if required
		if(header)
			addChild(new ContextTree(_bbs[i]));
	}
	
	// Remove loop headers
	int target = 0, source = 0;
	for(ContextTree *ctx = _children; ctx; ctx = ctx->next)
		if(ctx->kind() == LOOP) {
			while(_bbs[source] != ctx->bb()) {
				assert(source < _bbs.length());
				_bbs[target++] = _bbs[source++];
			}
			source++;
		}
	while(source < _bbs.length())
		_bbs[target++] = _bbs[source++];
	_bbs.setLength(target);
}


/**
 * Add a children context tree.
 * @param child	Context tree to add.
 */
void ContextTree::addChild(ContextTree *child) {
	assert(child);
	if(!_children)
		_children = child;
	else {
		ContextTree *cur = _children;
		while(cur->next) cur = cur->next;
		cur->next = child;
	}
	child->_parent = this;
	if(child->kind() == ROOT)
		child->_kind = FUNCTION;
}


/**
 * @fn BasicBlock *ContextTree::bb(void) const;
 * Get the BB that heads this tree (enter of functions for ROOT and FUNCTION,
 * header for LOOP).
 * @return	Header of the tree.
 */


/**
 * @fn kind_t ContextTree::kind(void) const;
 * Get the kind of the tree node.
 * @return	Tree node kind.
 */


/**
 * @fn ContextTree *ContextTree::parent(void) const;
 * Get the parent tree of the current one.
 * @return	Parent tree or null for root tree.
 */


/**
 * @fn elm::Collection<ContextTree *>& ContextTree::children(void);
 * Get the collection of children of the current tree.
 * @return	Collection of children.
 */


/**
 */	
IteratorInst<ContextTree *> *ContextTree::visit(void) {
	ChildrenIterator iter(this);
	return new IteratorObject<ChildrenIterator, ContextTree *>(iter);
};


/**
 */
MutableCollection<ContextTree *> *ContextTree::empty(void) {
	// !!TODO!!
	assert(false);
}


/**
 * @class ContextTree::ChildrenIterator
 * Iterator for the children of a context tree.
 */


/**
 * @fn ContextTree::ChildrenIterator::ChildrenIterator(ContextTree *tree);
 * Build a new iterator.
 * @param tree	Tree whose children are visited.
 */


/**
 * @fn bool ContextTree::ChildrenIterator::ended(void) const;
 * Test if the end of iteration is reached.
 * @return	True if the iteration is ended.
 */


/**
 * @fn ContextTree *ContextTree::ChildrenIterator::item(void) const;
 * Get the current item.
 * @return	Current context tree.
 */


/**
 * @fn void ContextTree::ChildrenIterator::next(void);
 * Go to the next children.
 */

} // otawa
