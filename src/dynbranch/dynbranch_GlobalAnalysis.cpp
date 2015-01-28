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

namespace otawa { namespace dynbranch {

/**
 * @class GlobalAnalysis
 * TODO
 */

p::declare GlobalAnalysis::reg = p::init("GlobalAnalysisFeature", Version(1,0,0))
	.require(COLLECTED_CFG_FEATURE)
	.require(LOOP_INFO_FEATURE)
	.provide(GLOBAL_ANALYSIS_FEATURE);
;

/**
 */
GlobalAnalysis::GlobalAnalysis(p::declare& r): Processor(r), time(false) {
}


/**
 */
void GlobalAnalysis::configure(const PropList &props) {
	Processor::configure(props) ;
	entry = GLOBAL_STATE_ENTRY(props) ;
	time = TIME(props) ;
}


/**
 */
void GlobalAnalysis::processWorkSpace(WorkSpace *ws) {
	const CFGCollection *coll = INVOLVED_CFGS(ws);
	ASSERT(coll);
	CFG *cfg = coll->get(0) ;

	system::StopWatch watch ;
	global::GlobalAnalysisProblem prob(ws,isVerbose(), entry);
	WideningListener<global::GlobalAnalysisProblem> list(ws, prob);
	WideningFixPoint<WideningListener<global::GlobalAnalysisProblem> > fp(list);
	HalfAbsInt<WideningFixPoint<WideningListener<global::GlobalAnalysisProblem> > > hai(fp, *ws);
	hai.solve(cfg);

	// Check the results
	int i = 0 ;
	for(otawa::CFG::BBIterator bbi(cfg); bbi; bbi++){
		GLOBAL_STATE_IN(*bbi) = *list.results[0][bbi->number()] ;
		otawa::CFG::BBIterator nextit(bbi) ;
		if (isVerbose()) {cout << *bbi << endl << GLOBAL_STATE_IN(*bbi) << endl ; }
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
Identifier<global::State> GLOBAL_STATE_IN("otawa::dynbranch::GLOBAL_STATE_IN") ;


/**
 */
Identifier<global::State> GLOBAL_STATE_OUT("otawa::dynbranch::GLOBAL_STATE_OUT") ;


/**
 */
Identifier<global::State> GLOBAL_STATE_ENTRY("otawa::dynbranch::GLOBAL_STATE_ENTRY") ;


/**
 */
Identifier<bool> TIME("otawa::global::TIME") ;

} }	// otawa::dynbranch
