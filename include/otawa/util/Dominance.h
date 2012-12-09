/*
 * $Id$
 * Copyright (c) 2005 IRIT-UPS
 *
 * include/util/Dominance.h -- Dominance class interface.
 */
#ifndef OTAWA_UTIL_DOMINANCE_H
#define OTAWA_UTIL_DOMINANCE_H

#include <otawa/proc/CFGProcessor.h>
#include <otawa/proc/Feature.h>
#include <otawa/cfg/features.h>

namespace otawa {

// External
class BasicBlock;
class Edge;
namespace dfa { class BitSet; }

// Dominance class
class Dominance: public CFGProcessor {
public:
	static void ensure(CFG *cfg);
	static bool dominates(BasicBlock *bb1, BasicBlock *bb2);
	static inline bool isDominated(BasicBlock *bb1, BasicBlock *bb2);
	static bool isLoopHeader(BasicBlock *bb);
	static bool isBackEdge(Edge *edge);

	// Constructor
	Dominance(void);

protected:
	virtual void processCFG(WorkSpace *fw, CFG *cfg);
private:
	void markLoopHeaders(CFG *cfg);
};

// Features
extern SilentFeature DOMINANCE_FEATURE;
extern Identifier<dfa::BitSet *> REVERSE_DOM;

} // otawa

#endif // OTAWA_UTIL_DOMINANCE_H
