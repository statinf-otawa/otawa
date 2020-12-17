/*
 *	dfa::State class -- state description for data flow analysis.
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
#ifndef OTAWA_DFA_STATE_H_
#define OTAWA_DFA_STATE_H_

#include <elm/avl/Map.h>
#include <elm/stree/Tree.h>
#include <otawa/base.h>
#include <otawa/prog/Process.h>

namespace otawa {

namespace hard { class Register; }

namespace dfa {

using namespace elm;

class Value {
public:
	typedef enum {
		NONE = 0,
		CONST = 1,
		INTERVAL = 2,
		CLP = 3
	} kind_t;

	Value(void);
	Value(t::uint32 base);
	Value(t::uint32 lo, t::uint32 up);
	Value(t::uint32 base, t::uint32 delta, t::uint32 count);
	static Value parse(const string& str);

	inline kind_t kind(void) const { return _kind; }
	inline operator bool(void) const { return _kind != NONE; }
	inline bool isConst(void) const { return _kind > NONE && _kind <= CONST; }
	inline bool isInterval(void) const { return _kind > NONE && _kind <= INTERVAL; }
	inline bool isCLP(void) const { return _kind > NONE && _kind <= CLP; }

	inline t::uint32 value(void) const { return _base; }
	inline t::uint32 base(void) const { return _base; }
	inline t::uint32 delta(void) const { return _delta; }
	inline t::uint32 count(void) const { return _count; }
	inline t::uint32 low(void) const { return _base; }
	inline t::uint32 up(void) const { return _base + _delta * _count; }

private:
	kind_t _kind;
	t::uint32 _base;
	t::uint32 _delta;
	t::uint32 _count;
};


inline io::Output& operator<<(io::Output& out, const Value &val)
{
	if ((val.delta() == 0) && (val.count() ==  0))
		out << "k(0x" << io::hex(val.base()) << ')';
	else {
		if(val.base() >= 0)
			out << "(0x" << io::hex(val.base());
		else {
			t::int32 _baseToPrint = 0 - val.base();
			out << "(-0x" << io::hex(_baseToPrint);
		}
		if(val.delta() >= 0)
			out << ", 0x" << io::hex(val.delta());
		else {
			t::int32 _deltaToPrint =  0 - val.delta();
			out << ", -0x" << io::hex(_deltaToPrint);
		}
		if(val.count() == 0xFFFFFFFF)
			out << ", inf)";
		else
			out << ", 0x" << io::hex(val.count()) << ')';
	}
	return out;
}


class MemCell {
public:
	inline MemCell(void): _type(&Type::no_type) { }
	inline MemCell(Address addr, const Type *type, Value val): _addr(addr), _type(type), _val(val) { }
	inline Address address(void) const { return _addr; }
	inline const Type *type(void) const { return _type; }
	inline const Value& value(void) const { return _val; }
private:
	Address _addr;
	const Type *_type;
	Value _val;
};

class State {
	typedef avl::Map<const hard::Register *, Value> reg_map_t;
	typedef avl::Map<Address, MemCell> mem_map_t;
public:

	typedef t::uint32 address_t;
	State(Process& process);
	inline Process& process(void) const { return proc; }

	// accessing registers
	void set(const hard::Register *reg, Value value);
	Value get(const hard::Register *reg) const;
	class RegIter: public reg_map_t::PairIter {
	public:
		inline RegIter(State *state): reg_map_t::PairIter(state->regs) { }
		inline RegIter(const RegIter& i): reg_map_t::PairIter(i) { }
	};

	// accessing configured memory
	void record(const MemCell& cell);
	class MemIter: public mem_map_t::Iter {
	public:
		inline MemIter(const State *state): mem_map_t::Iter(state->cmem) { }
		inline MemIter(const MemIter& iter): mem_map_t::Iter(iter) { }
	};

	// accessing initialized memory
	inline bool isReadOnly(Address addr) const { return mem.contains(addr.offset()); }
	inline void get(const Address& a, t::uint8 &v) const { proc.get(a, v); }
	inline void get(const Address& a, t::uint16 &v) const { proc.get(a, v); }
	inline void get(const Address& a, t::uint32 &v) const { proc.get(a, v); }
	inline void get(const Address& a, t::uint64 &v) const { proc.get(a, v); }
	inline void get(const Address& a, float &v) const { proc.get(a, v); }
	inline void get(const Address& a, double &v) const { proc.get(a, v); }
	inline void get(const Address& a, Address &v) const { proc.get(a, v); }

private:
	reg_map_t regs;
	stree::Tree<address_t, bool> mem;
	mem_map_t cmem;
	Process& proc;
};

extern p::id<Pair<const hard::Register *, Value> > REG_INIT;
extern p::id<MemCell> MEM_INIT;
extern p::interfaced_feature<State> INITIAL_STATE_FEATURE;
extern p::id<State *> INITIAL_STATE;

} }		// otawa::dfa

#endif /* OTAWA_DFA_STATE_H_ */
