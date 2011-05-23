/*
 *	$Id$
 *	CLP Pack definition
 *	
 *	This file is part of OTAWA
 *	Copyright (c) 2011, IRIT UPS.
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

#ifndef OTAWA_DATA_CLP_PACK_H_
#define OTAWA_DATA_CLP_PACK_H_

#include <elm/genstruct/Vector.h>
#include <otawa/data/clp/ClpState.h>

namespace otawa {

namespace clp {

/** A pack of Clp State inside a Basic Block.
*	Use it to retreive the state of a specific instruction or semantic
*	instruction.
*/
class ClpStatePack{
friend class ClpProblem;
public:
	typedef Vector<clp::State*>::Iterator StateIterator;
	
	class InstPack{
	friend class ClpStatePack; friend class ClpProblem;
	public:
		inline InstPack(): _inst_addr(0), _states() {}
		inline InstPack(address_t inst): _inst_addr(inst), _states() {}
		~InstPack(void);
		inline address_t inst_addr(void) { return _inst_addr; }
		inline StateIterator getIterator(void) { return StateIterator(_states);}
		clp::State& outputState(void) { return *(_states[_states.length() - 1]);}
		void append(clp::State &state);
	private:
		address_t _inst_addr;
		Vector<clp::State*> _states;
	};
	
	typedef Vector<InstPack*>::Iterator PackIterator;
	
	ClpStatePack(BasicBlock *bb);
	~ClpStatePack(void);
	
	inline BasicBlock *bb(void){ return _bb; }
	inline PackIterator getIterator(void) { return PackIterator(_packs); }
	
	clp::State state_after(address_t instruction);
	clp::State state_after(address_t instruction, int sem);
	clp::State state_before(address_t instruction);
	clp::State state_before(address_t instruction, int sem);
	
	InstPack* newPack(address_t inst);
	
private:
	BasicBlock *_bb;
	Vector<InstPack*> _packs;
};

}	// clp

}	// otawa

#endif /* OTAWA_DATA_CLP_PACK_H_ */
