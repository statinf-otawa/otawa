/*
 *	VirtualCFG class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	OTAWA is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	OTAWA is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with OTAWA; if not, write to the Free Software
 *	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
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
	VirtualCFG(bool addEntryExit = true);
	inline CFG *cfg(void) const { return _cfg; }
	void addBB(BasicBlock *bb);
	void removeBB(BasicBlock *bb);
	void numberBBs(void);
};

} // otawa

#endif	// OTAWA_CFG_VIRTUAL_CFG_H
