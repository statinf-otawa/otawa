/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	otawa/cache/CCGConstraintBuilder.h -- interface of CCGConstraintBuilder class.
 */
#ifndef OTAWA_CACHE_CCGCONSTRAINTBUILDER_H
#define OTAWA_CACHE_CCGCONSTRAINTBUILDER_H

#include <assert.h>
#include <otawa/proc/CFGProcessor.h>
#include <otawa/prop/Identifier.h>
#include <otawa/util/ContextTree.h>
#include <otawa/hard/Cache.h>

namespace otawa {

// Extern class
class LBlockSet;
class CFG;
class LBlock;
class CCGNode;

// CCGConstraintBuilder class
class CCGConstraintBuilder: public CFGProcessor {
	FrameWork *fw;
	void processLBlockSet(CFG *cfg, LBlockSet *lbset);
	void addConstraintHeader(CFG *cfg, LBlockSet *graph, ContextTree *ct,
		LBlock *boc);
public:
	inline CCGConstraintBuilder(FrameWork *framework);
	
	// CFGProcessor overload
	virtual void processCFG(FrameWork *fw, CFG *cfg);
};

// Inlines
inline CCGConstraintBuilder::CCGConstraintBuilder(FrameWork *framework)
: fw(framework) {
		assert(fw);
}

}	// otawa


#endif // OTAWA_CACHE_CCGCONSTRAINTBUILDER_H
