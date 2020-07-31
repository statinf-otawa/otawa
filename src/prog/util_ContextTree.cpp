/*
 *	ContextTree class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2005-07, IRIT UPS.
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

#include <elm/data/Vector.h>

#include <otawa/cfg.h>
#include <otawa/cfg/Dominance.h>
#include <otawa/dfa/BitSet.h>
#include <otawa/dfa/IterativeDFA.h>
#include <otawa/prog/WorkSpace.h>
#include <otawa/cfg/features.h>
#include "../../include/otawa/cfg/ContextTree.h"

//#define TRACE(x) cerr << x << io::endl;
#define TRACE(x)

using namespace elm;

namespace otawa {

/*
 * Cleaner used to clear the ENCLOSING_LOOP_HEADER identifier when the
 * LOOP_INFO_FEATURE is invalidated.
 */
class ContextTreeAllCleaner: public elm::Cleaner {
public:
	 ContextTreeAllCleaner(WorkSpace *_ws): ws(_ws) { }

protected:
	virtual void clean() {
		const CFGCollection* cfgc = INVOLVED_CFGS(ws);
		for(CFGCollection::Iter cfg(cfgc); cfg(); cfg++) {
			if(CONTEXT_TREE(*cfg).exists()) {
				delete CONTEXT_TREE(*cfg);
				CONTEXT_TREE(*cfg).remove();
			}
		}
	}
private:
	WorkSpace* ws;
};


/**
 * @enum ContextTree::kind_t
 * This enumerate represents the kind of a context tree.
 */


/**
 * @var ContextTree::ROOT
 * A function context tree root of the whole tree.
 */


/**
 * @var ContextTree::FUNCTION
 * A function context tree (due to a function call).
 */


/**
 * @var ContextTree::LOOP
 * A loop context tree.
 */



/**
 * Build a new context tree for the given CFG.
 * @param cfg		CFG to build the context tree for.
 * @param parent	Parent context tree.
 * @param _inline	If true, inline the call BB.
 */
ContextTree::ContextTree(CFG *cfg, ContextTree *parent, bool do_inline):
	_kind(ROOT),
	_cfg(cfg),
	_parent(parent)
{
	for(Block::EdgeIter ei=cfg->entry()->outs(); ei(); ei++)
		_bb = ei->target()->toBasic();

	ASSERT(cfg);
	TRACE("Computing " << cfg->label());
	
	/*
	 * First, create a ContextTree for each loop.
	 */
	for(CFG::BlockIter bb = cfg->blocks(); bb(); bb++) {
		if (LOOP_HEADER(*bb)) {
			OWNER_CONTEXT_TREE(*bb) = new ContextTree(bb->toBasic(), cfg, this);
			OWNER_CONTEXT_TREE(*bb)->addBlock(*bb, do_inline);
		}
	}

	/*
	 * Then, link each ContextTree to its parents.
	 */
	for (CFG::BlockIter bb = cfg->blocks(); bb(); bb++) {
		if (LOOP_HEADER(*bb)) {
			/* Loop header: add the ContextTree to its parent ContextTree */
			if (!ENCLOSING_LOOP_HEADER(*bb)) {
				/* The loop is not in another loop: add to the root context tree. */
				addChild(OWNER_CONTEXT_TREE(*bb));
			} else {
				/* The loop is in another loop, add to the outer loop's context tree. */
				OWNER_CONTEXT_TREE(ENCLOSING_LOOP_HEADER(*bb))->addChild(OWNER_CONTEXT_TREE(*bb));
			}
		} else {
			/* Not loop header: add the BasicBlock to its ContextTree */		
			if (!ENCLOSING_LOOP_HEADER(*bb)) {
				/* bb is not in a loop: add bb to the root ContextTree */
				addBlock(*bb, do_inline);
				OWNER_CONTEXT_TREE(*bb)=this;
			} else {
				/* The bb is in a loop: add the bb to the loop's ContextTree. */
				ContextTree *parent = OWNER_CONTEXT_TREE(ENCLOSING_LOOP_HEADER(*bb));
				parent->addBlock(*bb, do_inline);
				OWNER_CONTEXT_TREE(*bb) = parent;
			}
		}
	}	
}


/**
 * Build the context tree of a loop.
 * @param bb		Header of the loop.
 * @param cfg		Owner CFG.
 * @param parent	Parent context tree.
 */
ContextTree::ContextTree(BasicBlock *bb, CFG *cfg, ContextTree *parent):
	_kind(LOOP),
	_bb(bb),
	_cfg(cfg),
	_parent(parent)
{
	ASSERT(bb);
	ASSERT(parent);
}


/**
 * Free the entire tree.
 */
ContextTree::~ContextTree(void) {
	for(int i = 0; i < _children.length(); i++)
		delete _children[i];
};


/**
 * Add the given basic block to the context tree.
 * @param bb	Added BB.
 * @param bb	If true, inline the call.
 */
void ContextTree::addBlock(Block *bb, bool do_inline) {
	ASSERT(bb);
	TRACE("inline=" << _inline);
	
	// Add the BB
	_bbs.add(bb);
	
	// Process call
	if(do_inline && bb->isCall()) {
			if(bb->toSynth()->callee()) {

				
				// Detect recursivity
				for(ContextTree *cur = this; cur; cur = cur->parent())
					if(cur->kind() != LOOP && bb->toSynth()->callee() == cur->cfg()) {
						return;
					}
				
				// Add the child 
				addChild(new ContextTree(bb->toSynth()->callee(), enclosingFunction()));
			}
	}
}


/**
 * Add a children context tree.
 * @param child	Context tree to add.
 */
void ContextTree::addChild(ContextTree *child) {
	ASSERT(child);
	_children.add(child);

	if(child->kind() == ROOT) {
		TRACE("!!!" <<_cfg->label() << " calls " << child->cfg()->label());
		child->_kind = FUNCTION;
	}
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
 * @fn bool ContextTree::isChildOf(const ContextTree *ct);
 * Test transitively if the current context tree is a child of the given one.
 * @param ct	Parent context tree.
 * @return		True if the current context tree is a child of the given one.
 */


/**
 * Find the enclosing function containing this context tree. Called on a function
 * or root context tree returns the current context tree itself.
 * @return	Enclosing function context tree.
 */
ContextTree *ContextTree::enclosingFunction(void) {
	ContextTree *cur;
	for(cur = this; cur->kind() == LOOP; cur = cur->parent())
		ASSERT(cur);
	return cur;
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


/**
 * @fn CFG *ContextTree::cfg(void) const;
 * Get the CFG containing this context tree.
 * @return	Container CFG.
 */


/**
 * @fn elm::Collection<BasicBlock *>& ContextTree::bbs(void);
 * Get the basic blocks in the current context tree.
 * @return	Basic block collection.
 */


/**
 * @class ContextTreeBuilder
 * This processor produces context tree information. 
 *
 * @par Configuration
 * none
 *
 * @par Required Features
 * @li @ref DOMINANCE_FEATURE
 * @li @ref LOOP_HEADERS_FEATURE
 * @li @ref LOOP_INFO_FEATURE
 *
 * @par Provided Features
 * @li @ref CONTEXT_TREE_FEATURE
 *
 * @par Statistics
 * none
 */

ContextTreeBuilder::ContextTreeBuilder(void)
: Processor("otawa::context_tree_builder", Version(1, 0, 0)) {
	require(DOMINANCE_FEATURE);
	require(LOOP_HEADERS_FEATURE);
	require(LOOP_INFO_FEATURE);
	provide(CONTEXT_TREE_FEATURE);
}


/**
 */
void ContextTreeBuilder::processWorkSpace(WorkSpace *fw) {
	CONTEXT_TREE(fw) = new ContextTree(ENTRY_CFG(fw));
	addCleaner(CONTEXT_TREE_FEATURE, new ContextTreeAllCleaner(fw));
}


/**
 * @class ContextTreeByCFGBuilder
 * Build a context tree for each CFG involved in the current computation.
 * 
 * @par Configuration
 * none
 *
 * @par Required Features
 * @li @ref DOMINANCE_FEATURE
 * @li @ref LOOP_HEADERS_FEATURE
 * @li @ref LOOP_INFO_FEATURE
 * @li @ref COLLECTED_CFG_FEATURE
 *
 * @par Provided Features
 * @li @ref CONTEXT_TREE_BY_CFG_FEATURE
 *
 * @par Statistics
 * none
 */


/**
 */
ContextTreeByCFGBuilder::ContextTreeByCFGBuilder(void)
: CFGProcessor("otawa::ContextTreeByCFGBuilder", Version(1, 0, 0)), fst(true) {
	require(DOMINANCE_FEATURE);
	require(LOOP_HEADERS_FEATURE);
	require(LOOP_INFO_FEATURE);
	provide(CONTEXT_TREE_BY_CFG_FEATURE);
}


/**
 */
void ContextTreeByCFGBuilder::processCFG(WorkSpace *fw, CFG *cfg) {
	CONTEXT_TREE(cfg) = new ContextTree(cfg, 0, false);
	if(fst) { // only add the cleaner for the first time
		addCleaner(CONTEXT_TREE_BY_CFG_FEATURE, new ContextTreeAllCleaner(fw));
		fst = false;
	}
}


/**
 * This feature asserts that a context tree of the task is available in
 * the framework.
 * 
 * @par Properties
 * @li @ref CONTEXT_TREE (hooked to the @ref FrameWork). 
 */
Feature<ContextTreeBuilder> CONTEXT_TREE_FEATURE("otawa::CONTEXT_TREE_FEATURE");


/**
 * Assert that a context tree has been built for each CFG involved in the
 * current computation.
 * 
 * @par Properties
 * @li @ref CONTEXT_TREE (hooked to the @ref CFG).
 */
Feature<ContextTreeByCFGBuilder>
	CONTEXT_TREE_BY_CFG_FEATURE("otawa::CONTEXT_TREE_BY_CFG_FEATURE");


/**
 * This property identifier provides a context tree hooked to a framework.
 * A null pointer is retrieved if the context tree is not computed.
 * 
 * @par Hooks
 * @li @ref FrameWork
 */
Identifier<ContextTree *> CONTEXT_TREE("otawa::CONTEXT_TREE", 0);


/**
 * Annotations with this identifier are hooked to basic blocks and gives
 * the owner context tree (ContextTree * data).
 * 
 * @par Hooks
 * @li @ref BasicBlock
 * 
 * @deprecated	Not working without inlining.
 */
Identifier<ContextTree *> OWNER_CONTEXT_TREE("otawa::OWNER_CONTEXT_TREE", 0);

} // otawa
