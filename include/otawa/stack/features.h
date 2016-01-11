/*
 *	stack module features
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2015, IRIT UPS.
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
#ifndef OTAWA_STACK_FEATURES_H_
#define OTAWA_STACK_FEATURES_H_

#include <otawa/sem/PathIter.h>

namespace otawa {

class StackProblem;

namespace stack {

class State;

typedef enum {
	NONE,
	REG,	// only used for addresses
	SP,
	CST,
	ALL
} kind_t;

class Value {
public:
	inline Value(kind_t kind = NONE, unsigned long value = 0): _kind(kind), _value(value) { }
	inline Value(const Value& val): _kind(val._kind), _value(val._value) { }
	inline Value& operator=(const Value& val) { _kind = val._kind; _value = val._value; return *this; }
	static inline Value reg(int n) { return Value(REG, n); }

	inline bool operator==(const Value& val) const { return _kind == val._kind && _value == val._value; }
	inline bool operator!=(const Value& val) const { return ! operator==(val); }
	inline bool operator<(const Value& val) const { return (_kind < val._kind) || (_kind == val._kind && _value < val._value); }

	inline kind_t kind(void) const { return _kind; }
	inline unsigned long value(void) const { return _value; }

	void add(const Value& val);
	void sub(const Value& val);
	void shl(const Value& val);
	void shr(const Value& val);
	void asr(const Value& val);
	void neg(void);
	void _not(void);
	void _or(const Value& val);
	void _and(const Value& val);
	void _xor(const Value& val);
	void mul(const Value& val);
	void div(const Value& val);
	void divu(const Value& val);
	void mod(const Value& val);
	void modu(const Value& val);
	void mulh(const Value& val);
	void join(const Value& val);
	void print(io::Output& out) const;

	static const Value none, all;

private:
	typedef t::uint32 	int_t;
	typedef t::int32	signed_t;
	typedef t::uint64 	upper_t;
	inline void set(kind_t kind, int_t value) { _kind = kind; _value = value; }
	kind_t _kind;
	int_t _value;
};

inline io::Output& operator<<(io::Output& out, const Value& v) { v.print(out); return out; }
io::Output& operator<<(io::Output& out, const State& state);


class Iter: public PreIterator<Iter, sem::inst> {
public:
	Iter(WorkSpace *ws);
	~Iter(void);

	void start(BasicBlock *bb);

	inline bool pathEnd(void) const { return si.pathEnd(); }
	inline bool isCond(void) const { return si.isCond(); }
	inline bool instEnd(void) const { return si.ended(); }

	inline bool ended(void) const { return i.ended() && si.ended(); }
	inline sem::inst item(void) const { return si.item(); }
	void next(void);
	void nextInst(void);

	inline State& state(void) const { return *s; }
	Value getReg(int i);
	Value getMem(Value addr, int size);
	inline Inst *instruction(void) const { return *i; }

private:
	sem::Block b;
	sem::PathIter si;
	BasicBlock::InstIter i;
	State *s, *es;
	genstruct::Vector<State *> ss;
	StackProblem *p;
};

extern p::feature ADDRESS_FEATURE;

}		// stack

extern p::feature STACK_ANALYSIS_FEATURE;

}		// otawa

#endif /* OTAWA_STACK_FEATURES_H_ */
