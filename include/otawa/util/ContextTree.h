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
#include <otawa/proc/Processor.h>
#include <otawa/proc/Feature.h>

namespace otawa {

class BasicBlock;
class CFG;

// ContextTree class
class ContextTree: public PropList {
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
	elm::datastruct::Vector<ContextTree *> _children;
	
	ContextTree *_parent;
	ContextTree(BasicBlock *bb, CFG *cfg);
	void addChild(ContextTree *tree);
	void addBB(BasicBlock *bb);
public:

	// Globals
	//static GenericIdentifier<ContextTree *> ID;

	// Methods
	ContextTree(CFG *cfg);
	~ContextTree(void);
	inline BasicBlock *bb(void) const;
	inline kind_t kind(void) const;
	inline CFG *cfg(void) const;
	inline ContextTree *parent(void) const;
	inline elm::Collection<ContextTree *>& children(void);
	inline elm::Collection<BasicBlock *>& bbs(void);
	inline bool isChildOf(const ContextTree *ct);
	
	// Iterator
	class ChildrenIterator: public elm::Iterator<ContextTree *> {
	public:
		inline ChildrenIterator(ContextTree *tree);
	};
};


// ContextTreeBuilder class
class ContextTreeBuilder: public Processor {
public:
	ContextTreeBuilder(void);
	virtual void processFrameWork(FrameWork *fw);
};

// Features
extern Feature<ContextTreeBuilder> CONTEXT_TREE_FEATURE;

// Identifiers
extern Identifier<ContextTree *> CONTEXT_TREE;


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

inline elm::Collection<ContextTree *>& ContextTree::children(void) {
	return _children;
}


inline bool ContextTree::isChildOf(const ContextTree *ct) {
	ContextTree *cur = this;
	while(ct) {
		if(cur == ct)
			return true;
		else
			cur = cur->_parent;
	}
	return false;
}

inline CFG *ContextTree::cfg(void) const {
	return _cfg;
}

// ContextTree::ChildrenIterator class
inline ContextTree::ChildrenIterator::ChildrenIterator(ContextTree *tree)
: elm::Iterator<ContextTree *>(tree->_children.visit()) {
	assert(tree);
};

}	// otawa

#endif	// OTAWA_CONTEXT_TREE_H
