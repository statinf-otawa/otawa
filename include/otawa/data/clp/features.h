/*
 *	otawa::clp module features
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2013, IRIT UPS.
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
#ifndef OTAWA_CLP_FEATURES_H_
#define OTAWA_CLP_FEATURES_H_

#include <otawa/data/clp/ClpState.h>
#include <otawa/prog/sem.h>

namespace otawa {

namespace clp {

extern Identifier<bool> VERBOSE;

class ClpProblem;

class Manager {
public:
	typedef t::uint32 step_t;
	static const step_t
		NEW_SEM = 0x01,
		NEW_PATH = 0x02,
		NEW_INST = 0x04,
		ENDED = 0;
	static bool newSem(step_t s) { return s & NEW_SEM; }
	static bool newPath(step_t s) { return s & NEW_PATH; }
	static bool newInst(step_t s) { return s & NEW_INST; }
	static bool isEnded(step_t s) { return !s; }

	Manager(WorkSpace *ws);
	~Manager();
	step_t start(BasicBlock *bb);
	step_t next(void);
	sem::inst sem(void);
	Inst *inst(void);
	State *state(void);
	inline int ipc(void) { return i; }
	Value getCurrentAccessAddress(void);
	step_t rewind(State& rState);

private:
	ClpProblem *p;
	//BasicBlock::InstIter mi;
	BasicBlock::BundleIter mi;
	clp::State s, *cs;
	int i;
};

extern p::feature CLP_ANALYSIS_FEATURE;

extern Identifier<clp::State> STATE_IN;
extern Identifier<clp::State> STATE_OUT;
extern Identifier<bool> UNKOWN_BLOCK_EVALUATION;

} }		// otawa::clp

#endif /* OTAWA_CLP_FEATURES_H_ */
