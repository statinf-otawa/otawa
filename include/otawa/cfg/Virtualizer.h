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

#ifndef OTAWA_CFG_VIRTUALIZER_H_
#define OTAWA_CFG_VIRTUALIZER_H_

#include <elm/data/HashMap.h>
#include <elm/util/Option.h>
#include <otawa/proc/Processor.h>
#include <otawa/cfg/features.h>
#include <otawa/prop/ContextualProperty.h>


namespace otawa {

using namespace elm;
class CFGCollection;

// Virtualizer class
class Virtualizer: public Processor {

public:
	Virtualizer(void);
	static p::declare reg;

	void configure(const PropList& props) override;
	void *interfaceFor(const AbstractFeature& f) override;

protected:
	void processWorkSpace(WorkSpace *ws) override;
	void cleanup(WorkSpace *ws) override;
	void commit(WorkSpace *ws) override;
	void destroy(WorkSpace *ws) override;

private:
	void make(struct call_t *stack, CFG *cfg, CFGMaker& maker, elm::Option<int> local_inlining, ContextualPath &path);
	void makeCFG(struct call_t *call, CFG *cfg, Option<int> local_inlining);
	CFGMaker& makerOf(CFG *cfg);
	CFGMaker& newMaker(Inst *first);
	bool isInlined(CFG* cfg, Option<int> local_inlining, ContextualPath &path);
	bool virtualize;
	CFG *entry;
	List<CFG *> todo;
	HashMap<CFG *, CFGMaker *> map;
	FragTable<CFGMaker *> makers;
	CFGCollection *coll;
};

}	// otawa

#endif	// OTAWA_CFG_VIRTUALIZER_H_
