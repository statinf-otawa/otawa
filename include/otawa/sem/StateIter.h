/*
 *	StateIter class interface
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
 *	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 *	02110-1301  USA
 */
#ifndef OTAWA_SEM_STATEITER_H_
#define OTAWA_SEM_STATEITER_H_

#include <otawa/sem/BBIter.h>

namespace otawa { namespace sem {

template <class S>
class StateIter: public PreIterator<StateIter<S>, sem::inst> {
public:
	inline StateIter(S& manager): man(manager) {
		s = man.bot();
		es = man.bot();
	}

	void start(BasicBlock *bb) {
		i.start(bb);
		man.free(s);
		man.free(es);
		s = man.copy(man.state(bb));
		es = man.bot();
		for(int i = 0; i < ss.count(); i++)
			man.free(ss[i]);
		ss.clear();
	}

	void next(void) {
		if(i.ended())
			return;

		if(i.pathEnd()) {				// semantic path end
			es = man.joinPath(es, s);	// join path state
			if(ss)						// end of semantic path
				s = ss.pop();
			else {						// end of instruction
				s = es;
				es = man.bot();
			}
		}

		else {				// next semantic instruction
			if(i.isFork())
				ss.push(man.copy(s));
			s = man.updatePath(s, *i);
		}

		i.next();
	}

	inline void nextInst(void) { next(); while(!i.instEnd()) next(); }
	inline void toEnd(void) { while(!i.ended()) next(); }
	inline bool ended(void) const { return i.ended(); }

	inline Inst *instruction(void) const { return i.instruction(); }
	inline sem::inst item(void) const { return *i; }
	inline typename S::t state(void) { return s; }

private:
	BBIter i;
	S& man;
	typename S::t s, es;
	Vector<typename S::t> ss;
};

} }	// otawa::sem

#endif /* OTAWA_SEM_STATEITER_H_ */
