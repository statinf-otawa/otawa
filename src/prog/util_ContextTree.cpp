/*
 * $Id$
 * Copyright (c) 2005 IRIT-UPS
 *
 * src/prog/ContextTree.h -- ContextTree class implementation.
 */

#include <otawa/util/ContextTree.h>
#include <otawa/util/Dominance.h>
#include <elm/genstruct/Vector.h>
#include <otawa/cfg.h>
#include <otawa/dfa/IterativeDFA.h>
#include <otawa/util/BitSet.h>

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
 * @class ContextTree
 * This class defines a DFA problem for detecting which loop or function call
 * contains a BB.
 * @par
 * For building the context tree, we consider the CFG as pair <N, E>, N the set
 * of basic blocks and E the set of edges. We use a DFA that works on a
 * reversed CFG using the following sets :
 * <dl>
 * 	<dt>LOOP</dt><dd>set of loop headers</dd>
 * 	<dt>ENTRY</dt><dd>set of inlined function entries</dd>
 * 	<dt>EXIT</dt><dd>set of inlined function exits</dd>
 * </dl>
 * @par
 * The gen set is built is a follow :
 * gen(n) =<br>
 * 		if n in LOOP, { }<br>
 * 		else if n in EXIT(m) { m }<br>
 * 		else { m / (m, n) in E and m in LOOP }
 * @par
 * And the kill set is as follow :<br>
 *		kill(n) = { n / n in LOOP U ENTRY }
 * @note This implemenation does not yet support virtual CFG.
 */
class ContextTreeProblem {
	CFG& _cfg;
	genstruct::Vector<BasicBlock *> hdrs;
public:
	
	ContextTreeProblem(CFG& cfg): _cfg(cfg) {
		//Dominance::ensure(&cfg);
		for(Iterator<BasicBlock *> bb(cfg.bbs()); bb; bb++)
			if(!bb->isEntry() && Dominance::isLoopHeader(bb))
				hdrs.add(bb);
	}
	
	inline BitSet *empty(void) const {
		return new BitSet(hdrs.length());
	}

	BitSet *gen(BasicBlock *bb) const {
		BitSet *result = empty();
		if(!Dominance::isLoopHeader(bb))
			for(BasicBlock::OutIterator edge(bb); edge; edge++) {
				//cout << edge->kind() << '\n';
				if(edge->kind() != Edge::CALL
				&& Dominance::dominates(edge->target(), bb))
					result->add(hdrs.indexOf(edge->target()));
			}
		return result;
	}
	
	BitSet *kill(BasicBlock *bb) const {
		BitSet *result = empty();
		if(Dominance::isLoopHeader(bb))
			result->add(hdrs.indexOf(bb));
		return result;
	}

	bool equals(BitSet *set1, BitSet *set2) const {
		return set1->equals(*set2);
	}
	
	void reset(BitSet *set) const {
		set->empty();
	}
	
	void merge(BitSet *dst, BitSet *src) const {
		*dst += *src;
	}
	
	void set(BitSet *dst, BitSet *src) const {
		*dst = *src;
	}
	
	void add(BitSet *dst, BitSet *src) const {
		*dst += *src;
	}
	
	void diff(BitSet *dst, BitSet *src) {
		*dst -= *src;
	}
	
	inline int count(void) const {
		return hdrs.length();
	}
	
	inline BasicBlock *get(int index) const {
		return hdrs[index];
	}

	#ifndef NDEBUG
		/* Dump the content of a bit set.
		 */
		void dump(elm::io::Output& out, BitSet *set) {
			bool first = true;
			cout << "{ ";
			for(int i = 0; i < hdrs.length(); i++)
				if(set->contains(i)) {
					if(first)
						first = false;
					else
						cout << ", ";
					cout << hdrs[i]->number();
				}
			cout << " }";
		}
	#endif
};


/**
 * Annotations with this identifier are hooked to basic blocks and gives
 * the ower context tree (ContextTree * data).
 */
/*GenericIdentifier<ContextTree *>
	ContextTree::ID("ContextTree::id", 0, OTAWA_NS);*/


/**
 * Build a new context tree for the given CFG.
 * @param cfg	CFG to build the context tree for.
 */
ContextTree::ContextTree(CFG *cfg): _kind(ROOT), _bb(cfg->entry()),
_parent(0), _cfg(cfg) {
	assert(cfg);
	//cout << "Computing " << cfg->label() << "\n";
	
	// Define the problem
	ContextTreeProblem prob(*cfg);
	if(prob.count() == 0) {
		for(Iterator<BasicBlock *> bb(cfg->bbs()); bb; bb++)
			addBB(bb);
		_bbs.merge(&cfg->bbs());		
		return;
	}
	//cout << "children = " << prob.count() << "\n";
	
	// Compute the solution
	dfa::IterativeDFA<ContextTreeProblem, BitSet, dfa::Successor>
		dfa(prob, *cfg);
	dfa.compute();

	// Dump the DFA
	/*cout << "\nDFA\n";
	for(Iterator<BasicBlock *> bb(cfg->bbs()); bb; bb++) {
		cout << "BB " << bb->number() << " (" << bb->address() << ")";
		if(Dominance::isLoopHeader(bb))
			cout << " HEADER";
		cout << "\n\t gen = ";
		prob.dump(cout, dfa.genSet(bb));
		cout << "\n\t kill = ";
		prob.dump(cout, dfa.killSet(bb));
		cout << "\n\t in = ";
		prob.dump(cout, dfa.inSet(bb));
		cout << "\n\t out = ";
		prob.dump(cout, dfa.outSet(bb));
		cout << '\n';
	}*/
	
	// Prepare the tree analysis
	ContextTree *trees[prob.count()];
	BitSet *vecs[prob.count()];
	for(int i = 0; i < prob.count(); i++) {
		vecs[i] = dfa.outSet(prob.get(i));
		//cout << "Child " << i << " " << *vecs[i];
		trees[i] = new ContextTree(prob.get(i), cfg);
		if(vecs[i]->isEmpty()) {
			addChild(trees[i]);
			//cout << " root child";
		}
		//cout << "\n";
		vecs[i]->add(i);
	}
	
	// Children find their parent
	for(int i = 0; i < prob.count(); i++) {
		vecs[i]->remove(i);
		//cout << "INITIAL " << i << " " << *vecs[i] << "\n";
		for(int j = 0; j < prob.count(); j++) {
			/*if(i != j)
				cout << "TEST " << *vecs[i] << " == " << *vecs[j] << "\n";*/
			if(i != j && *vecs[i] == *vecs[j]) {
				trees[j]->addChild(trees[i]);
				//cout << "FOUND !\n";
				break;
			}
		}
		assert(trees[i]->_parent);
		vecs[i]->add(i);
	}
	
	// BB find their parent
	for(Iterator<BasicBlock *> bb(cfg->bbs()); bb; bb++) {
		BitSet& bv = *dfa.outSet(bb);
		if(bv.isEmpty())
			addBB(bb);
		else
			for(int i = 0; i < prob.count(); i++)
				if(vecs[i]->equals(bv)) {
					trees[i]->addBB(bb);
					break;
			}
	}
}


/**
 * Build the context tree of a loop.
 * @param bb	Header of the loop.
 */
ContextTree::ContextTree(BasicBlock *bb, CFG *cfg)
: _bb(bb), _kind(LOOP), _parent(0), _cfg(cfg) {
	assert(bb);
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
	_bbs.add(bb);
	if(bb->isCall())
		for(BasicBlock::OutIterator edge(bb); edge; edge++)
			if(edge->kind() == Edge::CALL && edge->calledCFG())
				addChild(new ContextTree(edge->calledCFG()));
}


/**
 * Add a children context tree.
 * @param child	Context tree to add.
 */
void ContextTree::addChild(ContextTree *child) {
	assert(child);
	_children.add(child);
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
 * @fn bool ContextTree::isChildOf(const ContextTree *ct);
 * Test transitively if the current context tree is a child of the given one.
 * @param ct	Parent context tree.
 * @return		True if the current context tree is a child of the given one.
 */


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
 * A processor used to build the context trees.
 * 
 * @par Required Features
 * @li @ref DOMINANCE_FEATURE
 * 
 * @par Provided Features
 * @li @ref CONTEXT_TREE_FEATURE
 */


/**
 * Build the context tree of the current task.
 * 
 * @par Provided Features
 * @li @ref CONTEXT_TREE_FEATURE
 * 
 * @par Required Features
 * @li @ref DOMINANCE_FEATURE
 * @li @ref LOOP_HEADERS_FEATURE
 */
ContextTreeBuilder::ContextTreeBuilder(void)
: Processor("otawa::context_tree_builder", Version(1, 0, 0)) {
	require(DOMINANCE_FEATURE);
	require(LOOP_HEADERS_FEATURE);
	provide(CONTEXT_TREE_FEATURE);
}


/**
 */
void ContextTreeBuilder::processFrameWork(FrameWork *fw) {
	CONTEXT_TREE(fw) = new ContextTree(ENTRY_CFG(fw));
}


/**
 * This feature asserts that a context tree of the task is available in
 * the framework.
 * 
 * @par Properties
 * @li @ref CONTEXT_TREE (@ref FrameWork). 
 */
Feature<ContextTreeBuilder> CONTEXT_TREE_FEATURE("otawa::context_tree");


/**
 * This property identifier provides a context tree hooked to a framework.
 * A null pointer is retrieved if the context tree is not computed.
 * 
 * @par Hooks
 * @li @ref FrameWork
 */
Identifier<ContextTree *> CONTEXT_TREE("otawa::context_tree", 0, otawa::NS);

} // otawa
