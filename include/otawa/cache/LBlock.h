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
#include <elm/genstruct/HashTable.h>
#include <elm/inhstruct/DLList.h>
#include <elm/Iterator.h>
#include <otawa/instruction.h>
#include <otawa/cache/ccg/CCGNode.h>
#include <otawa/cfg/BasicBlock.h>
#include <otawa/ilp/Var.h>
#include <otawa/cache/categorisation/CATNode.h>
#include <otawa/hard/Cache.h>

namespace otawa {

// Extern classes
class LBlockSet;

// LBlock class
class LBlock: public elm::inhstruct::DLNode, public PropList {
	friend class LBlockSet;
	
	address_t addr;
	size_t _size;
	int ident;
	int _cacheblock;
	BasicBlock *_bb;
	
	// Private methods
	~LBlock(void) { delete this; };

public:
	
	//constructor
	LBlock(LBlockSet *graphe, address_t head, BasicBlock *bb, size_t size, int _cacheblock);
	
	// methodes
	int id(void);
	int cacheblock(void);
	inline address_t address(void);
	inline BasicBlock *bb(void);
	int countInsts(void);
	inline size_t size(void) const;
};

// Inlines
inline size_t LBlock::size(void) const {
	return _size;
}

inline address_t LBlock::address(void) {
	return addr;
}

inline BasicBlock *LBlock::bb(void) {
	return _bb;
}

} // otawa

#endif // OTAWA_CACHE_LBLOCK_H
