/*
 * 	BranchProblem class implementation
 *	Copyright (c) 2007, IRIT UPS <casse@irit.fr>
 *
 *	This file is part of OTAWA
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

#include <otawa/branch/BranchProblem.h>
#include <otawa/branch/CondNumber.h>
#include <otawa/cache/LBlockSet.h>
#include <otawa/cfg.h>
#include <otawa/cfg/Dominance.h>
#include <otawa/cfg/features.h>
#include <otawa/hard/BHT.h>
#include <otawa/ilp.h>
#include <otawa/ipet.h>
#include <otawa/cache/LBlockBuilder.h>
#include <otawa/cfg/features.h>


namespace otawa {

/*
 * Soundness of branch analysis requires:
 * 	\forall x \in D,
 * 		x U _ = _ U x = x
 * 		x U T = T U x = x
 *
 *  Age			U		_	T
 *  MUST		max		0	A
 *  PERS		max		_	A
 *  MAY			min		A	0
 */

/**
 * @class MUSTBranch
 * Implementation of MUST analysis for BHT.
 */

/**
 */
MUSTBranch::MUSTBranch(const int _size, const int _A, const int _row)
: row(_row), bot(_size, _A, 0), _top(_size, _A, _A)
	{ }

/**
 */
MUSTBranch::~MUSTBranch() {
}

/**
 */
void MUSTBranch::update(Domain& out, const Domain& in, Block* bb) {
		ASSERTP(false, "FATAL: PERSProblem is not to be used directly, use MUSTPERS instead.");
}

/**
 * Print the domain value.
 * @param output	Output to use.
 */
void MUSTBranch::Domain::print(elm::io::Output &output) const {
	bool first = true;
	output << "[";
	for (int i = 0; i < size; i++) {
		if (age[i] != A) {
			if (!first)
				output << ", ";
			output << i;
			output << ":";
			output << age[i];
			first = false;
		}
	}
	output << "]";
}


/**
 * @class MAYBranch
 * Implementation of MAY analysis for BHT.
 */

/**
 */
MAYBranch::MAYBranch(int _size, int _A, int _row)
: row(_row), bot(_size, _A, _A), _top(_size, _A, 0)
{ }

/**
 */
MAYBranch::~MAYBranch(void) {
}

/**
 */
void MAYBranch::update(Domain& out, const Domain& in, BasicBlock* bb) {
		ASSERTP(false, "FATAL: PERSProblem is not to be used directly, use MUSTPERS instead.");
}

/**
 * Print the domain value.
 * @param output	Output to use.
 */
void MAYBranch::Domain::print(elm::io::Output &output) const {
	bool first = true;
	output << "[";
	for (int i = 0; i < size; i++) {
		if (age[i] != A) {
			if (!first)
				output << ", ";
			output << i;
			output << ":";
			output << age[i];
			first = false;
		}
	}
	output << "]";
}


/**
 * @class PERSBranch
 * Implementation of PERS analysis for BHT.
 */

/**
 */
PERSBranch::PERSBranch(int _size,  int _A, int _row)
: row(_row), bot(_size, _A, BOT), _top(_size, _A, _A)
{ }

/**
 */
PERSBranch::~PERSBranch(void) {
}

void PERSBranch::update(Domain& out, const Domain& in, Block* bb)  {
	ASSERTP(false, "FATAL: PERSProblem is not to be used directly, use MUSTPERS instead.");
}

void PERSBranch::Domain::print(elm::io::Output &output) const {
	bool first = true;
	if (isBottom) {
		output << "BOTTOM";
		return;
	}
	output << "(W=";
	whole.print(output);
	output << ", ";
	for (int i = 0; i < data.length(); i++) {
		if (!first)
			output << "|";
		data[i]->print(output);
		first = false;
	}
	output << ")";
}

/**
 */
elm::io::Output& operator<<(elm::io::Output& output, const PERSBranch::Domain& dom) {
	dom.print(output);
	return output;
}


/**
 * Print an item.
 * @param output	Output to use.
 */
void PERSBranch::Item::print(elm::io::Output &output) const {
	bool first = true;
	output << "[";
	for (int i = 0; i < size; i++) {
		if (age[i] != -1) {
			if (!first)
				output << ", ";
			output << i << ":" << age[i];
			first = false;
		}
	}
	output << "]";
}


/**
 * Accumulated analysis problems for MUST, MAY and PERS
 */

/**
 */
BranchProblem::BranchProblem(const int _size,  WorkSpace *_fw,  const int _A, const int _row)
: 	bot(_size, _A), _top(_size, _A),
	mustProb(_size, _A, _row), persProb(_size, _A, _row), mayProb(_size, _A, _row),
	fw(_fw), row(_row)
{
		persProb.assign(bot.pers, persProb.bottom());
		mustProb.assign(bot.must, mustProb.bottom());
		persProb.assign(_top.pers, persProb.entry());
		mustProb.assign(_top.must, mustProb.entry());
}


/**
 */
void BranchProblem::update(Domain& out, const Domain& in, Block* b) {
	assign(out, in);
	if(!b->isBasic())
		return;
	BasicBlock *bb = b->toBasic();
	Inst *last = bb->control();

	if (last
	&& int(hard::BHT_CONFIG(fw)->line(last->address())) == row
	&& branch::COND_NUMBER(bb) != -1)
		out.inject(branch::COND_NUMBER(bb));
}

}	// otawa





