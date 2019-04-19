/*
 *	StackAnalysis process implementation
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

//#define DEBUG
#ifdef DEBUG
#	define HAI_DEBUG
#endif

#include <otawa/dfa/State.h>
#include <otawa/hard/Platform.h>
#include <otawa/hard/Register.h>
#include <otawa/proc/BBProcessor.h>
#include <otawa/proc/CFGProcessor.h>
#include <otawa/prog/File.h>
#include <otawa/prog/sem.h>
#include <otawa/stack/AccessedAddress.h>
#include <otawa/stack/features.h>
#include <otawa/stack/StackAnalysis.h>
#include <otawa/dfa/hai/HalfAbsInt.h>
#include <otawa/dfa/hai/DefaultFixPoint.h>
#include <otawa/dfa/hai/DefaultListener.h>
#include <otawa/cfg/features.h>

using namespace elm;
using namespace otawa;
using namespace otawa::dfa::hai;

#define TRACEU(t)	//t
#define TRACEI(t)	//t
#define TRACES(t)	//t
#define TRACED(t)	//t

namespace otawa {

/**
 * This identifier is a configuration for the @ref StackAnalysis processor.
 * It allows to provide initial values for the registers involved in the analysis.
 * The argument is a pair of register and its initial value as an address.
 * A null address express the fact that the register is initialized with the default
 * stack pointer address.
 * @deprecated
 */
Identifier<StackAnalysis::init_t> StackAnalysis::INITIAL(
		"otawa::StackAnalysis::INITIAL",
		pair((const hard::Register *)0, Address::null));


namespace stack {

/**
 * Stack analysis state at entry of BBs.
 *
 * @par Hooks
 * @li @ref BasicBlock
 *
 * @par Feature
 * @li @ref otawa::STACK_ANALYSIS_FEATURE
 */
Identifier<State *> STATE("otawa::stack::STATE", 0);

/**
 * @class Value
 * Values in the @ref StackAnalysis.
 *
 * A value may be:
 * @li none (@ref NONE) -- undefined value,
 * @li absolute (@ref CST) -- constant value,
 * @li stack relative (@ref SP) -- offset relative to the initial address of the stack,
 * @li all (@ref ALL) -- any value.
 *
 * @ingroup ref
 */


/**
 * Construct for a value.
 * @deprecated	Use Value::reg(), Value::cst(), Value::addr(), Value::bot or Value::top instead.
 */
Value::Value(kind_t k, int_t v) {
	switch(k) {
	case ALL:	*this = top; break;
	case NONE:	*this = bot; break;
	case REG:	*this = reg(v); break;
	case SP:	*this = sp(v); break;
	case CST:	*this = cst(v); break;
	}
}


/**
 * Get the kind of the value.
 * @return	Kind of the value.
 * @deprecated	Usage must be replaced by "isXXX"() methods.
 */
kind_t Value::kind(void) const {
	if(isTop())
		return ALL;
	else if(isBot())
		return NONE;
	else if(isSP())
		return SP;
	else if(isReg())
		return REG;
	else
		return CST;
}


/**
 * Bottom value (uninitialized value).
 */
Value Value::bot(spec_page, bot_id);


/**
 * Top value (any value).
 */
Value Value::top(spec_page, top_id);


/**
 * Perform addition of values.
 */
void Value::add(const Value& val) {
	if((isConst() && val.isConst())
	|| (isSP()    && val.isConst()))
		_val += val._val;
	else if(isConst() && val.isSP()) {
		_page = sp_page;
		_val += val._val;
	}
	else if(isBot() && val.isBot())
		*this = bot;
	else
		*this = top;
}


/**
 * Perform subtraction of values.
 */
void Value::sub(const Value& val) {
	if((isConst() && val.isConst())
	|| (isSP()    && val.isConst()))
		_val -= val._val;
	else if(isBot() && val.isBot())
		*this = bot;
	else
		*this = top;
}


/**
 * Perform OR operator on values (modifying current one).
 * @param val	The value to OR with the current one.
 */
void Value::_or(const Value& val) {
	if(isConst() && val.isConst())
		_val |= val._val;
	else if(isBot() && val.isBot())
		*this = bot;
	else
		*this = top;
}


/**
 * Perform AND operator on values (modifying current one).
 * @param val	The value to OR with the current one.
 */
void Value::_and(const Value& val) {
	if(isConst() && val.isConst())
		_val &= val._val;
	else if(isBot() && val.isBot())
		*this = bot;
	else
		*this = top;
}


/**
 * Perform XOR operator on values (modifying current one).
 * @param val	The value to OR with the current one.
 */
void Value::_xor(const Value& val) {
	if(isConst() && val.isConst())
		_val ^= val._val;
	else if(isBot() && val.isBot())
		*this = bot;
	else
		*this = top;
}


/**
 * Perform MUL operator on values (modifying current one).
 * @param val	The value to OR with the current one.
 */
void Value::mul(const Value& val) {
	if(isConst() && val.isConst())
		_val *= val._val;
	else if(isBot() && val.isBot())
		*this = bot;
	else
		*this = top;
}


/**
 * Perform division on values (modifying current one).
 * @param val	The value to OR with the current one.
 */
void Value::div(const Value& val) {
	if(isConst() && val.isConst())
		_val = static_cast<signed_t>(_val) / static_cast<signed_t>(val._val);
	else if(isBot() && val.isBot())
		*this = bot;
	else
		*this = top;
}


/**
 * Perform division on values (modifying current one).
 * @param val	The value to OR with the current one.
 */
void Value::divu(const Value& val) {
	if(isConst() && val.isConst())
		_val /= val._val;
	else if(isBot() && val.isBot())
		*this = bot;
	else
		*this = top;
}


/**
 * Perform modulo on values (modifying current one).
 * @param val	The value to OR with the current one.
 */
void Value::mod(const Value& val) {
	if(isConst() && val.isConst())
		_val = static_cast<signed_t>(_val) % static_cast<signed_t>(val._val);
	else if(isBot() && val.isBot())
		*this = bot;
	else
		*this = top;
}


/**
 * Perform modulo on values (modifying current one).
 * @param val	The value to OR with the current one.
 */
void Value::modu(const Value& val) {
	if(isConst() && val.isConst())
		_val %= val._val;
	else if(isBot() && val.isBot())
		*this = bot;
	else
		*this = top;
}


/**
 * Compute upper-word multiplication on values (modifying current one).
 * @param val	The value to OR with the current one.
 */
void Value::mulh(const Value& val) {
	if(isConst() && val.isConst())
		_val = (static_cast<upper_t>(_val) % static_cast<upper_t>(val._val)) >> (sizeof(int_t) * 8);
	else if(isBot() && val.isBot())
		*this = bot;
	else
		*this = top;
}


/**
 * Print the value.
 */
void Value::print(io::Output& out) const {
	if(isBot())
		out << '_';
	else if(isTop())
		out << 'T';
	else if(isReg())
		out << 'r' << _val;
	else if(isSP()) {
		t::int32 v = _val;
		if(v >= 0)
			out << "sp+" << io::hex(v);
		else
			out << "sp-" << io::hex(-v);
	}
	else
		out << "k(" << io::hex(_val) << ')';
}


/**
 * Left-shit on a value.
 */
void Value::shl(const Value& val) {
	if(isConst() && val.isConst())
		_val <<= val._val;
	else if(isBot() && val.isBot())
		*this = bot;
	else
		*this = top;
}


/**
 * Right-shift on a value.
 */
void Value::shr(const Value& val) {
	if(isConst() && val.isConst())
		_val >>= val._val;
	else if(isBot() && val.isBot())
		*this = bot;
	else
		*this = top;
}


/**
 * Arithmetic right-shift on a value.
 */
void Value::asr(const Value& val) {
	if(isConst() && val.isConst())
		_val = static_cast<signed_t>(_val) >> val._val;
	else if(isBot() && val.isBot())
		*this = bot;
	else
		*this = top;
}


/**
 * Invert sign of value.
 */
void Value::neg(void) {
	if(isConst())
		_val = -_val;
	else if(isBot())
		*this = bot;
	else
		*this = top;
}


/**
 * Invert bits of the value.
 */
void Value::_not(void) {
	if(isConst())
		_val = ~_val;
	else if(isBot())
		*this = bot;
	else
		*this = top;
}


/**
 * Join of values.
 */
void Value::join(const Value& val) {
	if(*this != val) {
		if(isBot())
			*this = val;
		else if(!val.isBot())
			*this = top;
	}
}


const Value& Value::none	= bot,	/** undefined value */
			Value::all 		= top;	/** any value */


class State {
public:

	class Node {
	public:
		friend class State;
		inline Node(void): next(0), addr(Value::none) { }
		inline Node(const Value& address, const Value& value)
			: next(0), addr(address), val(value) { }
		inline Node(const Node *node)
			: next(0), addr(node->addr), val(node->val) { }
	private:
		Node *next;
		Value addr;
		Value val;
	};

	State(const Value& def = Value::all): first(Value::none, def)
		{ TRACED(cerr << "State(" << def << ")\n"); }
	State(const State& state): first(Value::none, Value::all)
		{ TRACED(cerr << "State("; state.print(cerr); cerr << ")\n"); copy(state); }
	~State(void) { clear(); }

	inline bool isBot(void) const { return first.addr == Value::none; }
	inline State& operator=(const State& state) { copy(state); return *this; }

	void copy(const State& state) {
		TRACED(cerr << "copy("; print(cerr); cerr << ", "; state.print(cerr); cerr << ") = ");
		clear();
		first = state.first;
		for(Node *prev = &first, *cur = state.first.next; cur; cur = cur->next) {
			prev->next = new Node(cur);
			prev = prev->next;
		}
		TRACED(print(cerr); cerr << io::endl);
	}

	void clear(void) {
		for(Node *cur = first.next, *next;
		cur;
		cur = next) {
			next = cur->next;
			delete cur;
		}
		first.next = 0;
	}

	void set(const Value& addr, const Value& val) {
		TRACED(cerr << "set("; print(cerr); cerr << ", " << addr << ", " << val << ") = ");
		Node *prev, *cur, *next;
		if(first.val == Value::none) {
			TRACED(print(cerr); cerr << io::endl);
			return;
		}

		// consum all memory references
		if(addr.kind() == ALL) {
			for(prev = &first, cur = first.next; cur && cur->addr.kind() <= SP; prev = cur, cur = cur->next) ;
			while(cur) {
				next = cur->next;
				delete cur;
				cur = next;
			}
			prev->next = 0;
		}

		// find a value
		else {
			for(prev = &first, cur = first.next; cur && cur->addr < addr; prev = cur, cur = cur->next);
			if(cur && cur->addr == addr) {
				if(val.kind() != ALL)
					cur->val = val;
				else {
					prev->next = cur->next;
					delete cur;
				}
			}
			else if(val.kind() != ALL) {
				next = new Node(addr, val);
				prev->next = next;
				prev->next->next = cur;
			}
		}

		TRACED(print(cerr); cerr << io::endl);
	}

	bool equals(const State& state) const {
		if(first.val.kind() != state.first.val.kind())
			return false;
		Node *cur = first.next, *cur2 = state.first.next;
		while(cur && cur2) {
			if(cur->addr != cur2->addr)
				return false;
			if(cur->val != cur->val)
				return false;
			cur = cur->next;
			cur2 = cur2->next;
		}
		return cur == cur2;
	}

	void join(const State& state) {
		TRACED(cerr << "join(\n\t"; print(cerr); cerr << ",\n\t";  state.print(cerr); cerr << "\n\t) = ");

		// test none states
		if(state.first.val == Value::none)
			return;
		if(first.val == Value::none) {
			copy(state);
			TRACED(print(cerr); cerr << io::endl;);
			return;
		}

		Node *prev = &first, *cur = first.next, *cur2 = state.first.next, *next;
		while(cur && cur2) {

			// addr1 < addr2 -> remove cur1
			if(cur->addr < cur2->addr) {
				prev->next = cur->next;
				delete cur;
				cur = prev->next;
			}

			// equality ? remove if join result in all
			else if(cur->addr == cur2->addr) {
				cur->val.join(cur2->val);
				if(cur->val.kind() == ALL) {
					prev->next = cur->next;
					delete cur;
					cur = prev->next;
				}
				else {
					prev = cur;
					cur = cur->next;
					cur2 = cur2->next;
				}
			}

			// addr1 > addr2 => remove cur2
			else
				cur2 = cur2->next;
		}

		// remove tail
		prev->next = 0;
		while(cur) {
			next = cur->next;
			delete cur;
			cur = next;
		}
		TRACED(print(cerr); cerr << io::endl;);
	}

	void print(io::Output& out) const {
		if(first.val == Value::none)
			out << '_';
		else {
			bool f =  true;
			out << "{ ";
			for(Node *cur = first.next; cur; cur = cur->next) {
				if(f)
					f = false;
				else
					out << ", ";
				out << cur->addr << " = " << cur->val;
			}
			out << " }";
		}
	}

	Value fromImage(const Address& addr, Process *proc, int size) const {
		switch(size) {
		case 1: { t::uint8 v; proc->get(addr, v); return Value(CST, v); }
		case 2: { t::uint16 v; proc->get(addr, v); return Value(CST, v); }
		case 4: { t::uint32 v; proc->get(addr, v); return Value(CST, v); }
		}
		return first.val;
	}

	Value get(const Value& addr, Process *proc, int size) const {
		Node * cur;
		for(cur = first.next; cur && cur->addr < addr; cur = cur->next) ;
		if(cur && cur->addr == addr)
			return cur->val;
		if(addr.kind() == CST)
			for(Process::FileIter file(proc); file(); file++)
				for(File::SegIter seg(*file); seg(); seg++)
					if(seg->contains(addr.value()))
						return fromImage(addr.value(), proc, size);
		return first.val;
	}

	static const State EMPTY, FULL;

private:
	Node first;
};
const State State::EMPTY(Value::none), State::FULL(Value::all);
io::Output& operator<<(io::Output& out, const State& state) { state.print(out); return out; }

} // stack


// Problem definition
class StackProblem {
public:
	typedef stack::State Domain;

	typedef StackProblem Problem;
	Problem& getProb(void) { return *this; }

	StackProblem(WorkSpace *ws): proc(ws->process()) {

		// execute process initialization
		sem::PathIter i;
		for(i.start(proc); i(); i++)
			update(_init, *i);

		// install the initial state
		dfa::State *s = dfa::INITIAL_STATE(ws);
		if(s) {
			for(dfa::State::RegIter r(s); r(); r++)
				if((*r).snd.kind() == dfa::Value::CONST)
					_init.set(stack::Value::reg((*r).fst->platformNumber()), stack::Value::cst((*r).snd.value()));
			for(dfa::State::MemIter m(s); m(); m++)
				if((*m).value().kind() == dfa::Value::CONST)
					_init.set(stack::Value::addr((*m).address()), stack::Value::cst((*m).value().value()));
		}

	}

	void initialize(const hard::Register *reg, const Address& address) {
		stack::Value v;
		if(address.isNull())
			v = stack::Value(stack::SP, 0);
		else
			v = stack::Value(stack::CST, address.offset());
		set(_init, reg->platformNumber(), v);
	}

	inline const Domain& top(void) const { return stack::State::FULL; }
	inline const Domain& bottom(void) const { return stack::State::EMPTY; }
	inline const Domain& entry(void) const { TRACED(cerr << "entry() = " << _init << io::endl); return _init; }
	inline void lub(Domain &a, const Domain &b) const { a.join(b); }
	inline void assign(Domain &a, const Domain &b) const { a = b; }
	inline bool equals(const Domain &a, const Domain &b) const { return a.equals(b); }
	inline void enterContext(Domain &dom, Block *header, dfa::hai::hai_context_t ctx) { }
	inline void leaveContext(Domain &dom, Block *header, dfa::hai::hai_context_t ctx) { }

	stack::Value get(const stack::State& state, int i) {
		if(i <  0)
			return tmp[-i];
		else {
			stack::Value addr(stack::REG, i);
			return state.get(addr, proc, 0);
		}
	}

	const void set(stack::State& state, int i, const stack::Value& v) {
		if(i < 0)
			tmp[-i] = v;
		else {
			stack::Value addr(stack::REG, i);
			return state.set(addr, v);
		}
	}

	void update(stack::State& s, sem::inst i) {
		switch(i.op) {
		case sem::BRANCH:
		case sem::TRAP:
		case sem::CONT:
		case sem::IF:
			break;
		case sem::NOP: break;
		case sem::LOAD: {
				stack::Value addr = get(s, i.a());
				set(s, i.d(), s.get(addr, proc, i.b()));
			} break;
		case sem::STORE: {
				stack::Value addr = get(s, i.a());
				s.set(addr, get(s, i.d()));
			} break;
		case sem::CMP:
		case sem::CMPU:
		case sem::SCRATCH:
			set(s, i.d(), stack::Value::all);
			break;
		case sem::SET: {
				stack::Value v = get(s, i.a());
				set(s, i.d(), v);
			} break;
		case sem::SETI: {
				stack::Value v(stack::CST, i.cst());
				set(s, i.d(), v);
			} break;
		case sem::SETP: {
				stack::Value v = get(s, i.d());
				if(v.isConst())
					v = stack::Value::addr(Address(i.cst(), v.value()));
				else
					v = stack::Value::top;
				set(s, i.d(), v);
			} break;
		case sem::ADD: {
				stack::Value v = get(s, i.a());
				v.add(get(s, i.b()));
				set(s, i.d(), v);
			} break;
		case sem::SUB: {
				stack::Value v = get(s, i.a());
				v.sub(get(s, i.b()));
				set(s, i.d(), v);
			} break;
		case sem::SHL: {
				stack::Value v = get(s, i.a());
				v.shl(get(s, i.b()));
				set(s, i.d(), v);
			} break;
		case sem::SHR: {
				stack::Value v = get(s, i.a());
				v.shr(get(s, i.b()));
				set(s, i.d(), v);
			} break;
		case sem::ASR: {
				stack::Value v = get(s, i.a());
				v.asr(get(s, i.b()));
				set(s, i.d(), v);
			} break;
		case sem::NEG: {
				stack::Value v = get(s, i.a());
				v.neg();
				set(s, i.d(), v);
			} break;
		case sem::NOT: {
				stack::Value v = get(s, i.a());
				v._not();
				set(s, i.d(), v);
			} break;
		case sem::OR: {
				stack::Value v = get(s, i.a());
				v._or(get(s, i.b()));
				set(s, i.d(), v);
			} break;
		case sem::AND: {
				stack::Value v = get(s, i.a());
				v._and(get(s, i.b()));
				set(s, i.d(), v);
			} break;
		case sem::XOR: {
				stack::Value v = get(s, i.a());
				v._xor(get(s, i.b()));
				set(s, i.d(), v);
			} break;
		case sem::MUL: {
				stack::Value v = get(s, i.a());
				v.mul(get(s, i.b()));
				set(s, i.d(), v);
			} break;
		case sem::MULU: {
				stack::Value v = get(s, i.a());
				v.mul(get(s, i.b()));
				set(s, i.d(), v);
			} break;
		case sem::DIV: {
				stack::Value v = get(s, i.a());
				v.div(get(s, i.b()));
				set(s, i.d(), v);
			} break;
		case sem::DIVU: {
				stack::Value v = get(s, i.a());
				v.divu(get(s, i.b()));
				set(s, i.d(), v);
			} break;
		case sem::MOD: {
				stack::Value v = get(s, i.a());
				v.mod(get(s, i.b()));
				set(s, i.d(), v);
			} break;
		case sem::MODU: {
				stack::Value v = get(s, i.a());
				v.modu(get(s, i.b()));
				set(s, i.d(), v);
			} break;
		case sem::MULH: {
				stack::Value v = get(s, i.a());
				v.mulh(get(s, i.b()));
				set(s, i.d(), v);
			} break;
		}
	}

	void update(Domain& is, Inst *i) {
		int pc;
		TRACEU(cerr << '\t' << i->address() << ": "; i->dump(cerr); cerr << io::endl);

		// get instructions
		b.clear();
		i->semInsts(b);
		pc = 0;
		Domain *state = &is;

		// perform interpretation
		while(true) {

			// interpret current
			while(pc < b.length()) {
				sem::inst& i = b[pc];
				switch(i.op) {
				case sem::CONT:
					pc = b.length();
					TRACES(cerr << "\t\tcut\n");
					break;
				case sem::IF:
					todo.push(pair(pc + i.b() + 1, new Domain(*state)));
					break;
				default:
					update(*state, i);
					break;
				}
				pc++;
			}

			// pop next
			if(state != &is) {
				is.join(*state);
				delete state;
			}
			if(!todo)
				break;
			else {
				Pair<int, Domain *> p = todo.pop();
				pc = p.fst;
				state = p.snd;
			}
		}
		TRACEI(cerr << "\t -> " << is << io::endl);
	}

	void update(Domain& out, const Domain& in, Block* bb) {
		if(bb->isBasic()) {
			out.copy(in);
			TRACEU(cerr << "update(BB" << bb->number() << ", " << in << ")\n");
			for(BasicBlock::InstIter inst = bb->toBasic()->insts(); inst(); inst++)
				update(out, *inst);
			TRACEU(cerr << "\tout = " << out << io::endl);
		}
		else
			out.copy(in);
	}

	inline stack::Value load(Domain& s, stack::Value addr, int size) const { return s.get(addr, proc, size); }

private:
	stack::Value tmp[16];
	stack::State _init;
	sem::Block b;
	Vector<Pair<int, Domain *> > todo;
	Process *proc;
};


/**
 * @class StackAnalysis
 *
 * This analyzer computes accessed addresses
 * @li non-array stack accesses,
 * @li non-array absolute address accesses.
 *
 * Basically, there is two use of this analyzer:
 * @li analysis of data cache
 * @li stack size analysis
 *
 * @par Configuration
 *
 * @par Provided Features
 * @li @ref otawa::STACK_ANALYSIS_FEATURE
 * @li @ref otawa::ADDRESS_ANALYSIS_FEATURE
 *
 * @par Required Features
 * @li @ref otawa::VIRTUALIZED_CFG_FEATURE
 * @li @ref otawa::LOOP_INFO_FEATURE
 *
 * @ingroup stack
 */
StackAnalysis::StackAnalysis(p::declare& r): Processor(r) {
}


p::declare StackAnalysis::reg = p::init("otawa::StackAnalysis", Version(1, 0, 0))
	.maker<StackAnalysis>()
	.require(LOOP_INFO_FEATURE)
	.require(dfa::INITIAL_STATE_FEATURE)
	.provide(STACK_ANALYSIS_FEATURE);


/**
 */
void StackAnalysis::processWorkSpace(WorkSpace *ws) {
	typedef DefaultListener<StackProblem> StackListener;
	typedef DefaultFixPoint<StackListener> StackFP;
	typedef HalfAbsInt<StackFP> StackAI;

	// get the entry
	const CFGCollection *coll = INVOLVED_CFGS(ws);
	ASSERT(coll);
	CFG *cfg = coll->get(0);

	// perform the analysis
	if(logFor(LOG_CFG))
		log << "FUNCTION " << cfg->label() << io::endl;
	StackProblem prob(ws);
	const hard::Register *sp = ws->process()->platform()->getSP();
	if(sp)
		prob.initialize(sp, Address::null);
	else
		log << "WARNING: no sp register on this platform?\n";
	for(int i = 0; i < inits.count(); i++)
		prob.initialize(inits[i].fst, inits[i].snd);
	StackListener list(ws, prob);
	StackFP fp(list);
	StackAI sai(fp, *ws);
	sai.solve(cfg);

	// record the results
	if(logFor(LOG_BLOCK))
		log << "\tSTORING RESULTS\n";
	for(CFGCollection::Iter cfg(coll); cfg(); cfg++)
		for(CFG::BlockIter bb = cfg->blocks(); bb(); bb++) {
			if(logFor(LOG_BLOCK))
				log << "\t\t" << *bb << "[" << cfg->index() << "][" << cfg->index() << "]: " << *list.results[cfg->index()][bb->index()] << io::endl;
			stack::STATE(*bb) = new stack::State(*list.results[cfg->index()][bb->index()]);
	}
}


// state cleanup
class StackStateCleaner: public otawa::BBCleaner {
public:
	StackStateCleaner(WorkSpace *ws): BBCleaner(ws) { }
protected:
	virtual void clean(WorkSpace *ws, CFG *cfg, Block *bb) {
		ASSERT(stack::STATE(bb));
		delete *stack::STATE(bb);
	}
};


/**
 */
void StackAnalysis::cleanup(WorkSpace *ws) {
	track(STACK_ANALYSIS_FEATURE, stack::MANAGER(ws) = new stack::Manager(ws));
	addCleaner(STACK_ANALYSIS_FEATURE, new StackStateCleaner(ws));
}


/**
 */
void StackAnalysis::configure(const PropList &props) {
	Processor::configure(props);
	for(Identifier<init_t>::Getter init(props, INITIAL); init(); init++)
		inits.add(*init);
}


/**
 * This features ensure that the stack analysis has been identified.
 * @par Default Processor
 * @li @ref otawa::StackAnalysis
 * @ingroup stack
 */
p::feature STACK_ANALYSIS_FEATURE("otawa::STACK_ANALYSIS_FEATURE", new Maker<StackAnalysis>());


/**
 * @defgroup stack Stack Analysis
 * This analysis is a very simple analysis using semantic instruction to determine constant values (and adresses)
 * and addresses relative to the initial stack address of the program. An interesting property is that
 * it can be called on each subprogram separately and provide local addresses relative to the initial
 * address of the stack without knowing it. This analysis may also be used to determine the stack memory size
 * used by subprograms (but only without recursive calls).
 */

namespace stack {

/**
 * @class Iter
 * Iterator on the result of stack analysis.
 */

/**
 */
Iter::Iter(WorkSpace *ws): otawa::sem::StateIter<Manager>(**MANAGER(ws)) {
}


/**
 * @class Manager
 * Manager for results of stack analysis. Implements concept StateManager.
 * @ingroup stack
 */

/**
 */
Manager::Manager(WorkSpace *ws) {
	p = new otawa::StackProblem(ws);
	_bot = new State(p->bottom());
}

/**
 */
Manager::~Manager(void) {
	delete _bot;
}

/**
 * Get register value.
 * @param i		Register index.
 * @return		Register value.
 */
Value Manager::getReg(t s, int i) {
	return p->get(*s, i);
}

/**
 * Get value from memory.
 * @param addr	Value address.
 * @param size	Value size.
 * @return		Memory value.
 */
Value Manager::getMem(t s, Value addr, int size) {
	return p->load(*s, addr, size);
}

/**
 */
Manager::t Manager::bot(void) {
	return _bot;
}

/**
 */
void Manager::free(t s) {
	if(s != _bot)
		delete s;
}

/**
 */
bool Manager::equals(t s1, t s2) {
	return s1 == s2 || p->equals(*s1, *s2);
}

/**
 */
Manager::t Manager::copy(t s) {
	return new State(*s);
}

/**
 */
Manager::t Manager::joinPath(t s1, t s2) {
	if(s1->isBot())
		return s2;
	else if(s2->isBot())
		return s1;
	else {
		s1->join(*s2);
		delete s2;
		return s1;
	}
}

/**
 */
Manager::t Manager::state(BasicBlock *bb) {
	return *STATE(bb);
}

/**
 */
Manager::t Manager::updatePath(t s, sem::inst i) {
	p->update(*s, i);
	return s;
}


/**
 * Provide accessed addresses from the stack analysis.
 * Basically, this means that it only provides address of type
 * @ref AccessedAddress::ANY or @ref AccessedAddress::SP.
 */
class AddressBuilder: public BBProcessor {
public:
	static p::declare reg;
	AddressBuilder(p::declare& r = reg): BBProcessor(r), do_stats(false) { }

	void configure(const PropList &props) {
		BBProcessor::configure(props);
		do_stats = STATS(props);
	}

protected:
	virtual void setup(WorkSpace *ws) {
		ss.all = 0;
		ss.abs = 0;
		ss.sprel = 0;
		ss.total = 0;
		iter = new Iter(ws);
	}

	virtual void cleanup(WorkSpace *ws) {
		if(do_stats)
			ADDRESS_STATS(stats) = new address_stat_t(ss);
		delete iter;
	}

	virtual void processBB(WorkSpace *ws, CFG *cfg, Block *bb) {
		if(!bb->isBasic())
			return;
		Iter& i = *iter;

		// collect addresses
		Vector<AccessedAddress *> addrs;
		for(i.start(bb->toBasic()); i(); i++)
			if((*i).op == sem::LOAD || (*i).op == sem::STORE) {
				bool is_store = (*i).op == sem::STORE;
				int r = (*i).addr();
				Value a = i.getReg(r);
				AccessedAddress *aa = nullptr;
				switch(a.kind()) {
				case NONE:
				case ALL:	aa = new AccessedAddress(i.instruction(), is_store, AccessedAddress::ANY); ss.all++; break;
				case SP:	aa = new SPAddress(i.instruction(), is_store, a.value()); ss.sprel++; break;
				case CST:	aa = new AbsAddress(i.instruction(), is_store, a.value()); ss.abs++; break;
				case REG:	ASSERT(false); break;
				}
				addrs.add(aa);
				ss.total++;
			}

		// improve the accessed addresses set
		AccessedAddresses *aa = ADDRESSES(bb);
		if(!aa) {
			aa = new AccessedAddresses();
			ADDRESSES(bb) = aa;
		}
		aa->set(addrs);
	}

private:
	address_stat_t ss;
	bool do_stats;
	Iter *iter;
};

p::declare AddressBuilder::reg = p::init("otawa::stack::AddressBuilder", Version(1, 0, 0))
	.base(BBProcessor::reg)
	.maker<AddressBuilder>()
	.require(STACK_ANALYSIS_FEATURE)
	.provide(ADDRESS_ANALYSIS_FEATURE)
	.provide(ADDRESS_FEATURE);


/**
 * Feature ensuring that addresses are built from stack analysis. This features must
 * always induce ADDRESS_ANALYSIS_FEATURE to be also provided.
 */
p::feature ADDRESS_FEATURE("otawa::stack::ADDRESS_FEATURE", new Maker<AddressBuilder>());


/**
 * Property providing analysis manager to use result of stack analysis.
 *
 * @par Features
 * @li @ref otawa:: STACK_ANAKYSIS_FEATURE
 *
 * @par Hooks
 * @li WorkSpace
 *
 * @ingroup stack
 */
Identifier<Manager *> MANAGER("otawa::stack::Manager", 0);

} }	// otawa::stack
