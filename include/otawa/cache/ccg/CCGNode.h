/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	
 */
#ifndef OTAWA_TEST_TEST_CCG_CCGNODE_H
#define OTAWA_TEST_TEST_CCG_CCGNODE_H


#include <elm/genstruct/SLList.h>
#include <elm/inhstruct/DLList.h>
#include <elm/Iterator.h>
#include <otawa/instruction.h>
#include <otawa/cache/ccg/CCGEdge.h>
#include <otawa/cfg/BasicBlock.h>
#include <otawa/ilp/Var.h>
#include <otawa/cache/LBlock.h>
namespace otawa {

class LBlock;
// CCGNode class

class CCGNode: public elm::inhstruct::DLNode, public ProgObject {

	//address_t lblc;
	LBlock *lbl;
	elm::genstruct::SLList<CCGEdge *> ins, outs;
	

	// CCGEdgeIterator
	class CCGEdgeIterator: public IteratorInst<CCGEdge *> {
		elm::genstruct::SLList<CCGEdge *>::Iterator iter;
		public:
		inline CCGEdgeIterator(elm::genstruct::SLList<CCGEdge *> &list)
		: iter(list) { };
		virtual bool ended(void) const;
		virtual CCGEdge *item(void) const;
		virtual void next(void);
	};
	

public:
	
	//constructor
	CCGNode(LBlock *node);
	
	// CCGEdge management
	inline void addInEdge(CCGEdge *edge) { ins.addFirst(edge); };
	void addOutEdge(CCGEdge *edge) { outs.addFirst(edge); };
	void removeInEdge(CCGEdge *edge) { ins.remove(edge); };
	void removeOutEdge(CCGEdge *edge) { outs.remove(edge); };
	inline IteratorInst<CCGEdge *> *inEdges(void) { return new CCGEdgeIterator(ins); };
	inline IteratorInst<CCGEdge *> *outEdges(void) { return new CCGEdgeIterator(outs);};
	
	// methodes
	LBlock *lblock(){return lbl;};
};

} // otawa

#endif // OTAWA_CACHE_CCG_CCGNode_H




