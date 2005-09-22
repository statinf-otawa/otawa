/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	otawa/cache/ccg/CCGBuilder.h -- interface of CCGBuilder class.
 */
#ifndef OTAWA_CACHE_CCGBUILDER_H
#define OTAWA_CACHE_CCGBUILDER_H

#include <assert.h>
#include <otawa/proc/CFGProcessor.h>
#include <otawa/prop/Identifier.h>

namespace otawa {
	
// Extern classes
class LBlockSet;
class CFG;
class LBlockSet;

// CCGBuilder class
class CCGBuilder: public CFGProcessor {
	FrameWork *fw;
	void processLBlockSet(FrameWork *fw, CFG *cfg, LBlockSet *lbset);
public:
	inline CCGBuilder(FrameWork *framework);

	// CFGProcessor overload
	virtual void processCFG(FrameWork *fw, CFG *cfg );
};

// Inlines
inline CCGBuilder::CCGBuilder(FrameWork *framework)
: fw(framework) {
	assert(fw);
}

}	// otawa

#endif // OTAWA_CACHE_CCGBUILDER_H
