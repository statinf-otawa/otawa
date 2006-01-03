/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	otawa/util/LBlockBuilder.h -- interface of LBlockBuilder class.
 */
#ifndef OTAWA_UTIL_LBLOCKBUILDER_H
#define OTAWA_UTIL_LBLOCKBUILDER_H

#include <otawa/proc/CFGProcessor.h>
#include <otawa/cache/LBlockSet.h>

namespace otawa {

// LBlockBuilder class
class LBlockBuilder: public CFGProcessor {
	void processLBlockSet(FrameWork *fw, CFG *cfg, LBlockSet *lbset);
public:
	LBlockBuilder(const PropList& props = PropList::EMPTY);
	
	// CFGProcessor overload
	virtual void processCFG(FrameWork *fw, CFG *cfg);
};

} // otawa

#endif // OTAWA_UTIL_LBLOCKBUILDER_H
