/*
 *	CFGInfo class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2004-16, IRIT UPS.
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
#ifndef OTAWA_CFG_CFG_INFO_H
#define OTAWA_CFG_CFG_INFO_H

#include <elm/data/HashMap.h>

#include <otawa/cfg/BasicBlock.h>
#include <otawa/cfg/features.h>

namespace otawa {

using namespace elm;

// Classes
class BasicBlock;
class CFG;
class CodeItem;
class WorkSpace;
class Inst;

// CFGInfo class
class CFGInfo {
	typedef HashMap<Address, CFG *> map_t;
public:
	static Identifier<const CFGInfo *>& ID;

	CFGInfo(WorkSpace *ws);
	virtual ~CFGInfo(void);

	CFG *findCFG(Address addr) const;
	inline CFG *findCFG(Inst *inst) const { return findCFG(inst->address()); }
	CFG *findCFG(String label);

	void add(CFG *cfg);
	void clear(void);

	// Iter class
	class Iter: public map_t::Iter {
	public:
		inline Iter(CFGInfo *info): map_t::Iter(info->_cfgs) { }
	};

private:
	WorkSpace *_ws;
	mutable map_t _cfgs;
};

} // otawa

#endif	// OTAWA_CFG_CFG_INFO_H
