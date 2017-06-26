/*
 *	AbstractCFGBuilder processor interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2016, IRIT UPS.
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
#ifndef OTAWA_CFG_ABSTRACT_CFG_BUILDER_H
#define OTAWA_CFG_ABSTRACT_CFG_BUILDER_H

#include <elm/data/FragTable.h>
#include <elm/data/Vector.h>
#include <otawa/cfg/features.h>
#include <otawa/proc/Processor.h>

namespace otawa {

// AbstractCFGBuilder Class
class AbstractCFGBuilder: public Processor {
	typedef FragTable<Pair<Inst *, CFGMaker *> > makers_t;
public:
	AbstractCFGBuilder(p::declare& r);
	virtual void configure(const PropList& props);

protected:
	void processWorkSpace(WorkSpace *ws);
	virtual void setup(WorkSpace *ws);
	virtual void cleanup(WorkSpace *ws);
	CFGMaker &maker(Inst *i);

	class Iter: public PreIterator<Iter, CFGMaker *> {
	public:
		inline Iter(AbstractCFGBuilder& b): i(b.makers) { }
		inline bool ended(void) const { return i.ended(); }
		inline CFGMaker *item(void) const { return (*i).snd; }
		inline void next(void) { i.next(); }
	private:
		makers_t::Iter i;
	};

private:
	void processCFG(Inst *i);
	void scanCFG(Inst *i, FragTable<Inst *>& bbs);
	void buildBBs(CFGMaker& maker, const FragTable<Inst *>& bbs);
	void buildEdges(CFGMaker& maker);
	void cleanBBs(const FragTable<Inst *>& bbs);
	void seq(CFGMaker& m, BasicBlock *b, Block *src);

	makers_t makers;
	Bag<Address> bounds;
};

} // otawa

#endif // OTAWA_CFG_CFG_COLLECTOR_H
