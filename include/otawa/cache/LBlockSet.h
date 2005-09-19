/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	otawa/cache/LBlockSet.h -- interface of LBlockSet cache.
 */
#ifndef OTAWA_CACHE_LBLOCKSET_H
#define OTAWA_CACHE_LBLOCKSET_H

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
	
	static int counter;
	int cptr ;
	int linenumber;
	elm::genstruct::Vector<LBlock *> listelbc;
	
	// Iterator class
	class Iterator: public IteratorInst<LBlock *> {
		elm::genstruct::Vector<LBlock *> & lbs;
		int pos;
	public:
		inline Iterator(elm::genstruct::Vector<LBlock *> &vector);
		virtual bool ended(void) const;
		virtual LBlock *item(void) const;
		virtual void next(void);
	};	
	
public:	 
	static Identifier ID_LBlockSet;
	 
	inline LBlockSet();
	inline IteratorInst<LBlock *> *visitLBLOCK(void);
	int addLBLOCK (LBlock *node);
	int returnCOUNTER(void);
	LBlock *returnLBLOCK(int i);
	int cacheline(void);
};

// Inlines
inline LBlockSet::LBlockSet() {
 	cptr = 0;
 	linenumber = counter;
 	counter = counter + 1;
}
	
inline IteratorInst<LBlock *> *LBlockSet::visitLBLOCK(void) {
	return new Iterator(listelbc); 
}

inline LBlockSet::Iterator::Iterator(elm::genstruct::Vector<LBlock *> &vector)
: lbs(vector) {
	pos = 0;
}

} //otawa

#endif // OTAWA_CACHE_LBLOCKSET_H
