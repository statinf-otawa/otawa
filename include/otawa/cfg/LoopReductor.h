/*
 *	LoopReductor processor interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2007-16, IRIT UPS.
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

#ifndef OTAWA_CFG_LOOPREDUCTOR_H_
#define OTAWA_CFG_LOOPREDUCTOR_H_

#include <elm/data/Vector.h>
#include <otawa/cfg/features.h>
#include <otawa/dfa/BitSet.h>
#include <otawa/proc/Feature.h>
#include <otawa/proc/Processor.h>

namespace otawa {

class LoopReductor: public Processor {
public:
	static p::declare reg;
	LoopReductor(p::declare& r = reg);
	void *interfaceFor(const AbstractFeature& f) override;

protected:
	void processWorkSpace(WorkSpace *ws) override;
	void commit(WorkSpace *ws) override;
	void destroy(WorkSpace *ws) override;

private:
	typedef Vector<dfa::BitSet *> loops_t;
	bool reduce(CFGMaker& G, loops_t& L);
	Block *clone(CFGMaker& G, Block *b, bool duplicate = false);
	void computeInLoops(CFGMaker& maker, loops_t& L);

	Vector<CFGMaker *> vcfgvec;
	CFGCollection *coll;

	static Identifier<bool> MARK;
	static Identifier<Block*> DUPLICATE_OF;
	static Identifier<dfa::BitSet*> IN_LOOPS;
};

}	// otawa

#endif	//. OTAWA_CFG_LOOPREDUCTOR_H_

