/*
 *	GlobalAnalysisProblem class interface
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
 *	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */
#ifndef OTAWA_DYNBRANCH_GLOBAL_ANALYSIS_PROBLEM_H
#define OTAWA_DYNBRANCH_GLOBAL_ANALYSIS_PROBLEM_H

#include <otawa/otawa.h>
#include <otawa/ipet.h>
#include <otawa/cfg.h>
#include <otawa/cfg/features.h>
#include <otawa/prog/sem.h>
#include <otawa/dfa/State.h>

#define GLOBAL_MEMORY_LOADER true

using namespace otawa;

namespace otawa { namespace dynbranch {

extern Identifier<bool> DEBUG_INFO;

class GlobalAnalysisProblem {
public:
	typedef FastStateWrapper Domain;
	GlobalAnalysisProblem(WorkSpace* workspace, bool verbose, Domain & entry, MyGC* m);
	~GlobalAnalysisProblem(void);

	const Domain& bottom();
	const Domain& top();
	const Domain& entry();
	void lub(Domain& a, const Domain& b) const;
	void assign(Domain& a, const Domain &b) const;
	void widening(otawa::Block* ob, Domain& a, Domain b) const;
	void updateEdge(Edge *edge, Domain& d);
	bool equals(const Domain &a, const Domain &b) const;
	void update(Domain& out, const Domain& in, Block *b);
	void enterContext(Domain &dom, Block *header, dfa::hai::hai_context_t ctx) { }
	void leaveContext(Domain &dom, Block *header, dfa::hai::hai_context_t ctx) { }
	inline Vector<PotentialValue>* getTempRegs() { return _tempRegs; }

	inline void printTempRegs(string begin = "", string end="") {
		int j = 0;
		for(Vector<PotentialValue>::Iterator i(*_tempRegs); i(); i++, j++) {
			if(((*i).bTop == false) && ((*i).length() == 0))
				continue;
		}
	}

	inline void setReg(Domain& out, int regNum, const PotentialValue& pv) {
		if(regNum >= 0)
			out.setReg(regNum, pv);
		else {
			_tempRegs->set(-regNum, pv);
		}
	}

	inline const PotentialValue& readReg(Domain& out, int regNum) {
		if(regNum >= 0)
			return out.readReg(regNum);
		else
			return _tempRegs->get(-regNum);
	}

	inline unsigned int get_nb_bb_count(void) { return _nb_bb_count; }

	void getStats(void);

private:
	WorkSpace* ws;
	dfa::State* istate;
	Domain bot;
	Domain topd;
	Domain ent;
	bool verbose;
	Vector<PotentialValue>* _tempRegs;
	MyGC* myGC;
	unsigned int _nb_bb_count;

	int bbCount;
	unsigned int** processBBFreq;
	unsigned int* processCFGFreq;

};


// need to implement when necessary
inline bool operator==(const GlobalAnalysisProblem::Domain& a, const GlobalAnalysisProblem::Domain& b) {
	ASSERT(0);
	return false;
}

}} // otawa::dynbranch

#endif	// OTAWA_DYNBRANCH_GLOBAL_ANALYSIS_PROBLEM_H
