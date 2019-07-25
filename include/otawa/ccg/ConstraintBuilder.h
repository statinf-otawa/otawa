/*
 *	ConstraintBuilder class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2006, IRIT UPS.
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
#ifndef OTAWA_CCG_CCGCONSTRAINTBUILDER_H
#define OTAWA_CCG_CCGCONSTRAINTBUILDER_H

#include <elm/assert.h>
#include <otawa/proc/Processor.h>
#include <otawa/hard/Cache.h>
#include <otawa/ccg/features.h>

namespace otawa {

// Extern class
class CFG;
class ContextTree;
class LBlock;
class LBlockSet;
namespace ilp {
	class System;
	class Var;
}

namespace ccg {

class Node;

// ConstraintBuilder class
class ConstraintBuilder: public Processor {
public:
	static p::declare reg;
	ConstraintBuilder(p::declare& r = reg);
	virtual void processWorkSpace(WorkSpace *fw);
	virtual void configure(const PropList& props = PropList::EMPTY);

private:
	bool _explicit;
	void processLBlockSet(WorkSpace *fw, LBlockSet *lbset);
	void addConstraintHeader(ilp::System *system, LBlockSet *graph, ContextTree *ct,
		LBlock *boc);
};

} }	// otawa::ccg

#endif // OTAWA_CCG_CCGCONSTRAINTBUILDER_H
