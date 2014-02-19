/*
 *	PathIter class -- semantics path iterator
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2014, IRIT UPS.
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
#ifndef OTAWA_SEM_PATHITER_H_
#define OTAWA_SEM_PATHITER_H_

#include <otawa/prog/Inst.h>
#include <otawa/sem/inst.h>

namespace otawa { namespace sem {

class PathIter: public PreIterator<PathIter, sem::inst> {
public:

	inline void start(Inst *inst) {
		b.clear();
		_inst = inst;
		inst->semInsts(b);
		todo.clear();
		pc = 0;
		b.add(sem::cont());
	}

	inline bool pathEnd(void) const { return b[pc].op == sem::CONT; }
	inline bool isCond(void) const { return b[pc].op == sem::IF; }

	inline bool ended(void) const { return pathEnd() && !todo; }
	inline sem::inst item(void) const { return b[pc]; }
	inline void next(void) {
		if(pathEnd())
			pc = todo.pop();
		else {
			if(isCond())
				todo.push(pc + b[pc].jump());
			pc++;
		}
	}

private:
	Inst *_inst;
	sem::Block b;
	genstruct::Vector<int> todo;
	int pc;
};

} }	// otawa::sem

#endif /* OTAWA_SEM_PATHITER_H_ */
