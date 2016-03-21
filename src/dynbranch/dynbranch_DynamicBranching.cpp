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
#include <otawa/data/clp/SymbolicExpr.h>
#include <elm/genstruct/quicksort.h>
#include <otawa/util/FlowFactLoader.h>
#include <otawa/display/CFGOutput.h>
#include <otawa/dynbranch/features.h>
#include <otawa/data/clp/SymbolicExpr.h> // to use the filters
#include <elm/log/Log.h> // to use the debugging messages

#define DEBUG_FILTERS(x) // x
#define DEBUG_CLP(x) // x

using namespace elm::log;
using namespace elm::color;
namespace otawa { namespace dynbranch {

static Identifier<Vector<Address> > POSSIBLE_BRANCH_TARGETS("otawa::dynbranch::POSSIBLE_BRANCH_TARGETS"); // only in this file, for the set of target addresses
Identifier<bool> NEW_BRANCH_TARGET_FOUND("otawa::dynbranch::NEW_BRANCH_TARGET_FOUND", false); // on workspace
Identifier<int> DYNBRANCH_TARGET_COUNT("otawa::dynbranch::DYNBRANCH_TARGET_COUNT", -1); // on last instruction of the BB
Identifier<int> DYNBRANCH_TARGET_COUNT_PREV("otawa::dynbranch::DYNBRANCH_TARGET_COUNT_PREV", -1); // on last instruction of the BB, previous


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
//Identifier<bool> TIME("otawa::dynbranch::TIME") ;
extern Identifier<bool> TIME;

/**
 */
void DynamicBranchingAnalysis::configure(const PropList &props) {
	time = TIME(props);
	BBProcessor::configure(props);
}

/**
 */
static PotentialValue setFromClp(clp::Value v) {
	PotentialValue res;
	int val = v.lower();
	res.insert(val);
	for(unsigned int nb = v.mtimes(); nb > 0; nb--) {
		val += v.delta();
		res.insert(val);
	}
	return res;
}

/**
 * Note that the vector of semantic instruction is "copied" for each find calls.
 */
PotentialValue DynamicBranchingAnalysis::find(BasicBlock* bb, MemID id, const clp::State & clpin, State & globalin, Vector<sem::inst> semantics) {
	// TODO: instead copy the whole vector of semantic, use the index ?

	/* Stop case
	 * We have the value from the global analysis, the value from CLP (with filters), and direct access to memory
	 * Since the global analysis only provide EXACT values, we first try to get them
	 * If they aren't known by the global analysis, we ask the clp  analysis for them
	 * If neither analysis has this value, we get this from memory, and if its a register... we continue but the result is going to be 99% wrong ..
	 */

	if(semantics.length() < 1) { // no other sem inst
		if (id.fst == MEM) {
			// Priority of memory reading: RO MEM -> Global Analysis -> CLP
			// the value is available in the READ ONLY REGION, then read from it
			if(istate && istate->isInitialized(id.snd)) {
				PotentialValue r;
				unsigned long val = 0;
				workspace()->process()->get(id.snd,val);
				r.insert(val);
				return r;
			}

			// From Global Analysis
			PotentialValue val = globalin.loadMemory(id.snd);
			if(val.length() > 0) {
				return val;
			}

			// Else try with CLP value
			clp::Value reg(clp::VAL, id.snd);
			clp::Value valreg;
			valreg = clpin.get(reg);
			PotentialValue res;
			if (valreg.kind() != clp::ALL) {
				// This value is provided at the beginning of the BB thanks to CLP analysis
				// (its the value of x in the instruction switch (x) )
				int val = valreg.lower();
				res.insert(val);
				for (int nb = valreg.mtimes(); nb > 0; nb--) {
					val += valreg.delta();
					res.insert(val);
				}
			} else {
				res = PotentialValue::bot;
			}
			return res;

		} else { // register

			// Try to read the register value from the Global Analysis first
			const PotentialValue& pv = globalin.readReg(id.snd);
			if(pv.length() >0) {
				return pv;
			}

			// Otherwise, try the CLP Analysis
			clp::Value reg(clp::REG, id.snd);
			// Get this value
			clp::Value valreg;
			valreg = clpin.get(reg);
			// We need to know this value, if we don't, we can't analyze anything
			if (valreg.kind() != clp::VAL) {
				if (isVerbose()) {cout << " Warning : we need a value from the analysis which is not here, be careful with results.. " << endl;}
				// We will fall back to the first place (the first find call which obtain the { } ) and try the clp there.
				return PotentialValue();
			}
			return setFromClp(valreg);
		}
	} else { // WHEN SEM INST COUNT > 1

		sem::inst i = semantics.top();
		semantics.pop();

		// when the Potential Value to trace is for a memory, we only care about STORE to memory
		if (id.fst == MEM) { // MEMb(a) <- d

			if (i.op == sem::STORE ) {
				MemID rega = elm::Pair<Memtype,int>(REG,i.a());
				PotentialValue potentialAddr = find(bb,rega,clpin,globalin,semantics);

				if ( potentialAddr.length() > 0 && potentialAddr.get(0) == id.snd) {
					MemID valid = elm::Pair<Memtype,int>(REG,i.d());
					PotentialValue r = find(bb,valid,clpin,globalin,semantics);
					return r;
				}
			} // end if (i.op == sem::STORE )

			// ignore all the other semantic instructions
			return find(bb,id,clpin,globalin,semantics);

		} else { // Register
			// if register value is available, then use

			// Ignore inst with doesnt concern our seek register
			if (i.d() != id.snd) {
				// do nothing to the current sem, we continue the processing to the next sem inst
				return find(bb,id,clpin,globalin,semantics);
			}

			PotentialValue v;
			switch(i.op) {
				case sem::NEG:		// d <- -a
				case sem::NOT:		// d <- ~a
				case sem::MUL:		// d <- a * b
				case sem::MULU:		// d <- unsigned(a) * unsigned(b)
				case sem::DIV:		// d <- a / b
				case sem::DIVU:		// d <- unsigned(a) / unsigned(b)
				case sem::MOD:		// d <- a % b
				case sem::MODU:		// d <- unsigned(a) % unsigned(b)
				case sem::NOP:
				case sem::TRAP:		// perform a trap
				case sem::CONT:		// continue in sequence with next instruction
				case sem::IF:		// continue if condition a is meet in register b, else jump c instructions
				case sem::CMP:		// d <- a ~ b
				case sem::CMPU:		// d <- a ~u b
				case sem::STORE:	// MEMb(a) <- d
				case sem::SETP:		// page(d) <- cst
				elm::cout << elm::log::Debug::debugPrefix(__FILE__, __LINE__,__FUNCTION__) << "fail to process sem inst: " << i << io::endl;
				assert(0);			// want to know, these are ignored instructions......!
				return find(bb,id,clpin,globalin,semantics);

				case sem::SCRATCH:	// d <- T
				case sem::BRANCH:	// perform a branch on content of register a
				ASSERT(false);		// No branch in a BB
				break;

				case sem::LOAD:// d <- MEMb(a)
				{
					PotentialValue r;
					MemID rega = elm::Pair<Memtype,int>(REG,i.a());
					PotentialValue toget = find(bb,rega,clpin,globalin,semantics);

					// Get the value in memory from each possible addresses
					for(PotentialValue::Iterator it(toget); it; it++) {
						if(istate && istate->isInitialized(*it)) {
							unsigned int val = 0;
							workspace()->process()->get(*it,val);
							r.insert(val);
						}
						else {
							int val = *it;
							MemID memid = elm::Pair<Memtype,int>(MEM,val);
							PotentialValue p = find(bb,memid,clpin,globalin,semantics);
							r = merge(r, p);
						}
					}

					if(r == PotentialValue::bot) { // means couldn't be found from the memory, lets try CLP?
						for(PotentialValue::Iterator it(toget); it; it++) {
							clp::Value valueFromCLP = clpState[semantics.length()-1].get(clp::Value(clp::VAL, *it, 0,0));
							if(valueFromCLP != clp::Value::top) {
								PotentialValue p = setFromClp(valueFromCLP);
								r = merge(r, p);
							}
						}
						
						if(toget.length() == 0) { // if there is no address to load, we look the value of the register in CLP state directly
							clp::Value x = clpState[semantics.length()-1].get(clp::Value(clp::REG, i.d(), 0,0));
							r = setFromClp(x);
						}

						DEBUG_CLP(if(toget.length() > 0) elm::cout << Debug::debugPrefix(__FILE__, __LINE__,__FUNCTION__) << "CLP result = " << r << io::endl;)
					}
					return r;
				}
				case sem::SET:    // d <- a
				{
					MemID rega = elm::Pair<Memtype,int>(REG,i.a());
					v = find(bb,rega,clpin,globalin,semantics);
					return v;
				}
				case sem::SETI:   // d <- cst
				v.insert(i.cst());
				return v;

				case sem::OR:
				{
					MemID rega = elm::Pair<Memtype,int>(REG,i.a());
					MemID regb = elm::Pair<Memtype,int>(REG,i.b());
					v = find(bb,rega,clpin,globalin, semantics) || find(bb,regb,clpin,globalin,semantics);
					return v;
				}
				case sem::AND:    // d <- a & b
				{
					MemID rega = elm::Pair<Memtype,int>(REG,i.a());
					MemID regb = elm::Pair<Memtype,int>(REG,i.b());
					PotentialValue va = find(bb,rega,clpin,globalin, semantics);
					takingCLPValueIfNecessary(va, semantics.length(), clp::Value(clp::REG,i.a()));
					PotentialValue vb = find(bb,regb,clpin,globalin,semantics);
					takingCLPValueIfNecessary(vb, semantics.length(), clp::Value(clp::REG,i.b()));
					v =  va & vb;
					return v;
				}
				case sem::ADD:    // d <- a + b
				{
					MemID rega = elm::Pair<Memtype,int>(REG,i.a());
					MemID regb = elm::Pair<Memtype,int>(REG,i.b());
					v = find(bb,rega,clpin,globalin, semantics) + find(bb,regb,clpin,globalin,semantics);
					return v;
				}
				case sem::SUB:    // d <- a - b
				{
					MemID rega = elm::Pair<Memtype,int>(REG,i.a());
					MemID regb = elm::Pair<Memtype,int>(REG,i.b());
					v = find(bb,rega,clpin,globalin, semantics) - find(bb,regb,clpin,globalin,semantics);
					return v;
				}
				case sem::SHL:    // d <- a << b
				{
					MemID rega = elm::Pair<Memtype,int>(REG,i.a());
					MemID regb = elm::Pair<Memtype,int>(REG,i.b());
					PotentialValue va = find(bb,rega,clpin,globalin, semantics);
					takingCLPValueIfNecessary(va, semantics.length(), clp::Value(clp::REG,i.a()));
					PotentialValue vb = find(bb,regb,clpin,globalin,semantics);
					takingCLPValueIfNecessary(vb, semantics.length(), clp::Value(clp::REG,i.b()));
					v =  va << vb;
					return v;
				}
				case sem::SHR:    // d <- a +>> b
				{
					MemID rega = elm::Pair<Memtype,int>(REG,i.a());
					MemID regb = elm::Pair<Memtype,int>(REG,i.b());
					PotentialValue va = find(bb,rega,clpin,globalin, semantics);
					takingCLPValueIfNecessary(va, semantics.length(), clp::Value(clp::REG,i.a()));
					PotentialValue vb = find(bb,regb,clpin,globalin,semantics);
					takingCLPValueIfNecessary(vb, semantics.length(), clp::Value(clp::REG,i.b()));
					v =  logicalShiftRight(va, vb);
					return v;
				}
				case sem::ASR:    // d <- a >> b
				{
					MemID rega = elm::Pair<Memtype,int>(REG,i.a());
					MemID regb = elm::Pair<Memtype,int>(REG,i.b());
					PotentialValue va = find(bb,rega,clpin,globalin, semantics);
					takingCLPValueIfNecessary(va, semantics.length(), clp::Value(clp::REG,i.a()));
					PotentialValue vb = find(bb,regb,clpin,globalin,semantics);
					takingCLPValueIfNecessary(vb, semantics.length(), clp::Value(clp::REG,i.b()));
					v =  va >> vb;
					return v;
				}

				default:
				// Unknown semantic instruction
				break;
			}
		}
	}
	return PotentialValue();
}

/*
 * If the find() method returns empty PotentialValue, we are looking into the CLP state to see if we can obtain the value
 */
void DynamicBranchingAnalysis::takingCLPValueIfNecessary(PotentialValue& pv, int semanticInstIndex, const clp::Value& regOrAddr) {
	if(pv.length() == 0) { // if there is no value for the potential value
		clp::Value clpva = clpState[semanticInstIndex].get(regOrAddr); // obtain the value from the CLP state
		if(clpva != clp::Value::all) { // if the CLP value is not Top
			pv = setFromClp(clpva); // convert to PotentialValue
		}
	}
}

/**
 */
void DynamicBranchingAnalysis::addTargetToBB(BasicBlock* bb) {
	Vector<sem::inst> semantics;
	for(BasicBlock::InstIter inst(bb); inst; inst++) {
		sem::Block block;
		inst->semInsts(block);
		for(sem::Block::InstIter semi(block); semi; semi++)
			semantics.push(*semi);
	}

	sem::inst i = semantics.top();
	semantics.pop();

	// not a branch, ignored
	if(i.op != sem::BRANCH)
		return;

	Domain globalStateBB = GLOBAL_STATE_IN(bb);

	// start looking for the possible branching address
	MemID branchID = elm::Pair<Memtype, int>(REG, i.d());
	PotentialValue addresses = find(bb, branchID, clp::STATE_IN(bb), globalStateBB, semantics);

	// when there is no addresses found
	if(addresses.length() < 1) {
		if(isVerbose()) {
			cout << "\t\tNo branch addresses found" << endl;
		}
		return;
	} else {
		for(Set<elm::t::uint32>::Iterator it(addresses); it; it++) {
			if(isVerbose()) {
				cout << "\t\t Possible branching addresses : 0x" << hex(*it) << endl;
			}
		}
	}

	for(PotentialValue::Iterator pvi(addresses); pvi; pvi++) {
		Address targetAddr = Address(*pvi);

		bool targetToAdd = true;
		Inst* last = bb->last();

		// Check if the found target has already been recorded for BRANCH TARGET
		for(Identifier<Address>::Getter target(last, BRANCH_TARGET); target; target++) {
			if(*target == targetAddr) {
				targetToAdd = false;
				break;
			}
		}

		// Check if the found target has already been recorded for CALL TARGET
		for(Identifier<Address>::Getter target(last, CALL_TARGET); target; target++) {
			if(*target == targetAddr) {
				targetToAdd = false;
				break;
			}
		}

		// when there are some targets for the BB
		if(targetToAdd) {
			// check the type: Branch or Call
			if(last->isBranch())
				BRANCH_TARGET(last).add(targetAddr);
			else
				CALL_TARGET(last).add(targetAddr);
			// set NEW_BRANCH_TARGET_FOUND to true so notified there is a new target addresses detected
			NEW_BRANCH_TARGET_FOUND(workspace()) = true;
			// increment the target count
			int a = DYNBRANCH_TARGET_COUNT(bb->last());
			a++;
			DYNBRANCH_TARGET_COUNT(bb->last()) = a;
		}
	}
}

p::declare DynamicBranchingAnalysis::reg = p::init("DynamicBranchingAnalysis", Version(1, 0, 0))
											.require(clp::FEATURE)
											.require(GLOBAL_ANALYSIS_FEATURE)
											.require(dfa::INITIAL_STATE_FEATURE)
											.provide(FEATURE);

/**
 */
DynamicBranchingAnalysis::DynamicBranchingAnalysis(p::declare& r)
		: BBProcessor(r), isDebug(false), time(false), clpManager(0), clpState() {
}

/*
 *
 */
DynamicBranchingAnalysis:: ~DynamicBranchingAnalysis(void) {
	if(clpManager) {
		delete clpManager;
		clpManager = 0;
	}
}

/**
 */
void DynamicBranchingAnalysis::processBB(WorkSpace *ws, CFG *cfg, Block *b) {

	istate = dfa::INITIAL_STATE(ws);

	// only creates the clp manager once (performance)
	if(!clpManager)
		clpManager = new clp::Manager(ws);

	_workspace = ws;
	isDebug = DEBUGDYN && isVerbose(); // need to be verbose to be debug

	if(!b->isBasic())
		return;

	BasicBlock *bb = b->toBasic();

	// if the BB has unknown target, that means the BB has not been through the analysis
	// initialize the target count to 0 (from -1)
	for(Block::EdgeIter ei = b->outs(); ei; ei++) {
		if(ei->sink()->isUnknown()) {
			DYNBRANCH_TARGET_COUNT(bb->last()) = 0; // from -1
			break;
		}
		if(ei->sink()->isSynth() && !ei->sink()->toSynth()->callee()) { // for calls without targets
			DYNBRANCH_TARGET_COUNT(bb->last()) = 0; // from -1
			break;
		}
	}

	// even though the current basic contains dynamic branching target
	// but when the number of target stays unchanged from the last analysis, that means the fixed point has been reached, and no need to check further
	int currentDynBranchTargetCount = DYNBRANCH_TARGET_COUNT(bb->last());
	int previousDynBranchTargetCount = DYNBRANCH_TARGET_COUNT_PREV(bb->last());

	bool cond1 = (currentDynBranchTargetCount != previousDynBranchTargetCount); // when there is newly found target address
	bool cond2 = (currentDynBranchTargetCount == previousDynBranchTargetCount) && (currentDynBranchTargetCount == 0); // when unknwon is identified but target is not found yet

	if(cond1 || cond2) {
		DYNBRANCH_TARGET_COUNT_PREV(bb->last()) = currentDynBranchTargetCount;

		// collecting all the CLP state for each semantic instruction in the block, maybe not necessary
		clp::Manager::step_t s = clpManager->start(bb);

		// obtain the filters (if there is any) in this BB.
		// FIXME: assumption: ONLY ONE IF per BB!
		Vector<se::SECmp *> regFilters = se::REG_FILTERS(bb);
		Vector<se::SECmp *> addrFilters = se::ADDR_FILTERS(bb);

		clpState.clear();

		int semInstIndex = 0;
		Inst* inst = 0;
		while(s) {
			// check if the current semantic instruction is an IF
			sem::inst si = clpManager->sem();

			DEBUG_FILTERS(
			if(inst != clpManager->inst()) {
				inst = clpManager->inst();
				elm::cout << Debug::debugPrefix(__FILE__, __LINE__,__FUNCTION__) << inst << io::endl;
			}
			)
			DEBUG_FILTERS(elm::cout << Debug::debugPrefix(__FILE__, __LINE__,__FUNCTION__) << "    " << si << io::endl;)
			DEBUG_FILTERS(elm::cout << Debug::debugPrefix(__FILE__, __LINE__,__FUNCTION__) << "        [" << semInstIndex++ << "] "; (clpManager->state()->print(elm::cout)); elm::cout << io::endl;)

			if(si.op == sem::IF) {
				for (int i=0; i < regFilters.length(); i++) { // now look into register filters
					se::SECmp *filter = regFilters[i];
					// we only take care of the comparisons...
					if(filter->op() < se::LE)
						continue;

					DEBUG_FILTERS(elm::cout << Debug::debugPrefix(__FILE__, __LINE__,__FUNCTION__) << "        " << "Applying filter " << filter->asString() << io::endl;)
					clp::Value rval = filter->a()->val();
					clp::Value r = clp::Value(clp::REG, rval.lower(), 0, 0);
					clp::Value v = clpManager->state()->get(r);
					applyFilter(v, filter->op(), filter->b()->val());
					clpManager->state()->set(r, v);
				} // end of register filtering
				for (int i=0; i < addrFilters.length(); i++) { // now look into memory filters
					se::SECmp *filter = addrFilters[i];
					// we only take care of the comparisons...
					if(filter->op() < se::LE)
						continue;

					DEBUG_FILTERS(elm::cout << Debug::debugPrefix(__FILE__, __LINE__,__FUNCTION__) << "        " << "Applying filter " << filter->asString() << io::endl;)
					clp::Value rval = filter->a()->val();
					clp::Value v = clpManager->state()->get(rval);
					applyFilter(v, filter->op(), filter->b()->val());
					clpManager->state()->set(rval, v);
				} // end of register filtering
			} // end of IF (possible filtering)

			clpState.push(*clpManager->state());
			s = clpManager->next();
		}

		if (time) {
			isDebug = false;
			system::StopWatch watch;
			watch.start();
			for (int i=0; i < NB_EXEC; i++) {addTargetToBB(bb);}
			watch.stop();
			otawa::ot::time t = watch.delay();
			cout << " ---------- Time stat for " << bb << "------------" << endl;
			cout << " Number of instructions in this BB : " << bb->count() << endl;
			cout << " Number executions : " << NB_EXEC << endl;
			cout << " Total time (microsec): " << t << endl;
			cout << " Average time (microsec): " << t/NB_EXEC << endl;
			cout << " ---------- End Time stat for " << bb << "------------" << endl;
		} else {
			addTargetToBB(bb);
		}
	} else {
		if (isVerbose()) {
			cout << "\t\tNothing to do for this BB (no dynamic branching detected)" << endl;
		}
	}

	DYNBRANCH_POTENTIAL_VALUE_LIST(workspace()) = &PotentialValue::potentialValueCollector;
}

} // end namespace dynbranch
} // end namespace otawa
