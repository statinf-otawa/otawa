/*
 * $Id$
 * Copyright (c) 2005 IRIT-UPS
 *
 * include/otawa/util/ContextTree.h -- ContextTree class interface.
 */
#ifndef OTAWA_CONTEXT_TREE_H
#define OTAWA_CONTEXT_TREE_H

#include <assert.h>
#include <elm/datastruct/Vector.h>
#include <otawa/properties.h>

namespace otawa {

class BasicBlock;
class CFG;

// ContextTree class
class ContextTree: public PropList, public elm::Collection<ContextTree *> {
public:
	// Kind
	typedef enum kind_t {
		ROOT = 0,
		FUNCTION,
		LOOP
	} kind_t;
	
private:
	kind_t _kind;
	BasicBlock *_bb;
	CFG *_cfg;
	elm::datastruct::Vector<BasicBlock *> _bbs;
	ContextTree *_parent;
	ContextTree *_children;
	ContextTree *next;
	ContextTree(BasicBlock *bb);
	void scan(BasicBlock *bb, int start = 0);
	void addChild(ContextTree *tree);
public:

	// Methods
	ContextTree(CFG *cfg);
	~ContextTree(void);
	inline BasicBlock *bb(void) const;
	inline kind_t kind(void) const;
	inline CFG *cfg(void) const { return _cfg; };
	inline ContextTree *parent(void) const;
	elm::Collection<ContextTree *>& children(void);
	elm::Collection<BasicBlock *>& bbs(void);
	
	// Collection overload
	virtual IteratorInst<ContextTree *> *visit(void);
	virtual MutableCollection<ContextTree *> *empty(void);

	// Iterator
	class ChildrenIterator
	: public PreIterator<ChildrenIterator, ContextTree *> {
		ContextTree *cur;
	public:
		inline ChildrenIterator(ContextTree *tree);
		inline bool ended(void) const;
		inline ContextTree *item(void) const;
		inline void next(void);
	};
};


// ContextTree inlines
inline BasicBlock *ContextTree::bb(void) const {
	return _bb;
};

inline ContextTree::kind_t ContextTree::kind(void) const {
	return _kind;
};

inline ContextTree *ContextTree::parent(void) const {
	return _parent;
};

inline elm::Collection<BasicBlock *>& ContextTree::bbs(void) {
	return _bbs;
}


// ContextTree::ChildrenIterator class
inline ContextTree::ChildrenIterator::ChildrenIterator(ContextTree *tree)
: cur(tree->_children) {
	assert(tree);
};

inline bool ContextTree::ChildrenIterator::ended(void) const {
	return !cur;
};

inline ContextTree *ContextTree::ChildrenIterator::item(void) const {
	assert(cur);
	return cur;
};

inline void ContextTree::ChildrenIterator::next(void) {
	cur = cur->next;
};

}	// otawa

#endif	// OTAWA_CONTEXT_TREE_H
