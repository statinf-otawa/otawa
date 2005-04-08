/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	util/DFA.h -- DFA class interface.
 */
#ifndef OTAWA_UTIL_DFA_H
#define OTAWA_UTIL_DFA_H

#include <otawa/properties.h>

namespace otawa {

// Extern class
class CFG;
class BasicBlock;

// DFASet abstract class
class DFASet {
public:
	virtual ~DFASet(void) = 0;
	virtual void reset(void) = 0;
	virtual bool equals(DFASet *set) = 0;
	virtual void add(DFASet *set) = 0;
	virtual void remove(DFASet *set) = 0;
};

// DFA class
class DFA {
	void startup(CFG *cfg);
	void cleanup(CFG *cfg, Identifier *in_id, Identifier *out_id);
public:
	void resolve(CFG *cfg, Identifier *in_id = 0, Identifier *out_id = 0);
	virtual DFASet *initial(void) = 0;
	virtual DFASet *generate(BasicBlock *bb) = 0;
	virtual DFASet *kill(BasicBlock *bb) = 0;
};

} // otawa

#endif	// OTAWA_UTIL_DFA_H
