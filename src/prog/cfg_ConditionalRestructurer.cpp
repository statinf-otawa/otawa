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
class MyNOP: public NOP {
public:
	MyNOP(WorkSpace *ws, Inst *i, const Condition& cond): NOP(ws, i), _cond(cond) { }

	virtual void semInsts (sem::Block &block) {
		if(!_cond.isEmpty())
			block.add(sem::assume(_cond.semCond(), _cond.reg()->platformNumber()));
	}

private:
	Condition _cond;
};

class CondInst: public VirtualInst {
public:
	CondInst(WorkSpace *ws, Inst *i, const Condition& cond): VirtualInst(ws, i), _cond(cond) { }

	virtual void semInsts (sem::Block &block) {
		if(!_cond.isEmpty()) {
			block.add(sem::assume(_cond.semCond(), _cond.reg()->platformNumber()));
			VirtualInst::semKernel(block);
		}
	}

private:
	Condition _cond;
};

static const t::uint8
	NONE		= 0b00,
	TAKEN		= 0b01,
	NOT_TAKEN	= 0b10,
	BOTH		= 0b11;

/**
 * Attached to a block to represent the different versions.
 * The pair is made of a new basic block and a branch mode
 * (one of NONE, TAKEN, NOT_TAKEN or BOTH).
 */
static p::id<Pair<Block *, int> > BB("");

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
	.require(VIRTUAL_INST_FEATURE)
	.provide(CONDITIONAL_RESTRUCTURED_FEATURE)
	.extend<CFGTransformer>()
	.make<ConditionalRestructurer>();


/**
 */
ConditionalRestructurer::ConditionalRestructurer(p::declare& r)
: CFGTransformer(r), _nop(0), _anop(0) {
}


/**
 */
void ConditionalRestructurer::transform (CFG *g, CFGMaker &m) {

	// split the basic blocks
	for(CFG::BlockIter b(g->blocks()); b; b++)
		split(b);
	BB(g->entry()) = pair(m.entry(), int(BOTH));
	BB(g->exit()) = pair(m.exit(), int(BOTH));
	if(g->unknown())
		BB(g->unknown()) = pair(m.unknown(), int(BOTH));

	// re-build the CFG
	for(CFG::BlockIter b(g->blocks()); b; b++)
		make(b);
}


// basic block
class Case {
public:
	inline Case(void): _bra(BOTH) { }
	inline t::uint8 branch(void) const { return _bra; }

	/**
	 * Remove conditions matching the condition which register is written.
	 * @param wr	Written registers.
	 */
	void remove(const RegSet& wr) {
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
		remove(wr);
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

	inline genstruct::Table<Inst *> insts(void) { return _insts.detach(); }

private:
	Vector<Condition> _conds;
	genstruct::Vector<Inst *> _insts;
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
		BB(b) = pair(cb, 0);
		return;
	}

	// split the block
	RegSet wr;
	BasicBlock *bb = b->toBasic();
	Vector<Case *> cases;
	cases.add(new Case());
	for(BasicBlock::InstIter i(bb); i; i++) {
		Condition c = i->condition();
		wr.clear();
		i->writeRegSet(wr);

		// no condition or final branch alone: just add the instruction
		if(c.isEmpty() || (cases.length() == 1 && i == bb->control())) {
			for(Vector<Case *>::Iter k(cases); k; k++)
				k->add(i, wr);
		}

		// any condition: duplicate all cases
		else if(c.isAny()) {
			int l = cases.length();
			for(int k = 0; k < l; k++) {
				cases.add(cases[k]->split(nop(i), wr));
				cases[k]->add(i, wr);
			}
		}

		// look at each condition
		else {
			int l = cases.length();
			for(int k = 0; k < l; k++) {
				Condition cc = cases[k]->matches(c);

				// cc = no condition => split
				if(cc.isEmpty()) {
					Condition ic = c.inverse();
					cases.add(cases[k]->split(nop(i, ic), wr, ic));
					cases[k]->add(guard(i, c), wr, c);
				}

				// c subset of cc => add instruction
				else if(cc <= c)
					cases[k]->add(i, wr, i->isControl() ? TAKEN : NONE);

				// cc subset of c => split
				else if(c & cc) {
					Condition ic = cc - (c & cc);
					cases.add(cases[k]->split(nop(i, ic), wr, ic, i->isControl() ? NOT_TAKEN : NONE));
					cases[k]->add(guard(i, c & cc), wr, c & cc, i->isControl() ? TAKEN : NONE);
				}

				// cc is out of c => not executed
				else
					cases[k]->add(nop(i), wr, i->isControl() ? NOT_TAKEN : NONE);
			}
		}

	}

	// build the block and clean
	for(Vector<Case *>::Iter k(cases); k; k++) {
		BB(bb).add(pair(static_cast<Block *>(build(k->insts())), int(k->branch())));
		delete *k;
	}
}


/**
 * Build a nop for the given instruction.
 * @param i		Instruction to build nop for.
 * @param c		Condition supported by the nop.
 */
Inst *ConditionalRestructurer::nop(Inst *i, const Condition& c) {
	if(_anop != i || !c.isEmpty()) {
		_nop = new MyNOP(workspace(), i, c);
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
	return new CondInst(workspace(), i, cond);
}


/**
 * Build the corresponding blocks.
 * @param b		Block to rebuild.
 */
void ConditionalRestructurer::make(Block *b) {
	cerr << "DEBUG:  " << b << io::endl;
	for(Block::EdgeIter e = b->outs(); e; e++)
		for(p::id<Pair<Block *, int> >::Getter sb(b, BB); sb; sb++)
			if(!e->flags()
			|| (e->isTaken() && ((*sb).snd & TAKEN))
			|| (e->isNotTaken() && ((*sb).snd & NOT_TAKEN)))
				for(p::id<Pair<Block *, int> >::Getter tb(e->target(), BB); tb; tb++) {
					build((*sb).fst, (*tb).fst, e->flags());
					cerr << "DEBUG: " << (*sb).fst << " -> " << (*tb).fst << io::endl;
				}
}

} // otawa
