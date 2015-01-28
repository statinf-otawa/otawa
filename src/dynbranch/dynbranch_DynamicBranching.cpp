/*
 *	DynamicBranchingAnalysis class implementation
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
#include "DynamicBranching.h"
#include "GlobalAnalysis.h"
#include <otawa/data/clp/features.h>


namespace otawa { namespace dynbranch {

/**
 * This feature try to compute for each dynamic/indirect branch the set of possible
 * targets. The result is stored on the branch instruction as a set of @ref BRANCH_TARGET
 * and @ref CALL_TARGET.
 *
 * @par Properties
 * @li @ref BRANCH_TARGET
 * @li @ref CALL_TARGET
 */
p::feature FEATURE("otawa::dynbranch::FEATURE", new Maker<DynamicBranchingAnalysis>());


/**
 */
Identifier<bool> TIME("otawa::dynbranch::TIME") ;


/**
 */
void DynamicBranchingAnalysis::configure(const PropList &props) {
	time = TIME(props) ;
	BBProcessor::configure(props) ;
}


/**
 */
static PotentialValue setFromClp(clp::Value v) {
	PotentialValue res ;
	int val = v.lower()  ;
	res.insert(val) ;
	for (int nb = v.mtimes() ; nb > 0 ; nb--) {
		val += v.delta() ;
		res.insert(val) ;
	}
	return res ;
}


/**
 */
PotentialValue DynamicBranchingAnalysis::find(MemID id,clp::State clpin , global::State globalin, Vector<sem::inst> semantics) {

	/* Stop case
	 * We have the value from the global analysis, the value from CLP (with filters), and direct access to memory
	 * Since the global analysis only provide EXACT values, we first try to get them
	 * If they aren't known by the global analysis, we ask the clp  analysis for them
	 * If neither analysis has this value, we get this from memory, and if its a register... we continue but the result is going to be 99% wrong ..
	 */
	if(semantics.length() < 1) {
		if (id.fst == MEM) {

			// Try with global analysis
			if(globalin.hasKey(id)) {
				PotentialValue val = globalin.get(id);
				PRINT_DEBUG("STOP CASE " << id << " FROM GLOBAL ANALYSIS " << val)
				return val;
			}

			// Else try with CLP value
			clp::Value reg(clp::VAL, id.snd) ;
			clp::Value valreg ;
			valreg = clpin.get(reg) ;
			PotentialValue res ;
			if (valreg.kind() != clp::ALL) {
				// This value is provided at the beginning of the BB thanks to CLP analysis
				// (its the value of x in the instruction switch (x) )
				int val = valreg.lower()  ;
				res.insert(val) ;
				for (int nb = valreg.mtimes() ; nb > 0 ; nb--) {
					val += valreg.delta() ;
					res.insert(val) ;
				}
				PRINT_DEBUG("STOP CASE " << id << " FROM CLP ANALYSIS " << res)
			} else {

				// Last case, load from memory
				unsigned long val = 0 ;
				workspace()->process()->get(id.snd,val) ;
				res.insert(val) ;
				PRINT_DEBUG("STOP CASE " << id << " FROM LOADING " << res)
			}
			return res ;

		} else {

			// Try with global analysis
			if(globalin.hasKey(id)) {
				PotentialValue val = globalin.get(id);
				PRINT_DEBUG("STOP CASE " << id << " FROM GLOBAL ANALYSIS " << val)
				return val;
			}

			// CLP Analysis
			clp::Value reg(clp::REG, id.snd) ;
			// Get this value
			clp::Value valreg ;
			valreg = clpin.get(reg) ;

			// We need to know this value, if we don't, we can't analyze anything
			if (valreg.kind() != clp::VAL) {
				if (isVerbose()) { cout << " Warning : we need a value from the analysis which is not here, be careful with results.. " << endl ; }
				return PotentialValue() ;
			}

			// Construct the set with all possible values
			return setFromClp(valreg) ;
		}

	} else {

		sem::inst i = semantics.top();
		semantics.pop() ;

		if (id.fst == MEM) { // MEMb(a) <- d

			if (i.op == sem::STORE ) {
				MemID rega = elm::Pair<Memtype,int>(REG,i.a()) ;
				PotentialValue potentialAddr = find(rega,clpin,globalin,semantics) ;

				if ( potentialAddr.length() > 0 && potentialAddr.get(0) == id.snd) {
					MemID valid = elm::Pair<Memtype,int>(REG,i.d()) ;
					PRINT_DEBUG("==START== STORE REG " << i.d() << " IN MEMORY AT ADRESS IN REG " << i.a()  << " STORING " << i.b() << "BYTES")
					PotentialValue r = find(valid,clpin,globalin,semantics) ;
					PRINT_DEBUG("==END== STORE REG " << i.d() << " IN MEMORY AT ADRESS IN REG " << i.a()  << " STORING " << i.b() << "BYTES")
					return r ;
				}
			} // store
			return find(id,clpin,globalin,semantics) ;

		} else { // Register


			// Ignore inst with doesnt concern our seek register
			if (i.d() != id.snd) {
				return find(id,clpin,globalin,semantics) ;
			}
			PotentialValue v ;
			switch(i.op) {
			case sem::NEG:	// d <- -a
			case sem::NOT:		// d <- ~a
			case sem::MUL:		// d <- a * b
			case sem::MULU:		// d <- unsigned(a) * unsigned(b)
			case sem::DIV:		// d <- a / b
			case sem::DIVU:		// d <- unsigned(a) / unsigned(b)
			case sem::MOD:		// d <- a % b
			case sem::MODU:		// d <- unsigned(a) % unsigned(b)
			case sem::NOP :
			case sem::TRAP:   // perform a trap
			case sem::CONT:   // continue in sequence with next instruction
			case sem::IF:  // continue if condition a is meet in register b, else jump c instructions
			case sem::CMP:    // d <- a ~ b
			case sem::ASR:    // d <- a +>> b
			case sem::CMPU:   // d <- a ~u b
			case sem::STORE:    // MEMb(a) <- d
			case sem::AND:    // MEMb(a) <- d
			case sem::SETP:   // page(d) <- cst
				return find(id,clpin,globalin,semantics) ;

			case sem::SCRATCH:  // d <- T
			case sem::BRANCH:   // perform a branch on content of register a
				ASSERT(false) ; // No branch in a BB
				break;

			case sem::LOAD:   // d <- MEMb(a)
			{
				PRINT_DEBUG("==START== SET REG " << i.d() << " FROM MEMOIRE AT ADRESS IN REG " << i.a() << " LOADING " << i.b() <<  " BYTES")
            		PotentialValue r ;
				MemID rega = elm::Pair<Memtype,int>(REG,i.a()) ;
				PotentialValue toget = find(rega,clpin,globalin,semantics) ;
				// Get the value in memory from each possible addresses
				for ( PotentialValue::Iterator it(toget) ; it ; it++) {
					// cout << "==START== SEARCHING MEMORY VALUE AT ADDR " << hex(*it) << endl ;
					int val = *it ;
					MemID memid = elm::Pair<Memtype,int>(MEM,val) ;
					PotentialValue p  = find(memid,clpin,globalin,semantics) ;
					r = merge(r, p) ;
					// cout << "==END== SEARCHING MEMORY VALUE AT ADDR " << hex(*it) << endl ;
				}
				PRINT_DEBUG("==END== SET REG " << i.d() << " FROM MEMOIRE AT ADRESS IN REG " << i.a() << " LOADING " << i.b() <<  " BYTES")
				return r ;
			}
			case sem::SET:    // d <- a
			{
				MemID rega = elm::Pair<Memtype,int>(REG,i.a()) ;
				PRINT_DEBUG("==START== SET REG " << i.d() << " FROM REG " << i.a())
				v = find(rega,clpin,globalin,semantics) ;
				PRINT_DEBUG("==END== SET REG " << i.d() << " FROM REG " << i.a())
				return v ;
			}
			case sem::SETI:   // d <- cst
				PRINT_DEBUG(" SET REG " << i.d() << " FROM CST " << i.cst())
				v.insert(i.cst()) ;
				return v ;

			case sem::OR:
			{
				MemID rega = elm::Pair<Memtype,int>(REG,i.a()) ;
				MemID regb = elm::Pair<Memtype,int>(REG,i.b()) ;
				PRINT_DEBUG("==START== SET REG " << i.d() << " FROM OR OF REG " << i.a() << " AND REG " << i.b())
				v = find(rega,clpin,globalin,semantics) || find(regb,clpin,globalin,semantics) ;
				PRINT_DEBUG("==END== SET REG " << i.d() << " FROM OR OF REG " << i.a() << " AND REG " << i.b())
				return v ;
			}
			case sem::ADD:    // d <- a + b
			{
				MemID rega = elm::Pair<Memtype,int>(REG,i.a()) ;
				MemID regb = elm::Pair<Memtype,int>(REG,i.b()) ;
				PRINT_DEBUG("==START== SET REG " << i.d() << " FROM ADDITION OF REG " << i.a() << " AND REG " << i.b())
				v = find(rega,clpin,globalin,semantics) + find(regb,clpin,globalin,semantics) ;
				PRINT_DEBUG("==END== SET REG " << i.d() << " FROM ADDITION OF REG " << i.a() << " AND REG " << i.b())
				return v ;
			}
			case sem::SUB:    // d <- a - b
			{
				MemID rega = elm::Pair<Memtype,int>(REG,i.a()) ;
				MemID regb = elm::Pair<Memtype,int>(REG,i.b()) ;
				PRINT_DEBUG("==START== SET REG " << i.d() << " FROM SUB OF REG " << i.a() << " AND REG " << i.b())
				v = find(rega,clpin,globalin,semantics) - find(regb,clpin,globalin,semantics) ;
				PRINT_DEBUG("==END== SET REG " << i.d() << " FROM SUB OF REG " << i.a() << " AND REG " << i.b())
				return v ;
			}
			case sem::SHL:    // d <- a << b
			{
				MemID rega = elm::Pair<Memtype,int>(REG,i.a()) ;
				MemID regb = elm::Pair<Memtype,int>(REG,i.b()) ;
				PRINT_DEBUG("==START== SET REG " << i.d() << " FROM SHL OF REG " << i.a() << " AND REG " << i.b())
				v = find(rega,clpin,globalin,semantics) << find(regb,clpin,globalin,semantics) ;
				PRINT_DEBUG("==END== SET REG " << i.d() << " FROM SHL OF REG " << i.a() << " AND REG " << i.b())
				return v ;
			}
			case sem::SHR:    // d <- a >> b
			{
				MemID rega = elm::Pair<Memtype,int>(REG,i.a()) ;
				MemID regb = elm::Pair<Memtype,int>(REG,i.b()) ;
				PRINT_DEBUG("==START== SET REG " << i.d() << " FROM SHR OF REG " << i.a() << " AND REG " << i.b())
				v = find(rega,clpin,globalin,semantics) >> find(regb,clpin,globalin,semantics) ;
				PRINT_DEBUG("==END== SET REG " << i.d() << " FROM SHR OF REG " << i.a() << " AND REG " << i.b())
				return v ;
			}

			default:
				// Unknown semantic instruction
				break ;
			}
		}
	}
	return PotentialValue();
}


/**
 */
void DynamicBranchingAnalysis::addTargetToBB(BasicBlock* bb) {
	Vector<sem::inst> semantics ;
	for(BasicBlock::InstIter inst(bb) ; inst ; inst++) {
		PRINT_DEBUG(*inst)
    		sem::Block block ;
		inst->semInsts(block) ;
		for(sem::Block::InstIter semi(block) ; semi ; semi++) {
			PRINT_DEBUG("\t" << *semi)
    		  semantics.push(*semi) ;
		}
	}
	sem::inst i = semantics.top();
	semantics.pop() ;

	if (i.op != sem::BRANCH) {
		if (isVerbose()) { cout << "\t\tThis BasicBlock doesn't end with a branch, we can't do anything" << endl ; }
		return ;
	}

	MemID branchID = elm::Pair<Memtype,int>(REG,i.d()) ;

	PotentialValue addresses = find(branchID, clp::STATE_IN(bb), GLOBAL_STATE_IN(bb), semantics) ;

	if (addresses.length() < 1 ) {
		if (isVerbose()) { cout << "\t\tNo branch addresses found" << endl ; }
		return ;
	} else {
		for ( Set<int>::Iterator it(addresses) ; it ; it++) {
			if (isVerbose()) { cout << "\t\t Addresses de branchements possibles (en hexa) : " << hex(*it) << endl ; }
		}
	}
}


p::declare DynamicBranchingAnalysis::reg = p::init("DynamicBranchingAnalysis", Version(1, 0, 0))
	.require(GLOBAL_ANALYSIS_FEATURE)
	.require(clp::FEATURE)
	.provide(FEATURE) ;


/**
 */
DynamicBranchingAnalysis::DynamicBranchingAnalysis(p::declare& r): BBProcessor(r), isDebug(false), time(false) {
}


/**
 */
void DynamicBranchingAnalysis::processBB(WorkSpace *ws, CFG *cfg, BasicBlock *bb) {
	isDebug = DEBUG && isVerbose() ; // need to be verbose to be debug
	if (bb->isTargetUnknown()) {
		PRINT_DEBUG(clp::STATE_IN(bb) << endl << " Global State" << endl <<  GLOBAL_STATE_IN(bb))
    		if (time) {
    			isDebug = false ;
    			system::StopWatch watch ;
    			watch.start() ;
    			for (int i=0 ; i < NB_EXEC ; i++) {addTargetToBB(bb) ; }
    			watch.stop() ;
    			otawa::ot::time t = watch.delay() ;
    			cout << " ---------- Time stat for " << bb << "------------" << endl  ;
    			cout << " Number of instructions in this BB : " << bb->countInstructions() << endl ;
    			cout << " Number executions : " << NB_EXEC << endl ;
    			cout << " Total time (microsec): " << t << endl ;
    			cout << " Average time (microsec): " << t/NB_EXEC << endl ;
    			cout << " ---------- End Time stat for " << bb << "------------" << endl  ;
    		} else {
    			addTargetToBB(bb) ;
    		}
	} else {
		if (isVerbose()) {
			cout << "\t\tNothing to do for this BB (no dynamic branching detected)" << endl ;
		}
	}
}

} }
