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

#include <otawa/prog/sem.h>
#include <otawa/proc/CFGProcessor.h>
#include <otawa/util/HalfAbsInt.h>
#include <otawa/util/DefaultFixPoint.h>
#include <otawa/util/DefaultListener.h>
#include <otawa/util/LoopInfoBuilder.h>
#include <otawa/proc/BBProcessor.h>
#include <otawa/cfg/Virtualizer.h>
#include <otawa/stack/StackAnalysis.h>
#include <otawa/stack/AccessedAddress.h>
#include <otawa/hard/Register.h>
#include <otawa/hard/Platform.h>
#include <otawa/dfa/State.h>

using namespace elm;
using namespace otawa;
using namespace otawa::util;

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
	inline Value& operator=(const Value& val)
		{ _kind = val._kind; _value = val._value; return *this; }

	inline bool operator==(const Value& val) const { return _kind == val._kind && _value == val._value; }
	inline bool operator!=(const Value& val) const { return ! operator==(val); }
	inline bool operator<(const Value& val) const
		{ return (_kind < val._kind) || (_kind == val._kind && _value < val._value); }

	inline kind_t kind(void) const { return _kind; }
	inline unsigned long value(void) const { return _value; }

	void add(const Value& val) {
		if((_kind == CST && val._kind == CST)
		|| (_kind == SP && val._kind == CST)
		|| (_kind == CST && val._kind == SP))
			_value += val._value;
		else if(_kind == NONE && val._kind == NONE)
			set(NONE, 0);
		else
			set(ALL, 0);
	}

	void sub(const Value& val) {
		if((_kind == CST && val._kind == CST)
		|| (_kind == SP && val._kind == CST))
			_value -= val._value;
		else if(_kind == NONE && val._kind == NONE)
			set(NONE, 0);
		else
			set(ALL, 0);
	}

	void print(io::Output& out) const {
		switch(_kind) {
		case NONE: out << '_'; break;
		case REG: out << 'r' << _value; break;
		case CST: out << "k(" << io::hex(_value) << ')'; break;
		case SP: {
				t::int32 v = _value;
				if(v >= 0)
					out << "sp+" << io::hex(v);
				else
					out << "sp-" << io::hex(-v);
			}
			break;
		case ALL: out << 'T'; break;
		}
	}

	void shl(const Value& val) {
		if(_kind == CST && val._kind == CST)
			_value <<= val._value;
		else if(_kind == NONE && val._kind == NONE)
			set(NONE, 0);
		else
			set(ALL, 0);
	}

	void shr(const Value& val) {
		if(_kind == CST && val._kind == CST)
			_value >>= val._value;
		else if(_kind == NONE && val._kind == NONE)
			set(NONE, 0);
		else
			set(ALL, 0);
	}

	void join(const Value& val) {
		if(!(_kind == val._kind && _value == val._value)) {
			if(_kind != ALL && val._kind != ALL
			&& (_kind == NONE || val._kind == NONE))
				set(NONE, 0);
			else
				set(ALL, 0);
		}
	}

	static const Value none, all;

private:
	inline void set(kind_t kind, unsigned long value) { _kind = kind; _value = value; }
	kind_t _kind;
	t::uint32 _value;
};
inline io::Output& operator<<(io::Output& out, const Value& v) { v.print(out); return out; }
const Value Value::none(NONE), Value::all(ALL);

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
		for(Node *cur = first.next, *next; cur; cur = next) {
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
				switch(cur->addr.kind()) {
				case NONE:
					out << "_";
					break;
				case REG:
					out << 'r' << cur->addr.value();
					break;
				case CST:
					out << Address(cur->addr.value());
					break;
				case SP: {
						t::int32 v = cur->addr.value();
						if(v >= 0)
							out << "[sp+" << io::hex(v) << ']';
						else
							out << "[sp-" << io::hex(-v) << ']';
					}
					break;
				case ALL:
					out << 'T';
					break;
				}
				out << " = " << cur->val;
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
			for(Process::FileIter file(proc); file; file++)
				for(File::SegIter seg(file); seg; seg++)
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

	StackProblem(Process *_proc): proc(_proc) {
		//stack::Value v(stack::SP, 0);
		//set(_init, 1, v);
	}

	void initialize(const hard::Register *reg, const Address& address) {
		stack::Value v;
		if(address.isNull())
			v = stack::Value(stack::SP, 0);
		else
			v = stack::Value(stack::CST, address.offset());
		set(_init, reg->platformNumber(), v);
	}

	inline const Domain& bottom(void) const { return stack::State::EMPTY; }
	inline const Domain& entry(void) const { TRACED(cerr << "entry() = " << _init << io::endl); return _init; }
	inline void lub(Domain &a, const Domain &b) const { a.join(b); }
	inline void assign(Domain &a, const Domain &b) const { a = b; }
	inline bool equals(const Domain &a, const Domain &b) const { return a.equals(b); }
	inline void enterContext(Domain &dom, BasicBlock *header, util::hai_context_t ctx) { }
	inline void leaveContext(Domain &dom, BasicBlock *header, util::hai_context_t ctx) { }

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

	void addAddress(Inst *inst, bool store, const stack::Value& v) {
		switch(v.kind()) {
		case stack::ALL:
		case stack::NONE:
			addrs.add(new AccessedAddress(inst, store));
			break;
		case stack::CST:
			addrs.add(new AbsAddress(inst, store, v.value()));
			break;
		case stack::SP:
			addrs.add(new SPAddress(inst, store, v.value()));
			break;
		case stack::REG:
			ASSERT(false);
			break;
		}
	}

	void update(Domain& out, const Domain& in, BasicBlock* bb) {
		int pc;
		out.copy(in);
		Domain *state;
		TRACEU(cerr << "update(BB" << bb->number() << ", " << in << ")\n");
		for(BasicBlock::InstIterator inst(bb); inst; inst++) {
			TRACEU(cerr << '\t' << inst->address() << ": "; inst->dump(cerr); cerr << io::endl);

			// get instructions
			b.clear();
			inst->semInsts(b);
			pc = 0;
			state = &out;

			// perform interpretation
			while(true) {

				// interpret current
				while(pc < b.length()) {
					sem::inst& i = b[pc];
					TRACES(cerr << "\t\t" << i << io::endl);
					switch(i.op) {
					case sem::BRANCH:
					case sem::TRAP:
					case sem::CONT:
						pc = b.length();
						TRACES(cerr << "\t\tcut\n");
						break;
					case sem::IF:
						todo.push(pair(pc + i.b() + 1, new Domain(*state)));
						break;
					case sem::NOP: break;
					case sem::LOAD: {
							stack::Value addr = get(*state, i.a());
							addAddress(inst, false, addr);
							set(*state, i.d(), state->get(addr, proc, i.b()));
						} break;
					case sem::STORE: {
							stack::Value addr = get(*state, i.a());
							addAddress(inst, true, addr);
							state->set(addr, get(*state, i.d()));
						} break;
					case sem::SETP:
					case sem::CMP:
					case sem::CMPU:
					case sem::ASR:
					case sem::SCRATCH:
						set(*state, i.d(), stack::Value::all);
						break;
					case sem::SET: {
							stack::Value v = get(*state, i.a());
							set(*state, i.d(), v);
						} break;
					case sem::SETI: {
							stack::Value v(stack::CST, i.cst());
							set(*state, i.d(), v);
						} break;
					case sem::ADD: {
							stack::Value v = get(*state, i.a());
							v.add(get(*state, i.b()));
							set(*state, i.d(), v);
						} break;
					case sem::SUB: {
							stack::Value v = get(*state, i.a());
							v.sub(get(*state, i.b()));
							set(*state, i.d(), v);
						} break;
					case sem::SHL: {
							stack::Value v = get(*state, i.a());
							v.shl(get(*state, i.b()));
							set(*state, i.d(), v);
						} break;
					case sem::SHR: {
							stack::Value v = get(*state, i.a());
							v.shr(get(*state, i.b()));
							set(*state, i.d(), v);
						} break;
					}
					pc++;
				}

				// pop next
				if(state != &out) {
					out.join(*state);
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
			TRACEI(cerr << "\t -> " << out << io::endl);
		}
		TRACEU(cerr << "\tout = " << out << io::endl);

		// record the addresses
		if(addrs) {
			AccessedAddresses *aa = ADDRESSES(bb);
			if(!aa) {
				aa = new AccessedAddresses();
				ADDRESSES(bb) = aa;
			}
			aa->set(addrs);
			addrs.clear();
		}
	}

private:
	stack::Value tmp[16];
	stack::State _init;
	sem::Block b;
	genstruct::Vector<Pair<int, Domain *> > todo;
	genstruct::Vector<AccessedAddress *> addrs;
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
 */
StackAnalysis::StackAnalysis(p::declare& r): Processor(r) {
}


p::declare StackAnalysis::reg = p::init("otawa::StackAnalysis", Version(1, 0, 0))
	.maker<StackAnalysis>()
	.require(LOOP_INFO_FEATURE)
	.require(VIRTUALIZED_CFG_FEATURE)
	.provide(STACK_ANALYSIS_FEATURE)
	.provide(ADDRESS_ANALYSIS_FEATURE);


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
	StackProblem prob(ws->process());
	const hard::Register *sp = ws->process()->platform()->getSP();
	if(sp)
		prob.initialize(sp, Address::null);
	for(int i = 0; i < inits.count(); i++)
		prob.initialize(inits[i].fst, inits[i].snd);
	StackListener list(ws, prob);
	StackFP fp(list);
	StackAI sai(fp, *ws);
	sai.solve(cfg);

	// output the result
	/* cout << "FUNCTION " << cfg->label() << io::endl;
	for(CFG::BBIterator bb(cfg); bb; bb++) {
		cout << "BB " << bb->number()
			 << " (" << bb->address() << ") =\n";
		AccessedAddresses *aa = ADDRESSES(bb);
		if(aa)
			out << aa;
	}*/
}


/**
 */
void StackAnalysis::configure(const PropList &props) {
	Processor::configure(props);
	for(Identifier<init_t>::Getter init(props, INITIAL); init; init++)
		inits.add(init);
}


/**
 * This features ensure that the stack analysis has been identified.
 * @par Default Processor
 * @li @ref otawa::StackAnalysis
 */
Feature<StackAnalysis> STACK_ANALYSIS_FEATURE("otawa::STACK_ANALYSIS_FEATURE");

}	//otawa
