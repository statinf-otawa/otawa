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


#include <otawa/otawa.h>
#include <otawa/dfa/hai/WideningListener.h>
#include <otawa/dfa/hai/WideningFixPoint.h>
#include <otawa/dfa/hai/HalfAbsInt.h>
#include <otawa/dfa/FastState.h>
#include <otawa/dynbranch/features.h>
#include <time.h>
#include "PotentialValue.h"
#include "State.h"
#include "GlobalAnalysisProblem.h"
#include "GC.h"
#include "GlobalAnalysis.h"



namespace otawa { namespace dynbranch {

/**
 * @class GlobalAnalysis
 * TODO
 */

p::declare GlobalAnalysis::reg = p::init("otawa::dynbranch::GlobalAnalysis", Version(1,0,0))
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
	clock_t clockWorkSpace;
	clockWorkSpace = clock();

	const CFGCollection *coll = INVOLVED_CFGS(ws);
	ASSERT(coll);
	CFG *cfg = coll->get(0) ;

	istate = dfa::INITIAL_STATE(ws);

	sys::StopWatch watch ;

	if(PotentialValue::MAGIC == 0)
		PotentialValue::MAGIC = 0xCAFEBABE;
	else
		PotentialValue::MAGIC++;

	MyGC *myGC = new MyGC(ws); // obtain the garbage collector
	myGC->setDisableGC(true);

	PotentialValue::pvgc = myGC; // link the PotentialValue with the gc
	//PotentialValue::tempPVAlloc = new PotentialValue(); // this will make use of the GC
	PotentialValue::tempPVAlloc = 0;

	dfa::FastState<PotentialValue, MyGC> *fs = new dfa::FastState<PotentialValue, MyGC>(&pv, dfa::INITIAL_STATE(ws), *myGC);
	myGC->setFastState(fs);

	entry.setFastState(fs);

	dynbranch::GlobalAnalysisProblem* prob = new dynbranch::GlobalAnalysisProblem(ws,isVerbose(), entry, myGC);

	// adding the fundamental domains
	myGC->add(&prob->bottom());
	myGC->add(&prob->top());
	myGC->add(&prob->entry());
	myGC->setTempRegs(prob->getTempRegs());

	WideningListener<dynbranch::GlobalAnalysisProblem>* list = new WideningListener<dynbranch::GlobalAnalysisProblem>(ws, *prob);
	WideningFixPoint<WideningListener<dynbranch::GlobalAnalysisProblem> >* fp = new  WideningFixPoint<WideningListener<dynbranch::GlobalAnalysisProblem> >(*list);
	HalfAbsInt<WideningFixPoint<WideningListener<dynbranch::GlobalAnalysisProblem> > >* hai = new HalfAbsInt<WideningFixPoint<WideningListener<dynbranch::GlobalAnalysisProblem> > >(*fp, *ws);

	myGC->setListener(*list);
	myGC->setFixPoint(*fp);
	myGC->setAbsInt(*hai);

	myGC->setDisableGC(false);

	hai->solve(cfg);

	// Check the results
	for(CFGCollection::Iter cfgi(*coll); cfgi(); cfgi++) {
		for(otawa::CFG::BlockIter bbi = cfgi->blocks(); bbi(); bbi++) {
			if(!bbi->isBasic())
				continue;

			GLOBAL_STATE_IN(*bbi) = *list->results[cfgi->index()][bbi->index()];
		}
	}


	// clear the memory
	DYNBRANCH_STACK_ALLOCATOR(ws) = myGC;
	DYNBRANCH_FASTSTATE(ws) = fs;

	if (time) {
		watch.start() ;
		for (int i=0 ; i < TIME_NB_EXEC_GLOBAL ; i++) {hai->solve(cfg) ; }
		watch.stop() ;
		otawa::ot::time t = watch.delay().micros() ;
		cout << " ---------- Time stat for Global Analysis -----------" << endl  ;
		cout << " Number executions : " << TIME_NB_EXEC_GLOBAL << endl ;
		cout << " Total time (microsec): " << t << endl ;
		cout << " Average time (microsec): " << t/TIME_NB_EXEC_GLOBAL << endl ;
		cout << " ----------------------------------------------------" << endl  ;
	}

	clockWorkSpace = clock() - clockWorkSpace;

	if(DEBUG_INFO(ws))
		elm::cerr << "Global Analyse takes " << clockWorkSpace << " micro-seconds for processing " << prob->get_nb_bb_count() << " blocks" << io::endl;

	ASSERTP(PotentialValue::countX == Vector<elm::t::uint32>::countX, "PV::CountX = " << PotentialValue::countX << " vs " << Vector<elm::t::uint32>::countX);

	myGC->doGC();
	// prob->getStats();
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

//Identifier<elm::StackAllocator*> DYNBRANCH_STACK_ALLOCATOR("", 0);
Identifier<MyGC*> DYNBRANCH_STACK_ALLOCATOR("", 0);
Identifier<dfa::FastState<PotentialValue, MyGC>*>DYNBRANCH_FASTSTATE("", 0);


} }	// otawa::dynbranch
