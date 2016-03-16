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

namespace otawa {
namespace dynbranch {

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
	propss = props;
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
PotentialValue DynamicBranchingAnalysis::find(BasicBlock* bb, MemID id, const clp::State & clpin, global::State & globalin, Vector<sem::inst> semantics) {
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
			if(otawa::dynbranch::inROData(id.snd, workspace())) {
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
				assert(0); // just to see what brings us here
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
				case sem::NEG:	// d <- -a
				case sem::NOT:// d <- ~a
				case sem::MUL:// d <- a * b
				case sem::MULU:// d <- unsigned(a) * unsigned(b)
				case sem::DIV:// d <- a / b
				case sem::DIVU:// d <- unsigned(a) / unsigned(b)
				case sem::MOD:// d <- a % b
				case sem::MODU:// d <- unsigned(a) % unsigned(b)
				case sem::NOP :
				case sem::TRAP:// perform a trap
				case sem::CONT:// continue in sequence with next instruction
				case sem::IF:// continue if condition a is meet in register b, else jump c instructions
				case sem::CMP:// d <- a ~ b
				case sem::ASR:// d <- a +>> b
				case sem::CMPU:// d <- a ~u b
				case sem::STORE:// MEMb(a) <- d
				case sem::AND:// MEMb(a) <- d
				case sem::SETP:// page(d) <- cst
				assert(0);// want to know, these are ignored instructions......!
				return find(bb,id,clpin,globalin,semantics);

				case sem::SCRATCH:// d <- T
				case sem::BRANCH:// perform a branch on content of register a
				ASSERT(false);// No branch in a BB
				break;

				case sem::LOAD:// d <- MEMb(a)
				{
					PotentialValue r;
					MemID rega = elm::Pair<Memtype,int>(REG,i.a());
					PotentialValue toget = find(bb,rega,clpin,globalin,semantics);
					// Get the value in memory from each possible addresses

					for ( PotentialValue::Iterator it(toget); it; it++) {
						if(otawa::dynbranch::inROData(*it, workspace())) {
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
						if(toget.length() > 0) {
							elm::cout << "toget[0] = " << hex(toget[0]) << io::endl;
							clp::Value x = clpState[semantics.length()-1].get(clp::Value(clp::VAL, toget[0], 0,0));
							elm::cout << "CLP result = " << x << io::endl;
							assert(0); // future work, see what it brings to us here
						}
					}
					writeReg(i.d(), r);
					return r;
				}
				case sem::SET:    // d <- a
				{
					MemID rega = elm::Pair<Memtype,int>(REG,i.a());
					v = find(bb,rega,clpin,globalin,semantics);
					writeReg(i.d(), v);
					return v;
				}
				case sem::SETI:   // d <- cst
				v.insert(i.cst());
				writeReg(i.d(), v);
				return v;

				case sem::OR:
				{
					MemID rega = elm::Pair<Memtype,int>(REG,i.a());
					MemID regb = elm::Pair<Memtype,int>(REG,i.b());
					v = find(bb,rega,clpin,globalin, semantics) || find(bb,regb,clpin,globalin,semantics);
					writeReg(i.d(), v);
					return v;
				}
				case sem::ADD:    // d <- a + b
				{
					MemID rega = elm::Pair<Memtype,int>(REG,i.a());
					MemID regb = elm::Pair<Memtype,int>(REG,i.b());
					v = find(bb,rega,clpin,globalin, semantics) + find(bb,regb,clpin,globalin,semantics);
					writeReg(i.d(), v);
					return v;
				}
				case sem::SUB:    // d <- a - b
				{
					MemID rega = elm::Pair<Memtype,int>(REG,i.a());
					MemID regb = elm::Pair<Memtype,int>(REG,i.b());
					v = find(bb,rega,clpin,globalin, semantics) - find(bb,regb,clpin,globalin,semantics);
					writeReg(i.d(), v);
					return v;
				}
				case sem::SHL:    // d <- a << b
				{
					MemID rega = elm::Pair<Memtype,int>(REG,i.a());
					MemID regb = elm::Pair<Memtype,int>(REG,i.b());
					PotentialValue va = find(bb,rega,clpin,globalin, semantics);
					PotentialValue vb = find(bb,regb,clpin,globalin,semantics);
					v =  va << vb;
					writeReg(i.d(), v);
					return v;
				}
				case sem::SHR:    // d <- a >> b
				{
					MemID rega = elm::Pair<Memtype,int>(REG,i.a());
					MemID regb = elm::Pair<Memtype,int>(REG,i.b());
					v = find(bb,rega,clpin,globalin, semantics) >> find(bb,regb,clpin,globalin,semantics);
					writeReg(i.d(), v);
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
											.provide(FEATURE);

/**
 */
DynamicBranchingAnalysis::DynamicBranchingAnalysis(p::declare& r)
		: BBProcessor(r), isDebug(false), time(false), clpManager(0), clpState() {
}

/**
 */
void DynamicBranchingAnalysis::processBB(WorkSpace *ws, CFG *cfg, Block *b) {

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
			DYNBRANCH_TARGET_COUNT(bb->last()) = 0;
			break;
		}
	}

	// even though the current basic contains dynamic branching target
	// but when the number of target stays unchanged from the last analysis, that means the fixed point has been reached, and no need to check further
	int currentDynBranchTargetCount = DYNBRANCH_TARGET_COUNT(bb->last());
	int previousDynBranchTargetCount = DYNBRANCH_TARGET_COUNT_PREV(bb->last());

	if(currentDynBranchTargetCount != previousDynBranchTargetCount) {
		DYNBRANCH_TARGET_COUNT_PREV(bb->last()) = currentDynBranchTargetCount;

		// collecting all the CLP state for each semantic instruction in the block, maybe not necessary
		clp::Manager::step_t s = clpManager->start(bb);
		clpState.clear();
		while(s) {
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
}

} // end namespace dynbranch
} // end namespace otawa
