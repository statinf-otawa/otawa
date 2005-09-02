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

namespace otawa { namespace ipet {

// TrivialBBTime class
class TrivialBBTime: public BBProcessor {
	int dep;
public:
	TrivialBBTime(int depth = 1, const PropList& props = PropList::EMPTY);
	inline int depth(void) const;

	// BBProcessor overload
	void processBB(FrameWork *fw, CFG *cfg, BasicBlock *bb);
};


// Inlines
inline int TrivialBBTime::depth(void) const {
	return dep;
}

} } // otawa::ipet

#endif // OTAWA_IPET_TRIVIALBBTIME_H
