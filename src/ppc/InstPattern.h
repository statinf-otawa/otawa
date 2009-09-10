/*
 *	$Id$
 *	InstPattern class
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2007, IRIT UPS.
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
#ifndef OTAWA_LOADER_INSTPATTERN_H_
#define OTAWA_LOADER_INSTPATTERN_H_

#define ISS_DISASM
#include <emul.h>
#include <iss_include.h>

#ifdef OTAWA_PATTERN_VERBOSE
#	define OTAWA_PATTERN_DISPLAY(txt) cerr << txt
#else
#	define OTAWA_PATTERN_DISPLAY(txt)
#endif

namespace otawa { namespace loader {

#include <otawa/prog/Inst.h>

// Ignore class
class Ignore {
public:
	static inline void reset(void) { }
	static inline bool match(instruction_t *inst, int& n) { n++; return true; }
};

// ConstInt32 class
template <int cst>
class ConstInt32 {
public:
	static inline void reset(void) { }
	static inline bool match(instruction_t *inst, int& n)
		{ return inst->instrinput[n++].val.int32 == cst; }
};


// ConstUInt8 class
template <int cst>
class ConstUInt8 {
public:
	static inline void reset(void) { }
	static inline bool match(instruction_t *inst, int& n)
		{ return inst->instrinput[n++].val.uint8 == cst; }
};


// ConstUInt16 class
template <int cst>
class ConstUInt16 {
public:
	static inline void reset(void) { }
	static inline bool match(instruction_t *inst, int& n)
		{ return inst->instrinput[n++].val.uint16 == cst; }
};


// UInt16 class
template <int _>
class UInt16 {
public:
	static inline void reset(void) { }
	static inline bool match(instruction_t *inst, int& n)
		{ val = inst->instrinput[n++].val.uint16; return true; }
	static inline unsigned short value(void) { return val; }
private:
	static unsigned short val;
};
template <int _>
unsigned short UInt16<_>::val;


// Int16 class
template <int _>
class Int16 {
public:
	static inline void reset(void) { }
	static inline bool match(instruction_t *inst, int& n)
		{ val = inst->instrinput[n++].val.int16; return true; }
	static inline signed short value(void) { return val; }
private:
	static signed short val;
};
template <int _>
signed short Int16<_>::val;


// Reg class
template <int _>
class Reg {
public:
	static inline void reset(void) { set = false; }
	
	static inline bool match(instruction_t *inst, int& n) {
		OTAWA_PATTERN_DISPLAY("\t\t[" << n << "] R" << _ << " = ");
		int opnum = inst->instrinput[n++].val.uint8;
		if(set) {
			if(opnum != num)
				OTAWA_PATTERN_DISPLAY(num << ", failed /= " << opnum << '\n');
			else
				OTAWA_PATTERN_DISPLAY(opnum << ", sucess\n");
			return opnum == num;
		}
		OTAWA_PATTERN_DISPLAY(opnum << ", set\n");
		set = true;
		num = opnum;
		return true;		
	}

	static inline int number(void) { return num; }

private:
	static bool set;
	static int num;
};
template <int _> bool Reg<_>::set;
template <int _> int Reg<_>::num;


// ConstReg class
template <int v>
class ConstReg {
public:
	static inline void reset(void) { }
	static inline bool match(instruction_t *inst, int& n)
		{ return inst->instrinput[n++].val.int8 == v; }
};

// OpCons class
template <class T1, class T2>
class OpCons {
public:
	static inline void reset(void) { T1::reset(); T2::reset(); }
	static bool match(instruction_t *inst, int& n)
		{ return T1::match(inst, n) && T2::match(inst, n); }
};

// Ops class
template <class T1, class T2 = Ignore, class T3 = Ignore, class T4 = Ignore, class T5 = Ignore>
class Ops: public OpCons<T1, OpCons<T2, OpCons <T3, OpCons<T4, T5> > > > { };

// Inst class
template <int id, class O1 = Ignore, class O2 = Ignore, class O3 = Ignore, class O4 = Ignore, class O5 = Ignore>
class I {
public:
	typedef Ops<O1, O2, O3, O4, O5> ops;
	static inline void reset(void) { ops::reset(); }
	static inline bool match(state_t *state, const Address& addr) {
		code_t buffer[20];
		iss_fetch(addr, buffer);
		instruction_t *inst = iss_decode(state, addr, buffer);
		char dis[256];
		iss_disasm(dis, inst);
		OTAWA_PATTERN_DISPLAY("\t" << addr << " " << dis << '\n');
		if(inst->ident != id) {
			OTAWA_PATTERN_DISPLAY("\t fail on ID = " << inst->ident << "!=" << id << '\n');
			return false;
		}
		int n = 0;
		return ops::match(inst, n);
	}
	static inline bool matchBack(state_t *state, Address& addr) {
		addr -= 4;	// !!WARNING!! PPC dependency
		return match(state, addr);
	}
};

// InstCons class
template <class I1, class I2>
class InstCons {
public:
	static inline void reset(void) { I1::reset(); I2::reset(); }
	static inline bool matchBack(state_t *state, Address& addr) {
		return I2::matchBack(state, addr) && I1::matchBack(state, addr);
	}
};


// NoInst class
class NoInst {
public:
	static inline void reset(void) { }
	static inline bool matchBack(state_t *state, Address& addr) { return true; }
};


// Seq class
template <class I1, class I2 = NoInst, class I3 = NoInst, class I4 = NoInst, class I5 = NoInst>
class Seq: public InstCons<I1, InstCons<I2, InstCons<I3, InstCons<I4, I5> > > > { };

} } // otawa::loader

#endif /* OTAWA_LOADER_INSTPATTERN_H_ */
