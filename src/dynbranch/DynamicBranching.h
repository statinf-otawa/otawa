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

#include <elm/genstruct/Vector.h>
#include <elm/system/StopWatch.h>
#include <otawa/otawa.h>
#include <otawa/ipet.h>
#include <otawa/cfg.h>
#include <otawa/cfg/features.h>
#include <otawa/prog/sem.h>
#include <otawa/data/clp/ClpAnalysis.h>
#include <otawa/data/clp/ClpPack.h>
#include "PotentialValue.h"
#include "GlobalAnalysis.h"
#include "MemType.h"

// TODO	Move it in separate file.
#define DEBUG true
#define NB_EXEC 1000
#define PRINT_DEBUG(a) if(isDebug) { cout << a << endl ; } 

namespace otawa { namespace dynbranch {

//using namespace otawa;

class DynamicBranchingAnalysis: public BBProcessor {
public:
	static p::declare reg;
	DynamicBranchingAnalysis(p::declare& r = reg);
	void processBB(WorkSpace*, CFG *cfg , BasicBlock *bb);
	void configure(const PropList &props) ;

private:
	void addTargetToBB(BasicBlock*) ;
	PotentialValue find(MemID id,clp::State clpin , global::State globalin, Vector<sem::inst> semantics) ;
	bool isDebug ;
	bool time ;
};

extern p::feature FEATURE;
extern Identifier<bool> TIME ;

} } // otawa::dynbranch

#endif	// OTAWA_DYNAMIC_BRANCHING_ANALYSIS_H

