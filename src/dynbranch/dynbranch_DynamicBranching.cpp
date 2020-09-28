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

#include "State.h"
#include "GlobalAnalysis.h"

#include <otawa/data/clp/features.h>
#include <otawa/data/clp/SymbolicExpr.h>
#include <elm/data/quicksort.h>
#include <otawa/display/CFGOutput.h>
#include <otawa/dynbranch/features.h>
#include <otawa/data/clp/SymbolicExpr.h> // to use the filters
#include <elm/log/Log.h> // to use the debugging messages

#include "../../include/otawa/flowfact/FlowFactLoader.h"
#include "GlobalAnalysisProblem.h" // for GC.h to see the Problem class
#include "GC.h"

#include "DynamicBranching.h"

#define DEBUG_FILTERS(x)  // x
#define DEBUG_CLP(x)  // x

using namespace elm::log;
using namespace elm::color;

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
p::feature DYNBRANCH_FEATURE("otawa::dynbranch::DYNBRANCH_FEATURE", new Maker<DynamicBranchingAnalysis>());


static Identifier<Vector<Address> > POSSIBLE_BRANCH_TARGETS("otawa::dynbranch::POSSIBLE_BRANCH_TARGETS"); // only in this file, for the set of target addresses
Identifier<bool> NEW_BRANCH_TARGET_FOUND("otawa::dynbranch::NEW_BRANCH_TARGET_FOUND", false); // on workspace
// if the values of DYNBRANCH_TARGET_COUNT and DYNBRANCH_TARGET_COUNT_PREV differ, it means that there is a new target found, NEW_BRANCH_TARGET_FOUND will be set to true
Identifier<int> DYNBRANCH_TARGET_COUNT("otawa::dynbranch::DYNBRANCH_TARGET_COUNT", -1); // // the current branching target counts for a given instruction
Identifier<int> DYNBRANCH_TARGET_COUNT_PREV("otawa::dynbranch::DYNBRANCH_TARGET_COUNT_PREV", -1); // the previous branching target counts for a given instruction


/*
 * The Cleaner class for DYNBRANCH_FEATURE
 */
class DynamicBranchingCleaner: public Cleaner {
public:
	DynamicBranchingCleaner(WorkSpace* _ws, potential_value_list_t* _pvl, bool _verbose) : ws(_ws), pvl(_pvl), verbose(_verbose) { }

protected:
	virtual void clean(void) {
		// clear the PotentialValues
		//if(verbose)
			//elm::cerr << "potentialValueList length = " << pvl->count() << io::endl;

		//for(potential_value_list_t::Iterator slli(*pvl); slli; slli++) {
#ifdef SAFE_MEM_ACCESS
			dynbranch::PotentialValueMem *pv = *slli;
			if(pv->status == true) {
				pv->pv->~Vector();
			}
			delete pv;
#else
			//dynbranch::PotentialValue *pv = *slli;
//			if(pv->magic != PotentialValue::MAGIC)
//				continue;
			//((Vector<t::uint32> *)pv)->~Vector();
#endif
		//}
		//pvl->clear();

		//elm::StackAllocator* psa = dynbranch::DYNBRANCH_STACK_ALLOCATOR(ws);
		MyGC* psa = dynbranch::DYNBRANCH_STACK_ALLOCATOR(ws);
		ASSERT(psa);
		delete psa;
		dynbranch::DYNBRANCH_STACK_ALLOCATOR(ws).remove();

		dfa::FastState<dynbranch::PotentialValue, MyGC>* dfs = dynbranch::DYNBRANCH_FASTSTATE(ws);
		ASSERT(dfs);
		delete dfs;
		dynbranch::DYNBRANCH_FASTSTATE(ws).remove();
	}
private:
	WorkSpace* ws;
	potential_value_list_t* pvl;
	bool verbose;
};

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
	if(v.mtimes() >= POTENTIAL_VALUE_LIMIT)
		return res;

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

	/* Stop case (semantics.length() < 1)
	 * We have the value from the global analysis, the value from CLP (with filters), and direct access to memory
	 * Since the global analysis only provide EXACT values, we first try to get them
	 * If they aren't known by the global analysis, we ask the clp  analysis for them
	 * If neither analysis has this value, we get this from memory, and if its a register... we continue but the result is going to be 99% wrong ...
	 */
	if(semantics.length() < 1) { // no other sem inst, the STOP CASE
		if (id.fst == MEM) {
			// Priority of memory reading: RO MEM -> Global Analysis -> CLP
			// the value is available in the READ ONLY REGION, then read from it
			if(istate && istate->isReadOnly(id.snd)) {
				PotentialValue r;
				t::uint64 val = 0;
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

		// if we are interested in the content of the memory address [id.snd], the only chances to change the content is when STOREs are issued.
		// hence we are interested in only the STORE but ignore all the other sem inst
		if (id.fst == MEM) { // MEMb(a) <- d

			if (i.op == sem::STORE ) {
				MemID rega = elm::Pair<Memtype,int>(REG,i.a());
				PotentialValue potentialAddr = find(bb,rega,clpin,globalin,semantics);

				if ( potentialAddr.length() > 0 && potentialAddr.get(0) == t::uint32(id.snd)) { // if the address specified with register i.a(), i.e. [i.a()], is found, and matches address from id
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
				case sem::STORE:	// MEMb(a) <- d, i.e. store d, a, Byte, which does not change the reg d at all, hence skip and continue
				case sem::SETP:		// page(d) <- cst
				case sem::TRAP:		// perform a trap
				case sem::NOP:
				case sem::CONT:		// continue in sequence with next instruction
				case sem::SCRATCH:	// d <- T
					return find(bb,id,clpin,globalin,semantics);

				case sem::NEG:		// d <- -a
				case sem::NOT:		// d <- ~a
				case sem::MUL:		// d <- a * b
				case sem::MULU:		// d <- unsigned(a) * unsigned(b)
				case sem::DIV:		// d <- a / b
				case sem::DIVU:		// d <- unsigned(a) / unsigned(b)
				case sem::MOD:		// d <- a % b
				case sem::MODU:		// d <- unsigned(a) % unsigned(b)
				case sem::IF:		// continue if condition a is meet in register b, else jump c instructions
				case sem::CMP:		// d <- a ~ b
				case sem::CMPU:		// d <- a ~u b
					// elm::cerr << elm::log::Debug::debugPrefix(__FILE__, __LINE__,__FUNCTION__) << "not handled sem inst: " << i << io::endl;
					elm::cerr << "WARNING: unsupported semantic instruction " << i << ", obtained the value from clpState" << io::endl;

					if(id.fst == REG) {
						clp::Value valueFromCLP = clpState[semantics.length()].get(clp::Value(clp::REG, id.snd, 0,0));
						PotentialValue r = setFromClp(valueFromCLP);
						return r;
					}
					else
						return PotentialValue();

					return find(bb,id,clpin,globalin,semantics);

				case sem::BRANCH:	// perform a branch on content of register a
					ASSERTP(false, "The BB should not contain other branch semantic instruction(s)!"); // No branch in a BB
				break;

				case sem::LOAD:// d <- MEMb(a)
				{
					PotentialValue r;
					MemID rega = elm::Pair<Memtype,int>(REG,i.a());
					PotentialValue toget = find(bb,rega,clpin,globalin,semantics);

					// Get the value in memory from each possible addresses
					for(PotentialValue::Iterator it(toget); it(); it++) {
						if(istate && istate->isReadOnly(*it)) { // if this specific address is initialized (in the binary)
							potential_value_type val = readFromMem(*it, i.type());// workspace()->process()->get(*it,val);
							r.insert(val);
						}
						else { // if the specific address is not initialized, then try to find this memory ....
							int val = *it;
							MemID memid = elm::Pair<Memtype,int>(MEM,val);
							PotentialValue p = find(bb,memid,clpin,globalin,semantics);
							r = merge(r, p);
						}
					}

					if(r == PotentialValue::bot) { // means couldn't be found from the memory, lets try CLP?
						for(PotentialValue::Iterator it(toget); it(); it++) {
							clp::Value valueFromCLP = clpState[semantics.length()].get(clp::Value(clp::VAL, *it, 0,0));
							if(valueFromCLP != clp::Value::top && valueFromCLP.mtimes() < POTENTIAL_VALUE_LIMIT) {
								PotentialValue p = setFromClp(valueFromCLP);
								r = merge(r, p);
							}
						}
						
						if(toget.length() == 0) { // if there is no address to load, we look the value of the register in CLP state directly
							clp::Value x = clpState[semantics.length()].get(clp::Value(clp::REG, i.d(), 0,0));
							if(x.mtimes() < POTENTIAL_VALUE_LIMIT)
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
				case sem::SETI:    // d <- cst
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
					PotentialValue va = find(bb,rega,clpin,globalin, semantics);
					PotentialValue vb = find(bb,regb,clpin,globalin,semantics);
					v = va + vb;
					return v;
				}
				case sem::SUB:    // d <- a - b
				{
					MemID rega = elm::Pair<Memtype,int>(REG,i.a());
					MemID regb = elm::Pair<Memtype,int>(REG,i.b());
					PotentialValue va = find(bb,rega,clpin,globalin, semantics);
					PotentialValue vb = find(bb,regb,clpin,globalin,semantics);
					v =  va - vb;
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
					ASSERTP(false, elm::log::Debug::debugPrefix(__FILE__, __LINE__,__FUNCTION__) << "unknown semantic instruction: " << i);

				break;
			}
		}
	}
	return PotentialValue();
}

/*
 * If the find() method returns empty PotentialValue, i.e. the length of the PotentialValue &pv is 0, we are looking into the CLP state to see if we can obtain the value
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
 * This is the place to find the target address of a BasicBlock that finishes with a branch.
 *
 */
void DynamicBranchingAnalysis::addTargetToBB(BasicBlock* bb) {
	// semantics is a vector containing the semantic instructions of a given basic block in a reverse order (in respect with the instruction order)
	Vector<sem::inst> semantics;
	for(BasicBlock::InstIter inst(bb); inst(); inst++) {
		sem::Block block;
		inst->semInsts(block);
		for(sem::Block::InstIter semi(block); semi(); semi++)
			semantics.push(*semi);
	}

	// obtain "last" semantic instruction of the basic block
	sem::inst i = semantics.top();
	semantics.pop();

	// not a branch, ignored // the last semantic instruction has to be a branch (at least) so that there can be a target to branch
	if(i.op != sem::BRANCH)
		return;

	Domain globalStateBB = GLOBAL_STATE_IN(bb);

	// start looking for the possible branching address, i.d() is the register containing the target address
	MemID branchID = elm::Pair<Memtype, int>(REG, i.d());
	PotentialValue addresses = find(bb, branchID, clp::STATE_IN(bb), globalStateBB, semantics);

	// when there is no addresses found
	if(addresses.length() < 1) {
		warn(_ << bb << ": no branch addresses found!");
		return;
	}
	/*else {
		for(Set<elm::t::uint32>::Iterator it(addresses); it; it++) {
			if(logFor(LOG_BB)) {
				log << "\t\t\tPossible branching addresses: " << *it << endl;
			}
		}
	}*/

	for(PotentialValue::Iterator pvi(addresses); pvi(); pvi++) {
		Address targetAddr = Address(*pvi);

		bool targetToAdd = true;
		Inst* last = bb->last();

		// Check if the found target has already been recorded for BRANCH TARGET
		for(Identifier<Address>::Getter target(last, BRANCH_TARGET); target(); target++) {
			if(*target == targetAddr) {
				targetToAdd = false;
				break;
			}
		}

		// Check if the found target has already been recorded for CALL TARGET
		for(Identifier<Address>::Getter target(last, CALL_TARGET); target(); target++) {
			if(*target == targetAddr) {
				targetToAdd = false;
				break;
			}
		}

		// Check if the address found is in the program memory
		bool isExecutable = false;
		for(Process::FileIter pfi(workspace()->process()); pfi(); pfi++) {
			for(File::SegIter fsi(*pfi); fsi() && !isExecutable; fsi++) {
				if(fsi->isExecutable() && (*pvi) >= fsi->address().offset() && (*pvi) <= fsi->topAddress().offset()) {
					isExecutable = true;
					break;
				}
			}
		}

		if(!isExecutable) {
			targetToAdd = false;
			log << "WARNING: address " << hex(*pvi) << " is not in the initialized memory, ignored by the dynamic branching analysis" << io::endl;
		}

		// when there are some targets for the BB
		if(targetToAdd) {

			// log
			if(logFor(LOG_BB)) {
				log << "\t\t\tPossible branching addresses: " << targetAddr << " to " << last <<  " @ " << last->address() << endl;
			}

			// check the type: Branch or Call
			if(last->isControl() && last->isBranch())
				BRANCH_TARGET(last).add(targetAddr);
			else if(last->isControl() && last->isCall())
				CALL_TARGET(last).add(targetAddr);
			else
				ASSERTP(0, "Unknown type of the target for " << last << " @ " << last->address());

			// set NEW_BRANCH_TARGET_FOUND to true so notified there is a new target addresses detected
			NEW_BRANCH_TARGET_FOUND(workspace()) = true;
			// increment the target count
			int a = DYNBRANCH_TARGET_COUNT(bb->last());
			a++;
			DYNBRANCH_TARGET_COUNT(bb->last()) = a;
		}
	}
}

p::declare DynamicBranchingAnalysis::reg = p::init("otawa::dynbranch::DynamicBranchingAnalysis", Version(1, 0, 0))
											.require(clp::CLP_ANALYSIS_FEATURE)
											.require(GLOBAL_ANALYSIS_FEATURE)
											.require(dfa::INITIAL_STATE_FEATURE)
											.provide(DYNBRANCH_FEATURE);

/**
 */
DynamicBranchingAnalysis::DynamicBranchingAnalysis(p::declare& r)
		: BBProcessor(r), isDebug(false), time(false), clpManager(0), clpState(), first(true) {
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

	if(first) {
		PotentialValue p;
		_workspace = ws;
		istate = dfa::INITIAL_STATE(ws);
		clpManager = new clp::Manager(ws); // only creates the clp manager once (performance)
		isDebug = DEBUGDYN && isVerbose(); // need to be verbose to be debug
		//addCleaner(COLLECTED_CFG_FEATURE, new DynamicBranchingCleaner(ws, &PotentialValue::potentialValueCollector, isVerbose()));
		addCleaner(COLLECTED_CFG_FEATURE, new DynamicBranchingCleaner(ws, 0, isVerbose()));
		first = false;
	}


	if(!b->isBasic())
		return;

	BasicBlock *bb = b->toBasic();

	// if the BB has unknown target, that means the BB has not been through the analysis
	// initialize the target count to 0 (from -1)
	for(Block::EdgeIter ei = b->outs(); ei(); ei++) {
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
		elm::Vector<se::SECmp *> regFilters = se::REG_FILTERS(bb);
		elm::Vector<se::SECmp *> addrFilters = se::ADDR_FILTERS(bb);


		// to prepare the clpState, which is a Vector of CLP states. Each element of the vector is associated with a semantic instruction
		clpState.clear();

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
			sys::StopWatch watch;
			watch.start();
			for (int i=0; i < NB_EXEC; i++) {addTargetToBB(bb);}
			watch.stop();
			otawa::ot::time t = watch.delay().micros();
			cout << " ---------- Time stat for " << bb << "------------" << endl;
			cout << " Number of instructions in this BB : " << bb->count() << endl;
			cout << " Number executions : " << NB_EXEC << endl;
			cout << " Total time (microsec): " << t << endl;
			cout << " Average time (microsec): " << t/NB_EXEC << endl;
			cout << " ---------- End Time stat for " << bb << "------------" << endl;
		} else {
			addTargetToBB(bb);
		}
	}
}

/**
 * Read a value from the memory.
 * @param address	Address to read.
 * @param type		Type of the data.
 * @return			Read data value.
 */
potential_value_type DynamicBranchingAnalysis::readFromMem(potential_value_type address, sem::type_t type) {
	switch(type) {
	case sem::INT8: 	{ t::int8 d; workspace()->process()->get(address, d); return potential_value_type(d); }
	case sem::INT16: 	{ t::int16 d; workspace()->process()->get(address, d); return potential_value_type(d); }
	case sem::INT32: 	{ t::int32 d; workspace()->process()->get(address, d); return potential_value_type(d); }
	case sem::UINT8: 	{ t::uint8 d; workspace()->process()->get(address, d); return potential_value_type(d); }
	case sem::UINT16: 	{ t::uint16 d; workspace()->process()->get(address, d); return potential_value_type(d); }
	case sem::UINT32: 	{ t::uint32 d; workspace()->process()->get(address, d); return potential_value_type(d); }
	default:			ASSERTP(false, "The type is unknown, please check."); return potential_value_type(0);
	}
}

} // end namespace dynbranch
} // end namespace otawa
