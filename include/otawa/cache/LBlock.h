/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	otawa/cache/LBlock.h -- interface of LBlock class.
 */
#ifndef OTAWA_CACHE_LBLOCK_H
#define OTAWA_CACHE_LBLOCK_H

#include <elm/string.h>
#include <elm/genstruct/SLList.h>
#include <elm/inhstruct/DLList.h>
#include <elm/Iterator.h>
#include <otawa/instruction.h>
#include <otawa/cache/ccg/CCGNode.h>
#include <otawa/cfg/BasicBlock.h>
#include <otawa/ilp/Var.h>
#include <otawa/cache/categorisation/CATNode.h>
#include <otawa/hardware/Cache.h>

namespace otawa {

// Extern classes
class LBlockSet;
class CCGNode;

// LBlock class
class LBlock: public elm::inhstruct::DLNode, public ProgObject {
	friend class LBlockSet;
	
	address_t lblc;
	int ident;
	BasicBlock *bblblock;
	bool nonconflit;
	ilp::Var *miss;
	ilp::Var *hit;
	ilp::Var *xi;
	int chit;
	int cmiss;
	CCGNode *ccgnod;
	CATNode *catnod; 
	
	// Private methods
	~LBlock(void) { delete this; };

public:
	static LBlockSet *idlblockset; 
	
	//constructor
	LBlock(LBlockSet *graphe, address_t head, BasicBlock *bb, ilp::Var *hit1, 
		ilp::Var *miss1, ilp::Var *xi1, elm::String tp);
	
	// methodes
	int id(void);
	address_t address(void);
	void setNonConflictState(bool set);
	bool getNonConflictState(void);
	BasicBlock *bb(void);
	ilp::Var *hitVar(void);
	ilp::Var *missVar(void);
	ilp::Var *bbVar(void);
	int countInsts(int cycle , const Cache *cach);
	int hitCount(void);
	int missCount(void);
	inline CCGNode *ccgNode();
	inline CATNode *catNode();
};

// Inlines
CCGNode *LBlock::ccgNode(){
	return ccgnod;
}

CATNode *LBlock::catNode() {
	return catnod;
}

} // otawa

#endif // OTAWA_CACHE_LBLOCK_H
