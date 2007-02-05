/*
 * $Id$
 * Copyright (c) 2005-07 IRIT-UPS
 *
 * DFAEngine class interface
 */
#ifndef OTAWA_UTIL_DFAENGINE_H
#define OTAWA_UTIL_DFAENGINE_H

#include <otawa/dfa/IterativeDFA.h>

#warning "Deprecared header and classes. Use otawa/dfa/IterativeDFA.h instead."

namespace otawa { namespace util {

// DFAPredecessor
class DFAPredecessor: public dfa::Predecessor {
public:
	inline DFAPredecessor(BasicBlock *bb): dfa::Predecessor(bb) { }
};


// DFASuccessor
class DFASuccessor: public dfa::Successor {
public:
	inline DFASuccessor(BasicBlock *bb): dfa::Successor(bb) { }
};


// DFAEngine class
template <class Problem, class Set, class Iter = DFAPredecessor>
class DFAEngine: public dfa::IterativeDFA<Problem, Set, Iter> {
public:
	inline DFAEngine(Problem& problem, CFG& cfg)
		: dfa::IterativeDFA<Problem, Set, Iter>(problem, cfg) { }
};

} } // otawa::util

#endif // OTAWA_UTIL_DFAENGINE_H

