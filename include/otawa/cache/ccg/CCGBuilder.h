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
	bool _explicit;
	int vars;
	void processLBlockSet(FrameWork *fw, CFG *cfg, LBlockSet *lbset);
	void initialize(const PropList& props);

public:
	static Identifier ID_NonConflict;
	static Identifier ID_Node;
	static Identifier ID_HitVar;
	static Identifier ID_MissVar;
	static Identifier ID_BBVar;

	CCGBuilder(const PropList& props = PropList::EMPTY);

	// CFGProcessor overload
	virtual void processCFG(FrameWork *fw, CFG *cfg );
	virtual void configure(const PropList& props);
};

}	// otawa

#endif // OTAWA_CACHE_CCGBUILDER_H
