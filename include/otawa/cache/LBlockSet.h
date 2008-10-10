/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	otawa/cache/LBlockSet.h -- interface of LBlockSet cache.
 */
#ifndef OTAWA_CACHE_LBLOCKSET_H
#define OTAWA_CACHE_LBLOCKSET_H

#include <assert.h>
#include <otawa/properties.h>
#include <otawa/instruction.h>
#include <elm/genstruct/Vector.h>
#include <elm/genstruct/Table.h>
#include <otawa/cache/LBlock.h>
#include <elm/PreIterator.h>
#include <otawa/cache/ccg/CCGDFA.h>

using namespace elm;

namespace otawa {

// LBlockSet class
class LBlockSet {
	friend class CCGDFA;
	
	//static int counter;
	//int cptr ;
	int linenumber;
	elm::genstruct::Vector<LBlock *> listelbc;
	int _cacheBlockCount;

public:

	// Iterator class
	class Iterator:  public elm::genstruct::Vector<LBlock *>::Iterator {
	public:
		inline Iterator(LBlockSet& bset);
	};
	
	// Methods
	inline LBlockSet(int line);
	//inline IteratorInst<LBlock *> *visit(void);
	int add (LBlock *node);
	int count(void);
	int cacheBlockCount(void);
	int newCacheBlockID(void);
	LBlock *lblock(int i);
	int line(void);
};


// Properties
extern Identifier<LBlockSet **> LBLOCKS;
extern Identifier<genstruct::AllocatedTable<LBlock* >* > BB_LBLOCKS;	 

// Inlines
inline LBlockSet::LBlockSet(int line): linenumber(line), _cacheBlockCount(0) {
	assert(line >= 0);
}
	
/*inline IteratorInst<LBlock *> *LBlockSet::visit(void) {
	Iterator iter(*this);
	return new IteratorObject<Iterator, LBlock *>(iter); 
}*/

// LBlockSet::Iterator inlines
inline LBlockSet::Iterator::Iterator(LBlockSet& lbset)
: elm::genstruct::Vector<LBlock *>::Iterator(lbset.listelbc) {
}

} //otawa

#endif // OTAWA_CACHE_LBLOCKSET_H
