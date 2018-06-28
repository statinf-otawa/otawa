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

#include <elm/data/Vector.h>
#include <otawa/data/clp/ClpState.h>
#include <otawa/prog/Process.h>

namespace otawa {

namespace clp {

class ClpProblem;


/** A pack of CLP states inside a BasicBlock.
*	Use it to retrieve the state of a specific instruction or semantic
*	instruction.
*	class ClpStatePAck
*	    |----InstPack: to associate CLP states for each semantic instruction of a given instruction
*	    |----Context
*/
class ClpStatePack{
	friend class ClpProblem;
public:
	typedef Vector<clp::State*>::Iter StateIterator;
	
	/** A pack of CLP states inside a machine instruction.
	*	Each state of this pack is the state after a semantic instruction, in
	*	the order of the semantic instructions.
	*/
	class InstPack{
	friend class ClpStatePack; friend class ClpProblem;
	public:
		/** Constructor for a new instruction pack */
		inline InstPack(): _inst_addr(0), _states() {}
		/** Constructor for a new instruction pack
		*	@param inst the address of the instruction of this pack
		*/
		inline InstPack(address_t inst): _inst_addr(inst), _states() {}
		/** Destructor */
		~InstPack(void);
		/** @return the address of the instruction of this Pack */
		inline address_t inst_addr(void) { return _inst_addr; }
		/** @return an iterator over the states of this Pack.*/
		inline StateIterator getIterator(void) { return StateIterator(_states);}
		/** @return the last output state of this pack (i.e. the output state
		*	of the machine instruction of this pack).
		*/
		clp::State& outputState(void) { 
			ASSERT(_states.length() > 0);
			return *(_states[_states.length() - 1]);
		}
		/** @return if the InstPack is empty */
		bool isEmpty(void){ return _states.length() == 0; }
		void append(clp::State &state);
	private:
		address_t _inst_addr;
		Vector<clp::State*> _states;
	};

	class Context {
	public:
		Context(Process *process);
		Context(ClpProblem& problem);
		~Context(void);
		inline ClpProblem& problem(void) const { return *prob; }
	private:
		ClpProblem *prob;
		bool to_free;
	};
	
	typedef Vector<InstPack*>::Iter PackIterator;
	
	ClpStatePack(BasicBlock *bb, Process *process);
	ClpStatePack(BasicBlock *bb, const Context& context);
	~ClpStatePack(void);
	
	/** @return the BasicBlock corresponding to this Pack. */
	inline BasicBlock *bb(void){ return _bb; }
	/** @return an iterator over instruction packs. */
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
