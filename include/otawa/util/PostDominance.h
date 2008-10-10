/*
 * $Id$
 * Copyright (c) 2005 IRIT-UPS
 *
 * include/util/PostDominance.h -- PostDominance class interface.
 */
#ifndef OTAWA_UTIL_POSTDOMINANCE_H
#define OTAWA_UTIL_POSTDOMINANCE_H

#include <otawa/proc/CFGProcessor.h>
#include <otawa/proc/Feature.h>

namespace otawa {

// External
class BasicBlock;
namespace dfa {
	class BitSet;
}

// PostDominance class
class PostDominance: public CFGProcessor {
public:
	static void ensure(CFG *cfg);
	static bool postDominates(BasicBlock *bb1, BasicBlock *bb2);
	static inline bool isPostDominated(BasicBlock *bb1, BasicBlock *bb2);

	// Constructor
	PostDominance(void);
	
	// CFGProcessor overload
	virtual void processCFG(WorkSpace *fw, CFG *cfg);
};

// Features
extern Feature<PostDominance> POSTDOMINANCE_FEATURE;

// Properties
extern Identifier<dfa::BitSet *> REVERSE_POSTDOM;

} // otawa

#endif // OTAWA_UTIL_POSTDOMINANCE_H
