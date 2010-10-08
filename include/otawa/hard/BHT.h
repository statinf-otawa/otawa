/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	otawa/hard/BHT.h -- interface for BHT class.
 */
#ifndef OTAWA_HARD_BHT_H
#define OTAWA_HARD_BHT_H

#include <assert.h>
#include <elm/io.h>
#include <elm/serial2/macros.h>
#include <elm/genstruct/Vector.h>
#include <otawa/base.h>
#include <otawa/hard/PureCache.h>

namespace otawa { namespace hard {
	
// BHT class
class BHT : public PureCache {
public:
	int cond_penalty;
	int indirect_penalty;
	int cond_indirect_penalty;
	
private:
	
	
	SERIALIZABLE(BHT, ELM_BASE(PureCache) &
	DFIELD(cond_penalty, 10) &
	DFIELD(indirect_penalty, 10) &
	DFIELD(cond_indirect_penalty, 10)
	);


public:
	inline BHT(void) {
	}
	virtual ~BHT(void) { }
	
	// Modifiers
	inline void setCondPenalty(int time) {
	  cond_penalty = time;
	}
	inline void setIndirectPenalty(int time) {
	  indirect_penalty = time;
	}
	inline void setCondIndirectPenalty(int time) {
	  cond_indirect_penalty = time;
	}
	
	// Accessors
	inline int getCondPenalty(void) {
	  return cond_penalty;
	}
	inline int getIndirectPenalty(void) {
	  return indirect_penalty;
	}
	inline int getCondIndirectPenalty(void) {
	  return cond_indirect_penalty;
	}
	
};

extern Identifier<BHT*> BHT_CONFIG;

} } // otawa::hard

#endif // OTAWA_HARD_BHT_H
