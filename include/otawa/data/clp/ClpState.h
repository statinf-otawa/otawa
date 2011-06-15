/*
 *	$Id$
 *	CLP State definition
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

#ifndef OTAWA_DATA_CLP_STATE_H_
#define OTAWA_DATA_CLP_STATE_H_

#include <elm/genstruct/Vector.h>
#include <otawa/data/clp/ClpValue.h>

namespace otawa {

namespace hard { class Register; }

namespace clp {
	/**
	 * The abstract state of the computer (abstract domain). The abstract state
	 * is a list of register states and memory states.
	*/
	class State {
	public:
		
		/**
		 * A node in the memory list
		*/
		class Node {
		public:
			friend class State;
			inline Node(void): next(0), addr(0), val(ALL) { }
			inline Node(const int address, const Value& value)
				: next(0), addr(address), val(value) { }
			inline Node(const Node *node)
				: next(0), addr(node->addr), val(node->val) { }
		private:
			Node *next;
			int addr;
			Value val;
		};
		
		/** Constructors of a new State	*/
		State(const Value& def = Value::all): first(0, def) {}
		/** Copy constructor */
		State(const State& state): first(0, Value::all){copy(state);}
		/** Destructor */
		~State(void) { clear(); }
		
		inline State& operator=(const State& state){copy(state); return *this; }
		inline bool operator==(const State& state) const {
			return equals(state);
		}
		
		void copy(const State& state);
		void clear(void);
		void set(const Value& addr, const Value& val);
		bool equals(const State& state) const;
		void join(const State& state);
		void widening(const State& state, int loopBound);
		void print(io::Output& out) const;
		const Value& get(const Value& addr) const;
		
		static const State EMPTY, FULL;
	
	protected:
		Node first;
		genstruct::Vector<Value> registers;
		genstruct::Vector<Value> tmpreg;
	};
	
}	// clp

}	// otawa

#endif /* OTAWA_DATA_CLP_STATE_H_ */
