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
#include <elm/Collection.h> 
#include <elm/genstruct/Vector.h>
#include <otawa/cache/LBlock.h>
#include <elm/Iterator.h>
#include <otawa/cache/ccg/CCGDFA.h>


namespace otawa {

// LBlockSet class
class LBlockSet {
	friend class CCGDFA;
	
	//static int counter;
	//int cptr ;
	int linenumber;
	elm::genstruct::Vector<LBlock *> listelbc;

public:

	// Iterator class
	class Iterator:  public elm::genstruct::Vector<LBlock *>::Iterator {
	public:
		inline Iterator(LBlockSet& bset);
	};
	
	// Methods
	inline LBlockSet(int line);
	inline IteratorInst<LBlock *> *visit(void);
	int add (LBlock *node);
	int count(void);
	LBlock *lblock(int i);
	int line(void);
};


// Properties
extern GenericIdentifier<LBlockSet **> LBLOCKS;
	 

// Inlines
inline LBlockSet::LBlockSet(int line): linenumber(line) {
	assert(line >= 0);
}
	
inline IteratorInst<LBlock *> *LBlockSet::visit(void) {
	Iterator iter(*this);
	return new IteratorObject<Iterator, LBlock *>(iter); 
}

// LBlockSet::Iterator inlines
inline LBlockSet::Iterator::Iterator(LBlockSet& lbset)
: elm::genstruct::Vector<LBlock *>::Iterator(lbset.listelbc) {
}

} //otawa

#endif // OTAWA_CACHE_LBLOCKSET_H
