/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	otawa/cfg/VirtualCFG.h -- interface of VirtualCFG class.
 */
#ifndef OTAWA_CFG_VIRTUAL_CFG_H
#define OTAWA_CFG_VIRTUAL_CFG_H

#include <otawa/cfg/CFG.h>

namespace otawa {
	
// VirtualCFG class
class VirtualCFG: public CFG {
	CFG *_cfg;
	void virtualize(struct call_t *stack, CFG *cfg,
		BasicBlock *entry, BasicBlock *exit);
protected:
	virtual void scan(void);
public:
	VirtualCFG(CFG *cfg, bool inlined = true);
	VirtualCFG();
	inline CFG *cfg(void) const;
	void addBB(BasicBlock *bb);
	void numberBBs(void);
};

// Identifiers
extern Identifier<CFG *> CALLED_CFG;
extern Identifier<bool> RECURSIVE_LOOP;
extern Identifier<bool> DONT_INLINE;

// Inlines
inline CFG *VirtualCFG::cfg(void) const {
	return _cfg;
}

} // otawa

#endif	// OTAWA_CFG_VIRTUAL_CFG_H
