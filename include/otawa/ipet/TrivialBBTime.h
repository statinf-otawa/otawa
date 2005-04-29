/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	otawa/ipet/TrivialBBTime.h -- TrivialBBTime class interface.
 */
#ifndef OTAWA_IPET_TRIVIALBBTIME_H
#define OTAWA_IPET_TRIVIALBBTIME_H

#include <assert.h>
#include <otawa/proc/BBProcessor.h>

namespace otawa {
	
class TrivialBBTime: public BBProcessor {
	int dep;
public:
	inline TrivialBBTime(int depth = 1);
	inline int depth(void) const;

	// BBProcessor overload
	void processBB(FrameWork *fw, CFG *cfg, BasicBlock *bb);
};


// Inlines
inline TrivialBBTime::TrivialBBTime(int depth): dep(depth) {
	assert(depth > 0);
}

inline int TrivialBBTime::depth(void) const {
	return dep;
}

} // otawa

#endif // OTAWA_IPET_TRIVIALBBTIME_H
