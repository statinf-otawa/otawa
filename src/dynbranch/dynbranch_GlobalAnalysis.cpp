/*
 *	GlobalAnalysis class implementation
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
#include "GlobalAnalysis.h"
#include "GlobalAnalysisProblem.h"
#include <otawa/otawa.h>
#include <otawa/util/WideningListener.h>
#include <otawa/util/WideningFixPoint.h>
#include <otawa/util/HalfAbsInt.h>
#include <otawa/dfa/FastState.h>

namespace otawa { namespace dynbranch {

/**
 * @class GlobalAnalysis
 * TODO
 */

p::declare GlobalAnalysis::reg = p::init("GlobalAnalysisFeature", Version(1,0,0))
	.require(COLLECTED_CFG_FEATURE)
	.require(LOOP_INFO_FEATURE)
	.require(dfa::INITIAL_STATE_FEATURE)
	.provide(GLOBAL_ANALYSIS_FEATURE);

/**
 */
GlobalAnalysis::GlobalAnalysis(p::declare& r): Processor(r), time(false) {
}


/**
 */
void GlobalAnalysis::configure(const PropList &props) {
	Processor::configure(props);
	//GLOBAL_STATE_ENTRY(props);
	//entry(&pv, dfa::INITIAL_STATE(workspace()), allocator);
	time = TIME(props);
}


/**
 */
void GlobalAnalysis::processWorkSpace(WorkSpace *ws) {
	const CFGCollection *coll = INVOLVED_CFGS(ws);
	ASSERT(coll);
	CFG *cfg = coll->get(0) ;

	istate = dfa::INITIAL_STATE(ws);

	system::StopWatch watch ;

	elm::StackAllocator* psa = new elm::StackAllocator(); // need to keep this in the heap so that the content of the fastState can be used between the Processors
	dfa::FastState<PotentialValue> *fs = new dfa::FastState<PotentialValue>(&pv, dfa::INITIAL_STATE(ws), *psa);
	entry.setFastState(fs);

	dynbranch::GlobalAnalysisProblem prob(ws,isVerbose(), entry);
	WideningListener<dynbranch::GlobalAnalysisProblem> list(ws, prob);
	WideningFixPoint<WideningListener<dynbranch::GlobalAnalysisProblem> > fp(list);
	HalfAbsInt<WideningFixPoint<WideningListener<dynbranch::GlobalAnalysisProblem> > > hai(fp, *ws);
	hai.solve(cfg);

	// Check the results
	int i = 0 ;
	for(CFGCollection::Iterator cfgi(*coll); cfgi; cfgi++) {
		for(otawa::CFG::BlockIter bbi = cfgi->blocks(); bbi; bbi++) {
			if(!bbi->isBasic())
				continue;

			GLOBAL_STATE_IN(*bbi) = *list.results[cfgi->index()][bbi->index()] ;
		}
	}


	if (time) {
		watch.start() ;
		for (int i=0 ; i < TIME_NB_EXEC_GLOBAL ; i++) {hai.solve(cfg) ; }
		watch.stop() ;
		otawa::ot::time t = watch.delay() ;
		cout << " ---------- Time stat for Global Analysis -----------" << endl  ;
		cout << " Number executions : " << TIME_NB_EXEC_GLOBAL << endl ;
		cout << " Total time (microsec): " << t << endl ;
		cout << " Average time (microsec): " << t/TIME_NB_EXEC_GLOBAL << endl ;
		cout << " ----------------------------------------------------" << endl  ;
	}
}


/**
 * TODO
 */
p::feature GLOBAL_ANALYSIS_FEATURE("otawa::dynbranch::GLOBAL_ANALYSIS_FEATURE", new Maker<GlobalAnalysis>());


/**
 * TODO
 */
//Identifier<dynbranch::State*> GLOBAL_STATE_IN("otawa::dynbranch::GLOBAL_STATE_IN") ;
Identifier<dynbranch::Domain> GLOBAL_STATE_IN("otawa::dynbranch::GLOBAL_STATE_IN") ;


/**
 */
//Identifier<dynbranch::State*> GLOBAL_STATE_OUT("otawa::dynbranch::GLOBAL_STATE_OUT") ;
Identifier<dynbranch::Domain> GLOBAL_STATE_OUT("otawa::dynbranch::GLOBAL_STATE_OUT") ;


/**
 */
//Identifier<dynbranch::State*> GLOBAL_STATE_ENTRY("otawa::dynbranch::GLOBAL_STATE_ENTRY") ;
Identifier<dynbranch::Domain> GLOBAL_STATE_ENTRY("otawa::dynbranch::GLOBAL_STATE_ENTRY") ;


/**
 */
Identifier<bool> TIME("otawa::dynbranch::TIME") ;

Identifier<Vector<Pair<Address, Address> >* > DATA_IN_READ_ONLY_REGION("otawa::dynbranch::DATA_IN_READ_ONLY_REGION", 0);

} }	// otawa::dynbranch
