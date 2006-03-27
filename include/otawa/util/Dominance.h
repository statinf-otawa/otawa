/*
 * $Id$
 * Copyright (c) 2005 IRIT-UPS
 *
 * include/util/Dominance.h -- Dominance class interface.
 */
#ifndef OTAWA_UTIL_DOMINANCE_H
#define OTAWA_UTIL_DOMINANCE_H

#include <elm/Collection.h>
#include <otawa/proc/CFGProcessor.h>

namespace otawa {

// External
class BasicBlock;

// Dominance class
class Dominance: public CFGProcessor {
public:
	static Identifier ID_RevDom;
	static Identifier ID_LoopHeader;
	static void ensure(CFG *cfg);
	static bool dominates(BasicBlock *bb1, BasicBlock *bb2);
	static inline bool isDominated(BasicBlock *bb1, BasicBlock *bb2);
	static bool isLoopHeader(BasicBlock *bb);
	static void markLoopHeaders(CFG *cfg,
		elm::MutableCollection<BasicBlock *> *headers = 0);
	
	// CFGProcessor overload
	virtual void processCFG(FrameWork *fw, CFG *cfg);
};

} // otawa

#endif // OTAWA_UTIL_DOMINANCE_H
