/*
 *	ConditionalRestructurer class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2016, IRIT UPS.
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

#include <otawa/cfg/ConditionalRestructurer.h>
#include <otawa/prog/VirtualInst.h>

#define DO_DEBUG

namespace otawa {

/*
 * ConditionalRestructure provides a minimal implementation to support efficiently
 * and easily the conditional instruction. Yet, it remains space for improvement:
 *
 * * when a condition is refining a current condition, it would be enough to apply
 *   the refined condition to the initial assertion (as it will separated according to the
 *   new refinement),
 *
 * * the current system supports only condition of the semantic instructions (that are
 *   the main user of this module); yet, even non-standard condition could be supported
 *   as the condition system needs only to create an inclusion order on the conditions
 *   and the possibly to refine a condition getting the refinement and the complement
 *   of the refinement relatively to the current condition.
 */


/**
 * Class representing an instruction turned into a NOP but providing an "assume".
 */
class GuardNOP: public NOP {
public:
	GuardNOP(WorkSpace *ws, Inst *i, const Condition& cond)
		: NOP(ws, i), _cond(cond) { }

	int semInsts(sem::Block &block, int t) override {
		if(!_cond.isEmpty())
			block.add(sem::assume(_cond.semCond(), _cond.reg()->platformNumber()));
		return t;
	}

#	ifdef DO_DEBUG
	void dump(io::Output &out) override {
		if(!_cond.isEmpty())
			out << "(G)  ";
		else
			out << "(C) ";
		NOP::dump(out);
	}
#	endif

private:
	Condition _cond;
};


/**
 * Class representing a conditional instruction expressing the condition with an
 * "assume" and the semantic block without condition.
 */
class GuardInst: public VirtualInst {
public:
	GuardInst(WorkSpace *ws, Inst *i, const Condition& cond): VirtualInst(ws, i), _cond(cond) { }

	int semInsts(sem::Block &block, int t) override {
		if(!_cond.isEmpty())
			block.add(sem::assume(_cond.semCond(), _cond.reg()->platformNumber()));
		return VirtualInst::semKernel(block, t);
	}

#	ifdef DO_DEBUG
	void dump(io::Output &out) override {
		if(!_cond.isEmpty())
			out << "(G)  ";
		else
			out << "(C) ";
		VirtualInst::dump(out);
	}
#	endif

private:
	Condition _cond;
};

/**
 * Class representing a conditional instruction executed in a context corresponding
 * to its condition and therefore that does not need to expression the condition.
 */
class CondInst: public VirtualInst {
public:
	CondInst(WorkSpace *ws, Inst *i): VirtualInst(ws, i) { }

	int semInsts(sem::Block& block, int t) override {
		return VirtualInst::semKernel(block, t);
	}

#	ifdef DO_DEBUG
	void dump(io::Output &out) override {
		out << "(C)  ";
		VirtualInst::dump(out);
	}
#	endif

};


/// Flags which branch edges needs to be generated.
static const t::uint8
	NONE		= 0b00,		// no branch edge generated
	TAKEN		= 0b01,		// taken branch edge generated
	NOT_TAKEN	= 0b10,		// not-taken branch edge generated
	BOTH		= 0b11;		// taken and not-taken branch edges generated

/**
 * Attached to a block to represent the different versions.
 * The pair is made of a new basic block and a branch mode
 * (one of NONE, TAKEN, NOT_TAKEN or BOTH).
 */
static p::id<Pair<Block *, int> > BB("");


/**
 * In the case where a loop header has to be duplicated, it
 * is first replaced by a virtual node (maintaining the simple)
 * loop structure that is tied to the conditional versions of
 * the header.
 */
static p::id<Block *> HD("");

/**
 * This feature ensures that the CFG is transformed to reflect the effects of conditional instructions.
 *
 * Default implementation:
 * @li @ref ConditionalRestructurer
 */
p::feature CONDITIONAL_RESTRUCTURED_FEATURE("otawa::CONDITIONAL_RESTRUCTURED_FEATURE", p::make<ConditionalRestructurer>());


/**
 */
p::declare ConditionalRestructurer::reg = p::init("otawa::ConditionalRestructurer", Version(2, 0, 0))
	.use(LOOP_HEADERS_FEATURE)
	.require(VIRTUAL_INST_FEATURE)
	.provide(CONDITIONAL_RESTRUCTURED_FEATURE)
	.extend<CFGTransformer>()
	.make<ConditionalRestructurer>();


/**
 */
ConditionalRestructurer::ConditionalRestructurer(p::declare& r)
: CFGTransformer(r), _nop(nullptr), _anop(nullptr) {
}


/**
 */
void ConditionalRestructurer::transform (CFG *g, CFGMaker &m) {

	// split the basic blocks
	for(CFG::BlockIter b(g->blocks()); b(); b++)
		split(*b);
	BB(g->entry()) = pair(m.entry(), int(BOTH));
	BB(g->exit()) = pair(m.exit(), int(BOTH));
	if(g->unknown())
		BB(g->unknown()) = pair(m.unknown(), int(BOTH));

	// re-build the CFG
	for(CFG::BlockIter b(g->blocks()); b(); b++)
		make(*b);
}


class Status {
public:

	typedef enum {
		DISABLED,
		NO_MATCH,
		MATCH
	} match_t;

	Status() {}

	Status(Inst *i): _inst(i), icnt(1), _ena(true) {
		auto c = i->condition();
		_reg = c.reg();
		_unsigned = c.isUnsigned();
		_part = c.cond();
	}

	match_t match(Inst *i) {
		auto ic = i->condition();
		if(ic.reg() != _reg)
			return NO_MATCH;
		if(!_ena)
			return DISABLED;
		icnt++;
		if(_part != 0) {
			if(ic.isUnsigned() != _unsigned) {
				_part = 0;
				icnt = 0;
			}
			else if(ic.cond() != _part
			&& (ic.cond() != (~_part & Condition::ANY)))
				_part = Condition::ANY;
		}
		return MATCH;
	}

	void write(const RegSet& rs) {
		if(!_ena && rs.contains(_reg->platformNumber()))
			_ena = false;
	}

	inline bool worths() const { return _part != 0 && icnt > 1; }
	inline Inst *inst() const { return _inst; }
	inline hard::Register *reg() const { return _reg; }
	inline int count() const { return icnt; }
	inline int part() const { return _part; }

	void getCases(Condition cs[3]) const {
		if(_part == Condition::ANY) {
			cs[0] = Condition(_unsigned, Condition::EQ, _reg);
			cs[1] = Condition(_unsigned, Condition::LT, _reg);
			cs[2] = Condition(_unsigned, Condition::GT, _reg);
		}
		else {
			cs[0] = Condition(_unsigned, _part, _reg);
			cs[1] = Condition(_unsigned, (~_part) & Condition::ANY, _reg);
			cs[2] = Condition();
		}
	}

private:
	Inst *_inst;
	hard::Register *_reg;
	int icnt;
	t::uint8 _part;
	bool _ena, _unsigned;
};


/**
 * Split the block according to the conditions.
 * Alternatives of the block are stored on the block itself using
 * BB identifier.
 *
 * @param b	Split block.
 */
void ConditionalRestructurer::split(Block *b) {

	// throw up non BB
	if(b->isEnd())
		return;
	if(b->isSynth()) {
		Block *cb = build(b->toSynth()->callee());
		BB(b) = pair(cb, int(BOTH));
		return;
	}
	if(!b->isBasic())
        return;
	auto bb = b->toBasic();

	// build the statuses
	Vector<Status> status;
	RegSet wr;
	for(auto i: *bb) {
		auto c = i->condition();
		if(!c.isEmpty()) {

			// find corresponding status
			int p = status.length() - 1;
			bool matched = false;
			while(p >= 0)
				switch(status[p].match(i)) {
				case Status::MATCH:
					matched = true;
				case Status::DISABLED:
					p = -1;
					break;
				case Status::NO_MATCH:
					break;
				}

			// or create a new one
			if(!matched)
				status.add(Status(i));

			// update status according to writes
			wr.clear();
			i->writeRegSet(wr);
			for(auto& s: status)
				s.write(wr);
		}
	}

	// prepare the BB
	typedef struct case_t {

		case_t(): branch(BOTH) {}

		case_t *duplicate() const {
			auto r = new case_t;
			r->conds = conds;
			r->insts = insts;
			r->branch = branch;
			return r;
		}

		void add(Inst *i, t::uint b, bool is_control) {
			insts.add(i);
			if(is_control)
				branch = b;
		}

		Vector<Condition::cond_t> conds;
		Vector<Inst *> insts;
		t::uint8 branch;
	} case_t;

	Vector<case_t *> cases;
	cases.add(new case_t);
	int cc = 0;

	for(auto inst: *bb) {
		auto icond = inst->condition();

		// new condition
		if(cc < status.length() && status[cc].inst() == inst) {
			if(!status[cc].worths())
				for(auto c: cases) {
					c->conds.add(Condition().cond());
					c->add(inst, BOTH, inst->isControl());
				}

			else {
				Condition conds[3];
				status[cc].getCases(conds);
				auto clen = cases.length();
				for(int i = 0; i < 3; i++) {
					if(conds[i].isEmpty())
						continue;

					// select the instruction
					auto mcond = icond & conds[i];
					Inst *added_inst;
					t::uint8 branch;
					if(!mcond.isEmpty()) {
						added_inst = guard(inst, mcond);
						branch = TAKEN;
					}
					else {
						added_inst = nop(inst, ~mcond & conds[i]);
						branch = NOT_TAKEN;
					}

					// need to build
					int offset = 0;
					if(i >= 1) {
						offset = cases.length();
						for(int j = 0; j < clen; j++) {
							cases.add(cases[j]->duplicate());
							cases.top()->conds.pop();
							cases.top()->insts.pop();
						}
					}

					// add condition and guard
					for(int j = 0; j < clen; j++) {
						cases[j + offset]->conds.add(conds[i].cond());
						cases[j + offset]->add(added_inst, branch, false);
					}
				}
			}
			cc++;
		}

		// no condition: add to all
		else if(icond.isEmpty())
			for(auto c: cases)
				c->insts.add(inst);

		// condition: sometimes add inst, sometimes NOP
		else {
			auto nop = new NOP(workspace(), inst);
			auto ninst = cond(inst);
			auto c = inst->condition();

			// find condition
			int i = cc - 1;
			while(status[i].reg() != icond.reg())
				i--;
			ASSERT(i >= 0);

			// decorate cases
			for(auto ca: cases)
				if(c.cond() & ca->conds[i])
					ca->add(ninst, TAKEN, inst->isControl());
				else
					ca->add(nop, NOT_TAKEN, inst->isControl());
		}
	}

	// header block with several cases
	Block *h = nullptr;
	if(LOOP_HEADER(b) && cases.length() > 1) {
		h = build();
		HD(b) = h;
	}

	// create the duplicates
	for(auto c: cases) {
		Block *nbb = build(c->insts.detach());
		BB(bb).add(pair(nbb, int(c->branch)));
		if(h != nullptr)
			build(h, nbb, 0);
		delete c;
	}

}


/**
 * Build a nop for the given instruction.
 * @param i		Instruction to build nop for.
 * @param c		Condition supported by the nop.
 */
Inst *ConditionalRestructurer::nop(Inst *i, const Condition& c) {
	if(_anop != i || !c.isEmpty()) {
		_nop = new GuardNOP(workspace(), i, c);
		_anop = i;
	}
	return _nop;
}


/**
 * Build a guarded instruction shield.
 * @param i		Guarded instruction.
 * @param cond	Condition guarding the instruction.
 * @return		Corresponding guarded instruction.
 */
Inst *ConditionalRestructurer::guard(Inst *i, const Condition& cond) {
	return new GuardInst(workspace(), i, cond);
}


/**
 * Build an instruction executing in its condition context (no need for
 * condition).
 * @param i		Condition instruction.
 * @return		Corresponding condition instruction.
 */
Inst *ConditionalRestructurer::cond(Inst *i) {
	return new CondInst(workspace(), i);
}


/**
 * Test if duplicating the BB at the given instruction with the given condition
 * is useful or not, i.e. if the condition will apply to several instructions
 * (in sequence).
 * @param i		Current instruction.
 * @param cond	Instruction condition.
 */
bool ConditionalRestructurer::useful(BasicBlock::InstIter i, const Condition& cond) {
	i++;
	if(i.ended())
		return false;
	auto nc = (*i)->condition();
	return !nc.isEmpty() && !nc.isAny() && nc.reg() == cond.reg();
}


/**
 * Build the corresponding blocks.
 * @param b		Block to rebuild.
 */
void ConditionalRestructurer::make(Block *b) {
	for(Block::EdgeIter e = b->outs(); e(); e++)
		for(auto sb: BB.all(b))
			if((e->isTaken() && ((sb.snd & TAKEN) != 0))			// taken and taken generated
			|| (e->isNotTaken() && ((sb.snd & NOT_TAKEN) != 0))	// not-taken and not-taken generated
			|| (!e->isTaken() && !e->isNotTaken()))
			{
				if(HD(e->sink()) != nullptr)
					build(sb.fst, HD(e->sink()), e->flags());
				else
					for(auto tb: BB.all(e->sink()))
						build(sb.fst, tb.fst, e->flags());
			}
}

} // otawa
