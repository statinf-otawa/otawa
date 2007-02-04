/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	util/DFA.h -- DFA class interface.
 */
#ifndef OTAWA_UTIL_DFA_H
#define OTAWA_UTIL_DFA_H

#include <otawa/properties.h>

#warning "Use of the DFA class is deprecared. Use DFAEngine instead."

namespace otawa {

// Extern class
class CFG;
class BasicBlock;

// DFASet abstract class
class DFASet {
public:
	virtual ~DFASet(void) { };
	virtual bool equals(DFASet *set) = 0;
	virtual void add(DFASet *set) = 0;
	virtual void remove(DFASet *set) = 0;
};

// DFA class
class DFA {
	void startup(CFG *cfg);
	void cleanup(CFG *cfg,
		AbstractIdentifier *in_id,
		AbstractIdentifier *out_id);
public:
	void resolve(CFG *cfg,
		AbstractIdentifier *in_id = 0,
		AbstractIdentifier *out_id = 0);
	virtual DFASet *initial(void) = 0;
	virtual DFASet *generate(BasicBlock *bb) = 0;
	virtual DFASet *kill(BasicBlock *bb) = 0;
	virtual void clear(DFASet *set) = 0;
	virtual void merge(DFASet *acc, DFASet *set) = 0;
};

} // otawa

#endif	// OTAWA_UTIL_DFA_H
