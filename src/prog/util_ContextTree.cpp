/*
 * $Id$
 * Copyright (c) 2005 IRIT-UPS
 *
 * src/prog/ContextTree.h -- ContextTree class implementation.
 */

#include <otawa/util/ContextTree.h>
#include <otawa/util/Dominance.h>
#include <otawa/util/LoopInfoBuilder.h>
#include <elm/genstruct/Vector.h>
#include <otawa/cfg.h>
#include <otawa/dfa/IterativeDFA.h>
#include <otawa/dfa/BitSet.h>
#include <otawa/prog/WorkSpace.h>

using namespace elm;

namespace otawa {

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
 */
ContextTree::ContextTree(CFG *cfg, ContextTree *parent):
	_kind(ROOT),
	_bb(cfg->entry()),
	_parent(parent),
	_cfg(cfg)
{
	assert(cfg);
	//cerr << "Computing " << cfg->label() << "\n";
	
	
	/*
	 * First, create a ContextTree for each loop.
	 */
	for (CFG::BBIterator bb(cfg); bb; bb++)
		if (Dominance::isLoopHeader(bb)) {
			OWNER_CONTEXT_TREE(bb) = new ContextTree(bb, cfg, this);
			OWNER_CONTEXT_TREE(bb)->addBB(bb);
		}
	
	/*
	 * Then, link each ContextTree to its parents.
	 */
	for (CFG::BBIterator bb(cfg); bb; bb++) {
		if (Dominance::isLoopHeader(bb)) {
			/* Loop header: add the ContextTree to its parent ContextTree */
			if (!ENCLOSING_LOOP_HEADER(bb)) {
				/* The loop is not in another loop: add to the root context tree. */
				addChild(OWNER_CONTEXT_TREE(bb));
			} else {
				/* The loop is in another loop, add to the outer loop's context tree. */
				OWNER_CONTEXT_TREE(ENCLOSING_LOOP_HEADER(bb))->addChild(OWNER_CONTEXT_TREE(bb));
			}
		} else {
			/* Not loop header: add the BasicBlock to its ContextTree */		
			if (!ENCLOSING_LOOP_HEADER(bb)) {
				/* bb is not in a loop: add bb to the root ContextTree */
				addBB(bb);
				OWNER_CONTEXT_TREE(bb)=this;			
			} else {
				/* The bb is in a loop: add the bb to the loop's ContextTree. */
				ContextTree *parent = OWNER_CONTEXT_TREE(ENCLOSING_LOOP_HEADER(bb));
				parent->addBB(bb);
				OWNER_CONTEXT_TREE(bb) = parent;
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
	_bb(bb),
	_kind(LOOP),
	_parent(parent),
	_cfg(cfg)
{
	assert(bb);
	assert(parent);
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
 */
void ContextTree::addBB(BasicBlock *bb) {
	
	// Add the BB
	_bbs.add(bb);
	
	// Process call
	if(bb->isCall())
		for(BasicBlock::OutIterator edge(bb); edge; edge++)
			if(edge->kind() == Edge::CALL && edge->calledCFG()) {
				
				// Detect recursivity
				for(ContextTree *cur = this; cur; cur = cur->parent())
					if(cur->kind() != LOOP && edge->calledCFG() == cur->cfg()) {
						//_children.add(cur);
						return;
					}
				
				// Add the child 
				addChild(new ContextTree(edge->calledCFG(), enclosingFunction()));
			}
			
}


/**
 * Add a children context tree.
 * @param child	Context tree to add.
 */
void ContextTree::addChild(ContextTree *child) {
	assert(child);
	_children.add(child);
	//child->_parent = this;

	if(child->kind() == ROOT) {
	  //cerr << "!!!" <<_cfg->label() << " calls " << child->cfg()->label() << io::endl;
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
		assert(cur);
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
}


/**
 * This feature asserts that a context tree of the task is available in
 * the framework.
 * 
 * @par Properties
 * @li @ref CONTEXT_TREE (@ref FrameWork). 
 * @li @ref OWNER_CONTEXT_TREE (@ref BasicBlock).
 */
Feature<ContextTreeBuilder> CONTEXT_TREE_FEATURE("otawa::context_tree");


/**
 * This property identifier provides a context tree hooked to a framework.
 * A null pointer is retrieved if the context tree is not computed.
 * 
 * @par Hooks
 * @li @ref FrameWork
 */
Identifier<ContextTree *> CONTEXT_TREE("otawa::context_tree", 0);


/**
 * Annotations with this identifier are hooked to basic blocks and gives
 * the owner context tree (ContextTree * data).
 * @par Hooks
 * @li @ref BasicBlock
 */
Identifier<ContextTree *> OWNER_CONTEXT_TREE("otawa::owner_context_tree", 0);

} // otawa
