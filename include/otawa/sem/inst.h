/*
 *	$Id$
 *	sem module interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2009, IRIT UPS.
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

#ifndef OTAWA_SEM_INST_H_
#define OTAWA_SEM_INST_H_

#include <elm/io.h>
#include <elm/genstruct/Vector.h>

namespace otawa {

using namespace elm;

namespace hard { class Platform; }

namespace sem {

// type of instruction
// NOTE:	a, b, d, cond, sr, jump, type, addr, reg are field of "inst" class
typedef enum opcode {
	NOP = 0,
	BRANCH,		// perform a branch on content of register a
	TRAP,		// perform a trap
	CONT,		// stop the execution of the block
	IF,			// continue if condition cond is meet in register sr, else skip "jump" instructions
	LOAD,		// reg <- MEM_type(addr)
	STORE,		// MEM_type(addr) <- reg
	SCRATCH,	// d <- T
	SET,		// d <- a
	SETI,		// d <- cst
	SETP,		// page(d) <- cst
	CMP,		// d <- a ~ b
	CMPU,		// d <- a ~u b
	ADD,		// d <- a + b
	SUB,		// d <- a - b
	SHL,		// d <- unsigned(a) << b
	SHR,		// d <- unsigned(a) >> b
	ASR,		// d <- a >> b
	NEG,		// d <- -a
	NOT,		// d <- ~a
	AND,		// d <- a & b
	OR,			// d <- a | b
	XOR,		// d <- a ^ b
	MUL,		// d <- a * b
	MULU,		// d <- unsigned(a) * unsigned(b)
	DIV,		// d <- a / b
	DIVU,		// d <- unsigned(a) / unsigned(b)
	MOD,		// d <- a % b
	MODU,		// d <- unsigned(a) % unsigned(b)
	SPEC		// special instruction (d: code, cst: sub-code)
} opcode;


// type of conditions
typedef enum cond_t {
	NO_COND = 0,
	EQ,
	LT,
	LE,
	GE,
	GT,
	ANY_COND = 8,
	NE,
	ULT,
	ULE,
	UGE,
	UGT,
	MAX_COND
} cond_t;


// types for load and store
typedef enum type_t {
	NO_TYPE = 0,
	INT8 = 1,
	INT16 = 2,
	INT32 = 3,
	INT64 = 4,
	UINT8 = 5,
	UINT16 = 6,
	UINT32 = 7,
	UINT64 = 8,
	FLOAT32 = 9,
	FLOAT64 = 10,
	MAX_TYPE = 11
} type_t;

// inst type
typedef struct inst {
	t::uint16 op;
	t::int16 _d;
	union {
		t::uint32 cst;								// set, seti, setp
		struct { t::int16 a, b;  } regs;			// others
	} args;

	inst(void): op(NOP), _d(0) { }
	inst(opcode _op): op(_op), _d(0) { }
	inst(opcode _op, int d): op(_op)
		{ _d = d; }
	inst(opcode _op, int d, int a): op(_op)
		{ _d = d; args.regs.a = a; }
	inst(opcode _op, int d, int a, int b): op(_op)
		{ _d = d; args.regs.a = a; args.regs.b = b; }

	inline t::int16 d(void) const { return _d; }
	inline t::int16 a(void) const { return args.regs.a; }
	inline t::int16 b(void) const { return args.regs.b; }

	// seti/setp instruction
	inline t::uint32 cst(void) const { return args.cst; }

	// load/store instruction
	inline t::int16 reg(void) const { return d(); }
	inline t::int16 addr(void) const { return a(); }
	inline type_t type(void) const { return type_t(b()); }

	// "if" instruction
	inline cond_t cond(void) const { return cond_t(d()); }
	inline t::int16 sr(void) const { return a(); }
	inline t::uint16 jump(void) const { return b(); }

	void print(elm::io::Output& out) const;
} inst;
inline elm::io::Output& operator<<(elm::io::Output& out, inst i) { i.print(out); return out; }

inline inst nop(void) { return inst(NOP); }
inline inst branch(int to) { return inst(BRANCH, to); }
inline inst trap(void) { return inst(TRAP); }
inline inst cont(void) { return inst(CONT); }
inline inst _if(int cond, int sr, int jump) { ASSERT(cond >= 0 && cond < MAX_COND); return inst(IF, cond, sr, jump); }
inline inst load(int d, int a, int b) { return inst(LOAD, d, a, b); }
inline inst load(int d, int a, type_t b) { return inst(LOAD, d, a, b); }
inline inst store(int d, int a, int b) { return inst(STORE, d, a, b); }
inline inst store(int d, int a, type_t b) { return inst(STORE, d, a, b); }
inline inst scratch(int d) { return inst(SCRATCH, d); }
inline inst set(int d, int a) { return inst(SET, d, a); }
inline inst seti(int d, unsigned long cst) { inst i(SETI, d); i.args.cst = cst; return i; }
inline inst setp(int d, unsigned long cst) { inst i(SETP, d); i.args.cst = cst; return i; }
inline inst cmp(int d, int a, int b) { return inst(CMP, d, a, b); }
inline inst cmpu(int d, int a, int b) { return inst(CMPU, d, a, b); }
inline inst add(int d, int a, int b) { return inst(ADD, d, a, b); }
inline inst sub(int d, int a, int b) { return inst(SUB, d, a, b); }
inline inst shl(int d, int a, int b) { return inst(SHL, d, a, b); }
inline inst shr(int d, int a, int b) { return inst(SHR, d, a, b); }
inline inst asr(int d, int a, int b) { return inst(ASR, d, a, b); }
inline inst neg(int d, int a) { return inst(NEG, d, a); }
inline inst _not(int d, int a) { return inst(NOT, d, a); }
inline inst _and(int d, int a, int b) { return inst(AND, d, a, b); }
inline inst _or(int d, int a, int b) { return inst(OR, d, a, b); }
inline inst mul(int d, int a, int b) { return inst(MUL, d, a, b); }
inline inst mulu(int d, int a, int b) { return inst(MULU, d, a, b); }
inline inst div(int d, int a, int b) { return inst(DIV, d, a, b); }
inline inst divu(int d, int a, int b) { return inst(DIVU, d, a, b); }
inline inst mod(int d, int a, int b) { return inst(MOD, d, a, b); }
inline inst modu(int d, int a, int b) { return inst(MODU, d, a, b); }
inline inst _xor(int d, int a, int b) { return inst(XOR, d, a, b); }

// Block class
class Block: public elm::genstruct::Vector<inst> {
	typedef elm::genstruct::Vector<inst> S;
public:
	class InstIter: public S::Iterator {
	public:
		inline InstIter(const Block& block): S::Iterator(block) { }
		inline InstIter(const InstIter& iter): S::Iterator(iter) { }
		inline opcode op(void) const { return opcode(item().op); }
		inline t::int16 d(void) const { return item().d(); }
		inline t::int16 a(void) const { return item().a(); }
		inline t::int16 b(void) const { return item().b(); }
		inline t::uint32 cst(void) const { return item().cst(); }
	};
	void print(elm::io::Output& out) const;
};
inline elm::io::Output& operator<<(elm::io::Output& out, const Block& b) { b.print(out); return out; }

// Printer class
class Printer {
public:
	inline Printer(const hard::Platform *platform = 0): pf(platform) { }

	void print(elm::io::Output& out, const Block& block) const;
	void print(elm::io::Output& out, const inst& inst) const;
private:
	const hard::Platform *pf;
};

// useful functions
cond_t invert(cond_t cond);
int size(type_t type);
io::Output& operator<<(io::Output& out, type_t type);
io::Output& operator<<(io::Output& out, cond_t cond);

} }	// otawa::sem

#endif /* OTAWA_SEM_INST_H_ */
