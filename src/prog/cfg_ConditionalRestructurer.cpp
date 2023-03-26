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

//#define DO_DEBUG

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
	GuardNOP(WorkSpace *ws, Inst *i, const Condition& cond): NOP(ws, i), _cond(cond) { }

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
p::declare ConditionalRestructurer::reg = p::init("otawa::ConditionalRestructurer", Version(1, 0, 0))
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


// basic block
class Case {
public:
	inline Case(): _bra(BOTH) { }

	Case(const Case& c): _bra(c._bra) {
		_conds = c._conds;
		_insts = c._insts;
	}

	/**
	 * Get the branch mask.
	 * @return	Branch mask.
	 */
	inline t::uint8 branch(void) const { return _bra; }

	/**
	 * Remove conditions matching the condition which register is written.
	 * @param wr	Written registers.
	 */
	void removeCond(const RegSet& wr) {
		int i = 0, j = 0;
		while(i < _conds.length())
			if(wr.contains(_conds[i].reg()->platformNumber()))
				i++;
			else {
				if(i != j)
					_conds[j] = _conds[i];
				i++;
				j++;
			}
		_conds.setLength(j);
	}

	/**
	 * Get the index of a condition.
	 * @param c	Looked condition.
	 * @return	Matching index or -1.
	 */
	int index(const Condition& c) {
		for(int i = 0; i < _conds.length(); i++)
			if(c.isSigned() == _conds[i].isSigned()
			&& c.reg() == _conds[i].reg())
				return i;
		return -1;
	}

	/**
	 * Find the case condition matching the given condition.
	 * @param c	Looked condition.
	 * @return	Matching condition or empty condition.
	 */
	Condition matches(const Condition& c) {
		int i = index(c);
		if(i >= 0)
			return _conds[i];
		else
			return Condition();
	}

	/**
	 * Add an instruction to the basic block.
	 * @param i		Added instruction.
	 * @param wr	Written registers.
	 * @param bra	Branch mode (default to NONE).
	 */
	void add(Inst *i, const RegSet& wr, t::uint8 bra = NONE) {
		_insts.add(i);
		removeCond(wr);
		if(bra != NONE)
			_bra = bra;
	}

	/**
	 * Add an instruction with its condition.
	 * @param i		Added instruction.
	 * @param wr	Written registers.
	 * @param c		Instruction condition.
	 * @param bra	Branch mode (default to NONE).
	 */
	void add(Inst *i, const RegSet& set, const Condition& c, t::uint8 bra = NONE) {
		int ci = index(c);
		if(ci >= 0)
			_conds[ci] = c;
		else
			_conds.add(c);
		add(i, set, bra);
	}

	/**
	 * Build a new case where the given instruction is added.
	 * @param i		Added instruction.
	 * @param wr	Written registers.
	 * @param bra	Branch mode (default to NONE).
	 */
	Case *split(Inst *i, const RegSet& wr, t::uint8 bra = NONE) {
		Case *nc = new Case(*this);
		nc->add(i, wr, bra);
		return nc;
	}

	/**
	 * Build a new case where the given instruction is added
	 * with the given condition.
	 * @param i		Added instruction.
	 * @param wr	Written registers.
	 * @param c		Instruction condition.
	 * @param bra	Branch mode (default to NONE).
	 */
	Case *split(Inst *i, const RegSet& wr, const Condition& c, t::uint8 bra = NONE) {
		Case *nc = new Case(*this);
		nc->add(i, wr, c, bra);
		return nc;
	}

	/**
	 * Take the instructions composing the case.
	 * @return	Instruction composing the case.
	 */
	inline Array<Inst *> insts(void) { return _insts.detach(); }

private:
	Vector<Condition> _conds;
	Vector<Inst *> _insts;
	t::uint8 _bra;
};


/**
 * Split the block according to the conditions.
 * Alternatives of the block are stored on the block itself using
 * BB identifier.
 *
 * @param b	Split block.
 */
void ConditionalRestructurer::split(Block *b) {

	// end block
	if(b->isEnd())
		return;

	// synthetic block
	if(b->isSynth()) {
		Block *cb = build(b->toSynth()->callee());
		BB(b) = pair(cb, int(BOTH));
		return;
	}

	if(!b->isBasic())
        return;

	// split the block
	RegSet wr;
	BasicBlock *bb = b->toBasic();
	Vector<Case *> cases;
	cases.add(new Case());
	for(auto i: *bb) {
		Condition ic = i->condition();
		wr.clear();
		i->writeRegSet(wr);

		// no condition: add i to all cases
		if(ic.isEmpty())
			for(auto k: cases)
				k->add(i, wr);

		// any condition: 1 case with i and 1 case with NOP
		else if(ic.isAny()) {
			int l = cases.length();
			for(int k = 0; k < l; k++) {
				cases.add(cases[k]->split(nop(i), wr));
				cases[k]->add(i, wr);
			}
		}

		// a solid condition: process each case
		else {
			int l = cases.length();
			for(int k = 0; k < l; k++) {
				Condition cc = cases[k]->matches(ic);

				// cc not already set: 1 case with ic and 1 case with not ic
				if(cc.isEmpty()) {

					// except it is the final branch
					if(i == bb->control() && i == bb->last())
						cases[k]->add(i, wr);

					// new condition: duplicate
					else {
						Condition nic = ic.inverse();
						cases.add(cases[k]->split(nop(i, nic), wr, nic, i->isControl() ? NOT_TAKEN : NONE));
						cases[k]->add(guard(i, ic), wr, ic, i->isControl() ? TAKEN : NONE);
					}
				}

				// cc subset of ic => add i to current case
				else if(cc <= ic)
					cases[k]->add(cond(i), wr, i->isControl() ? TAKEN : NONE);

				// cc meet ic != {} => current case has to be split: cc meet ic, cc\(ic meet cc)
				else if(ic & cc) {
					Condition mc = cc - (ic & cc);
					cases.add(cases[k]->split(nop(i, mc), wr, mc, i->isControl() ? NOT_TAKEN : NONE));
					cases[k]->add(guard(i, ic & cc), wr, ic & cc, i->isControl() ? TAKEN : NONE);
				}

				// i becomes NOP in this case
				else
					cases[k]->add(nop(i), wr, i->isControl() ? NOT_TAKEN : NONE);
			}
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
		Block *nbb = build(c->insts());
		BB(bb).add(pair(nbb, int(c->branch())));
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
 * Build the corresponding blocks.
 * @param b		Block to rebuild.
 */
void ConditionalRestructurer::make(Block *b) {
	for(Block::EdgeIter e = b->outs(); e(); e++)
		for(auto sb: BB.all(b)) {
			if((e->isTaken() && (sb.snd & TAKEN))			// taken and taken generated
			|| (e->isNotTaken() && (sb.snd & NOT_TAKEN))	// not-taken and not-taken generated
			|| (!e->isTaken() && !e->isNotTaken()))
			{
				if(HD(e->sink()) != nullptr)
					build(sb.fst, HD(e->sink()), e->flags());
				else
					for(auto tb: BB.all(e->sink()))
						build(sb.fst, tb.fst, e->flags());
			}
		}
}

} // otawa
