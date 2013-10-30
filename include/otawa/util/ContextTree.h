/*
 *	$Id$
 *	ContextTree class interface
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
#ifndef OTAWA_CONTEXT_TREE_H
#define OTAWA_CONTEXT_TREE_H

#include <elm/assert.h>
#include <elm/genstruct/Vector.h>
#include <otawa/properties.h>
#include <otawa/proc/CFGProcessor.h>
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
	elm::genstruct::Vector<BasicBlock *> _bbs;
	elm::genstruct::Vector<ContextTree *> _children;
	
	ContextTree *_parent;
	ContextTree(BasicBlock *bb, CFG *cfg, ContextTree *parent);
	void addChild(ContextTree *tree);
	void addBB(BasicBlock *bb, bool _inline);
public:

	// Methods
	ContextTree(CFG *cfg, ContextTree *parent = 0, bool _inline = true);
	~ContextTree(void);
	inline BasicBlock *bb(void) const;
	inline kind_t kind(void) const;
	inline CFG *cfg(void) const;
	inline ContextTree *parent(void) const;
	/*inline elm::Collection<ContextTree *>& children(void);
	inline elm::Collection<BasicBlock *>& bbs(void);*/
	inline bool isChildOf(const ContextTree *ct);
	ContextTree *enclosingFunction(void);
	
	// ChildrenIterator class
	class ChildrenIterator: public elm::genstruct::Vector<ContextTree *>::Iterator {
	public:
		inline ChildrenIterator(ContextTree *tree);
	};
	
	// BBIterator class
	class BBIterator: public elm::genstruct::Vector<BasicBlock *>::Iterator {
	public:
		inline BBIterator(ContextTree *tree):
			elm::genstruct::Vector<BasicBlock *>::Iterator(tree->_bbs) { }
	};
};


// ContextTreeBuilder class
class ContextTreeBuilder: public Processor {
public:
	ContextTreeBuilder(void);
protected:
	virtual void processWorkSpace(WorkSpace *fw);
};


// ContextTreeByCFGBuilder class
class ContextTreeByCFGBuilder: public CFGProcessor {
public:
	ContextTreeByCFGBuilder(void);
protected:
	virtual void processCFG(WorkSpace *fw, CFG *cfg);	
};

// Features
extern Feature<ContextTreeBuilder> CONTEXT_TREE_FEATURE;
extern Feature<ContextTreeByCFGBuilder> CONTEXT_TREE_BY_CFG_FEATURE;

// Identifiers
extern Identifier<ContextTree *> CONTEXT_TREE;
extern Identifier<ContextTree *> OWNER_CONTEXT_TREE;



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

/*inline elm::Collection<BasicBlock *>& ContextTree::bbs(void) {
	return _bbs;
}

inline elm::Collection<ContextTree *>& ContextTree::children(void) {
	return _children;
}*/


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
: elm::genstruct::Vector<ContextTree *>::Iterator(tree->_children) {
	ASSERT(tree);
};

// Output
inline Output& operator<<(Output& out, ContextTree *ct) {
	out << "context_tree(" << (void *)ct << ")";
	return out;
} 

}	// otawa

#endif	// OTAWA_CONTEXT_TREE_H
