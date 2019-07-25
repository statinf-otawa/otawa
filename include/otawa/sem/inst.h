/*
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
#include <elm/data/Vector.h>

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
	IF,			// continue if condition cond is met in register sr, else skip "jump" instructions (deprecated)
	LOAD,		// rd <- MEM_rb(ra)
	STORE,		// MEM_rb(ra) <- rd
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
	SPEC,		// special instruction (d: code, cst: sub-code)
	MULH,		// d <- (a * b) >> bitlength(d)
	ASSUME,		// assume that cond in sr is true
	FORK		// creates two executions paths: one at pc+1, other at pc + jump
} opcode;


// type of conditions
typedef enum cond_t {
	NO_COND 	= 0,
	EQ			= 1,
	LT			= 2,
	LE			= 3,
	GE			= 4,
	GT			= 5,
	ANY_COND	= 8,
	NE			= 9,
	ULT			= 10,
	ULE			= 11,
	UGE			= 12,
	UGT			= 13,
	MAX_COND	= 14
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

// useful types
typedef int	reg_t;
typedef t::uint32 uint_t;
typedef t::int32 int_t;
enum {
	intmin 	= 0x80000000,
	intmax 	= 0x7fffffff,
	uintmin	= 0,
	uintmax = 0xffffffff
};

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

	inline reg_t d(void) const { return _d; }
	inline reg_t a(void) const { return args.regs.a; }
	inline reg_t b(void) const { return args.regs.b; }

	// seti/setp instruction
	inline uint_t cst(void) const { return args.cst; }

	// load/store instruction
	inline reg_t reg(void) const { return d(); }
	inline reg_t addr(void) const { return a(); }
	inline type_t type(void) const { return type_t(b() & 0xFF); }
	inline int memIndex(void) const { return (b() & 0xFF00) >> 8; }

	// "if" instruction
	inline cond_t cond(void) const { return cond_t(d()); }
	inline reg_t sr(void) const { return a(); }
	inline uint_t jump(void) const { return b(); }

	void print(elm::io::Output& out, const hard::Platform *pf = 0) const;
} inst;
inline elm::io::Output& operator<<(elm::io::Output& out, inst i) { i.print(out); return out; }

inline inst nop		(void) 							{ return inst(NOP); }
inline inst branch	(uint_t to) 					{ return inst(BRANCH, to); }
inline inst trap	(void) 							{ return inst(TRAP); }
inline inst cont	(void) 							{ return inst(CONT); }
inline inst load	(reg_t d, reg_t a, int t, int id = 1) 		{ return inst(LOAD, d, a, t | (id << 8)); }
inline inst load	(reg_t d, reg_t a, type_t t, int id = 1)	{ return inst(LOAD, d, a, t | (id << 8)); }
inline inst store	(reg_t d, reg_t a, int t, int id = 1) 		{ return inst(STORE, d, a, t | (id << 8)); }
inline inst store	(reg_t d, reg_t a, type_t t, int id = 1)	{ return inst(STORE, d, a, t | (id << 8)); }
inline inst scratch	(reg_t d) 						{ return inst(SCRATCH, d); }
inline inst set		(reg_t d, reg_t a) 				{ return inst(SET, d, a); }
inline inst seti	(reg_t d, uint_t cst) 			{ inst i(SETI, d); i.args.cst = cst; return i; }
inline inst setp	(reg_t d, uint_t cst) 			{ inst i(SETP, d); i.args.cst = cst; return i; }
inline inst cmp		(reg_t d, reg_t a, reg_t b)		{ return inst(CMP, d, a, b); }
inline inst cmpu	(reg_t d, reg_t a, reg_t b) 	{ return inst(CMPU, d, a, b); }
inline inst add		(reg_t d, reg_t a, reg_t b)		{ return inst(ADD, d, a, b); }
inline inst sub		(reg_t d, reg_t a, reg_t b)		{ return inst(SUB, d, a, b); }
inline inst shl		(reg_t d, reg_t a, reg_t b) 	{ return inst(SHL, d, a, b); }
inline inst shr		(reg_t d, reg_t a, reg_t b)		{ return inst(SHR, d, a, b); }
inline inst asr		(reg_t d, reg_t a, reg_t b)		{ return inst(ASR, d, a, b); }
inline inst neg		(reg_t d, reg_t a)				{ return inst(NEG, d, a); }
inline inst _not	(reg_t d, reg_t a)				{ return inst(NOT, d, a); }
inline inst _and	(reg_t d, reg_t a, reg_t b) 	{ return inst(AND, d, a, b); }
inline inst _or		(reg_t d, reg_t a, reg_t b) 	{ return inst(OR, d, a, b); }
inline inst mul		(reg_t d, reg_t a, reg_t b) 	{ return inst(MUL, d, a, b); }
inline inst mulu	(reg_t d, reg_t a, reg_t b) 	{ return inst(MULU, d, a, b); }
inline inst div		(reg_t d, reg_t a, reg_t b) 	{ return inst(DIV, d, a, b); }
inline inst divu	(reg_t d, reg_t a, reg_t b) 	{ return inst(DIVU, d, a, b); }
inline inst mod		(reg_t d, reg_t a, reg_t b) 	{ return inst(MOD, d, a, b); }
inline inst modu	(reg_t d, reg_t a, reg_t b) 	{ return inst(MODU, d, a, b); }
inline inst _xor	(reg_t d, reg_t a, reg_t b) 	{ return inst(XOR, d, a, b); }
inline inst spec	(reg_t d, uint_t cst) 			{ inst i(SPEC, d); i.args.cst = cst; return i; }
inline inst mulh	(reg_t d, reg_t a, reg_t b) 	{ return inst(MULH, d, a, b); }
inline inst assume	(cond_t c, reg_t s)				{ ASSERT(c >= 0 && c < MAX_COND); return inst(ASSUME, c, s); }
inline inst fork	(uint_t j)						{ return inst(FORK, 0, 0, j); }


// deprecated
inline inst _if		(cond_t c, reg_t s, uint_t j) 	{ ASSERT(c >= 0 && c < MAX_COND); return inst(IF, c, s, j); }
inline inst _if		(uint_t c, reg_t s, uint_t j) 	{ ASSERT(c >= 0 && c < MAX_COND); return inst(IF, c, s, j); }


// Block class
class Block: public Vector<inst> {
	typedef Vector<inst> S;
public:
	class InstIter: public S::Iter {
	public:
		inline InstIter(const Block& block): S::Iter(block) { }
		inline InstIter(const InstIter& iter): S::Iter(iter) { }
		inline opcode op(void) const { return opcode(item().op); }
		inline reg_t d(void) const { return item().d(); }
		inline reg_t a(void) const { return item().a(); }
		inline reg_t b(void) const { return item().b(); }
		inline uint_t cst(void) const { return item().cst(); }
	};
	void print(elm::io::Output& out, const hard::Platform *pf = 0) const;
};
inline elm::io::Output& operator<<(elm::io::Output& out, const Block& b) { b.print(out); return out; }

// Printer class
class Printer {
public:
	inline Printer(const hard::Platform *platform = 0): pf(platform) { }

	void print(elm::io::Output& out, const Block& block, const hard::Platform *pf = 0) const;
	void print(elm::io::Output& out, const inst& inst, const hard::Platform *pf = 0) const;
private:
	const hard::Platform *pf;
};

// useful functions
cond_t invert(cond_t cond);
bool isSigned(cond_t cond);
bool isUnsigned(cond_t cond);
int size(type_t type);

io::Output& operator<<(io::Output& out, type_t type);
io::Output& operator<<(io::Output& out, cond_t cond);

} }	// otawa::sem

#endif /* OTAWA_SEM_INST_H_ */
