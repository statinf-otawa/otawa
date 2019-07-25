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

#include <otawa/sem/StateIter.h>

namespace otawa {

class StackProblem;

namespace stack {

typedef enum {
	NONE,
	REG,	// only used for addresses
	SP,
	CST,
	ALL
} kind_t;

class State;

class Value {
public:
	typedef t::uint32	int_t;
	typedef t::int32	signed_t;
	typedef t::uint64 	upper_t;
private:
	static const int_t
		bot_id = -1,
		top_id = -2,
		max_id = 0x80000000;
	typedef t::int32 page_t;
	static const page_t
		spec_page 	= Address::null_page - 2,
		sp_page		= Address::null_page - 1;

	inline Value(page_t page, int_t val = 0): _page(page), _val(val) { }

public:

	static Value bot, top;
	inline Value(void): _page(spec_page), _val(bot_id) { }
	Value(kind_t k, int_t v = 0);
	static inline Value reg(int n) { return Value(spec_page, n); }
	static inline Value addr(Address a) { return Value(a.offset()); }
	static inline Value cst(int_t k) { return Value(0, k); }
	static inline Value sp(int_t off) { return Value(sp_page, off); }
	inline Value& operator=(const Value& val) { _page = val._page; _val = val._val; return *this; }

	inline bool isTop(void) const { return _page == spec_page && _val == top_id; }
	inline bool isBot(void) const { return _page == spec_page && _val == bot_id; }
	inline bool isReg(void) const { return _page == spec_page && _val < max_id; }
	inline bool isSP(void) const { return _page == sp_page; }
	inline bool isConst(void) const { return _page > sp_page; }

	inline bool operator==(const Value& val) const { return _page == val._page && _val == val._val; }
	inline bool operator!=(const Value& val) const { return ! operator==(val); }
	inline bool operator<(const Value& val) const { return (_page < val._page) || (_page == val._page && _val < val._val); }

	kind_t kind(void) const;
	inline int_t value(void) const { return _val; }

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

	static const Value& none, all;

private:
	inline void set(page_t kind, int_t value) { _page = kind; _val = value; }
	page_t _page;
	int_t _val;
};

inline io::Output& operator<<(io::Output& out, const Value& v) { v.print(out); return out; }
io::Output& operator<<(io::Output& out, const State& state);
inline io::Output& operator<<(io::Output& out, const State *state) { return out << *state; }
inline io::Output& operator<<(io::Output& out, State *state) { return out << *state; }

class Manager {
public:
	Manager(WorkSpace *ws);
	~Manager(void);

	// StateManager concept
	typedef State *t;
	t bot(void);
	void free(t s);
	bool equals(t s1, t s2);
	t copy(t s);
	t joinPath(t s1, t s2);
	t state(BasicBlock *bb);
	t updatePath(t s, sem::inst i);

	Value getReg(t s, int i);
	Value getMem(t s, Value addr, int size);

private:
	StackProblem *p;
	State *_bot;
};


class Iter: public sem::StateIter<Manager> {
public:
	Iter(WorkSpace *ws);
	inline Value getReg(int i) { return man->getReg(state(), i); }
	inline Value getMem(Value addr, int size) { return man->getMem(state(), addr, size); }
private:
	Manager *man;
};

extern p::feature ADDRESS_FEATURE;
extern Identifier<Manager *> MANAGER;

}		// stack

extern p::feature STACK_ANALYSIS_FEATURE;

}		// otawa

#endif /* OTAWA_STACK_FEATURES_H_ */
