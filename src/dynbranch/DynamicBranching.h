/*
 *	DynamicBranchingAnalysis class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2014, IRIT UPS.
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
#ifndef OTAWA_DYNAMIC_BRANCHING_ANALYSIS_H
#define OTAWA_DYNAMIC_BRANCHING_ANALYSIS_H

#include <elm/data/Vector.h>
#include <elm/sys/StopWatch.h>
#include <otawa/otawa.h>
#include <otawa/ipet.h>
#include <otawa/cfg.h>
#include <otawa/cfg/features.h>
#include <otawa/prog/sem.h>
#include <otawa/data/clp/ClpAnalysis.h>
#include <otawa/data/clp/ClpPack.h>
#include <otawa/data/clp/features.h>
#include "PotentialValue.h"
#include "MemType.h"

// TODO	Move it in separate file.
#define DEBUGDYN true
#define NB_EXEC 1000
#define PRINT_DEBUG(a) if(isDebug) { cout << a << endl ; }

namespace otawa { namespace dynbranch {

extern p::feature FEATURE;
extern Identifier<bool> TIME ;

class DynamicBranchingAnalysis: public BBProcessor {
public:
	static p::declare reg;
	DynamicBranchingAnalysis(p::declare& r = reg);
	~DynamicBranchingAnalysis();
	void processBB(WorkSpace*, CFG *cfg , Block *b);
	void configure(const PropList &props) ;

private:
	void addTargetToBB(BasicBlock*) ;
	PotentialValue find(BasicBlock* bb, MemID id, const clp::State & clpin, State & globalin, Vector<sem::inst> semantics) ;
	void takingCLPValueIfNecessary(PotentialValue& pv, int semanticInstIndex, const clp::Value& regOrAddr);
	unsigned int readFromMem(unsigned int address, sem::type_t type);
	bool isDebug ;
	bool time ;
	Vector<PotentialValue> _regValues;
	WorkSpace* _workspace;
	clp::Manager* clpManager;
	Vector<otawa::clp::State> clpState;
	dfa::State *istate;
	bool first;
};
} } // otawa::dynbranch

#endif	// OTAWA_DYNAMIC_BRANCHING_ANALYSIS_H

