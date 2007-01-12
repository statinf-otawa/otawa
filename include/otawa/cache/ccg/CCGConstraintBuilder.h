/*
 *	$Id$
 *	Copyright (c) 2005-07, IRIT UPS.
 *
 *	CCGConstraintBuilder class interface
 */
#ifndef OTAWA_CACHE_CCGCONSTRAINTBUILDER_H
#define OTAWA_CACHE_CCGCONSTRAINTBUILDER_H

#include <assert.h>
#include <otawa/proc/Processor.h>
#include <otawa/hard/Cache.h>

namespace otawa {

// Extern class
class CCGNode;
class CFG;
class ContextTree;
class LBlock;
class LBlockSet;
namespace ilp {
	class System;
	class Var;
}

namespace ipet {

// CCGConstraintBuilder class
class CCGConstraintBuilder: public Processor {
	bool _explicit;
	void processLBlockSet(FrameWork *fw, LBlockSet *lbset);
	void addConstraintHeader(ilp::System *system, LBlockSet *graph, ContextTree *ct,
		LBlock *boc);
public:
	CCGConstraintBuilder(void);
	
	// CFGProcessor overload
	virtual void processFrameWork(FrameWork *fw);
	virtual void configure(const PropList& props = PropList::EMPTY);
};

} }	// otawa::ipet

#endif // OTAWA_CACHE_CCGCONSTRAINTBUILDER_H
