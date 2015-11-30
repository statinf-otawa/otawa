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
	static bool dominates(Block *bb1, Block *bb2);
	static inline bool isDominated(Block *bb1, Block *bb2) { return dominates(bb2, bb1); }
	static bool isLoopHeader(Block *bb);
	static bool isBackEdge(Edge *edge);

	// Constructor
	Dominance(void);

protected:
	virtual void processCFG(WorkSpace *fw, CFG *cfg);
	virtual void cleanup(WorkSpace *ws);
private:
	void markLoopHeaders(CFG *cfg);
};

// Features
extern Identifier<const dfa::BitSet *> REVERSE_DOM;

} // otawa

#endif // OTAWA_UTIL_DOMINANCE_H
