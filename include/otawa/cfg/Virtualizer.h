/*
 *	$Id$
 *	Virtualizer processor (function inliner)
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

#ifndef CFG_VIRTUALIZER_H_
#define CFG_VIRTUALIZER_H_

#include <otawa/proc/Processor.h>
#include <otawa/proc/Feature.h>
#include <otawa/prop/Identifier.h>
#include <elm/genstruct/HashTable.h>
#include <otawa/cfg/CFG.h>
#include <otawa/cfg/VirtualCFG.h>
#include <otawa/cfg/BasicBlock.h>
#include <otawa/prog/WorkSpace.h>
#include <otawa/prop/PropList.h>
namespace otawa {



class Virtualizer: public Processor {

	public:
	Virtualizer(void);
	virtual void processWorkSpace(WorkSpace*);
	virtual void configure(const PropList& props);
	
	
	private:	
	void virtualize(struct call_t*, CFG *cfg, VirtualCFG *vcfg, BasicBlock *entry, BasicBlock *exit);
	bool isInlined();
	bool virtual_inlining;
	CFG *entry;
	elm::genstruct::HashTable<void *, VirtualCFG *> cfgMap;
};

extern Feature<Virtualizer> VIRTUALIZED_CFG_FEATURE;
extern Identifier<bool> VIRTUAL_INLINING;
extern Identifier<BasicBlock*> VIRTUAL_RETURN_BLOCK;
}


#endif 
