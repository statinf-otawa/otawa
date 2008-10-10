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
#include <elm/PreIterator.h>
#include <otawa/instruction.h>
#include <otawa/cache/ccg/CCGEdge.h>
#include <otawa/cfg/BasicBlock.h>
#include <otawa/ilp/Var.h>
#include <otawa/cache/LBlock.h>
namespace otawa {

class LBlock;
// CCGNode class

class CCGNode: public GenGraph<CCGNode,CCGEdge>::GenNode {

	//address_t lblc;
	LBlock *lbl;

public:
	
	//constructor
	CCGNode(LBlock *node);
	
	// methodes
	LBlock *lblock(){return lbl;};
};

} // otawa

#endif // OTAWA_CACHE_CCG_CCGNode_H




