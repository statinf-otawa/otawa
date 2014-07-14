/*
 *	LoopUnroller processor interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2007, IRIT UPS.
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
 *	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 *	02110-1301  USA
 */
 
#ifndef OTAWA_CFG_LOOPUNROLLER_H_
#define OTAWA_CFG_LOOPUNROLLER_H_

#include <otawa/proc/Processor.h>
#include <otawa/cfg/VirtualCFG.h>
#include <otawa/cfg/CFGCollector.h>
#include <otawa/cfg/VirtualBasicBlock.h>
#include <elm/genstruct/HashTable.h>
#include <otawa/proc/Feature.h>

namespace otawa {

// LoopUnroller class
class LoopUnroller: public Processor {
public:
	static p::declare reg;
	LoopUnroller(p::declare& r = reg);

protected:
	virtual void processWorkSpace(WorkSpace *ws);
	virtual void cleanup(WorkSpace *ws);

private:
	void unroll(otawa::CFG *cfg, BasicBlock *header, VirtualCFG *vcfg);

	elm::genstruct::HashTable<void *, VirtualBasicBlock *> map;		
	CFGCollection *coll;
	int idx;
};

}	// otawa

#endif 	// OTAWA_CFG_LOOPUNROLLER_H_

