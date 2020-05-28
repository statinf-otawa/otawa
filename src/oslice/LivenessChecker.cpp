/*
 *	LivenessChecker class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2016, IRIT UPS.
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
#include <otawa/oslice/LivenessChecker.h>
#include <otawa/util/Bag.h>
#include <elm/util/Pair.h>

namespace otawa { namespace oslice {
Identifier<dfa::MemorySet::t* > MEM_BB_END_IN("otawa::oslice::LIVING_MEM_IN", NULL);
Identifier<dfa::MemorySet::t*> MEM_BB_BEGIN_OUT("otawa::oslice::LIVING_MEM_OUT", NULL);
Identifier<BitVector> REG_BB_END_IN("otawa::oslice::LIVING_REG_IN");
Identifier<BitVector> REG_BB_BEGIN_OUT("otawa::oslice::LIVING_REG_OUT");

static Identifier<inst_clp_bag_t> MEM_READ_BAG_BB("otawa::oslice::LIVING_MEM_READ_BAG");
static Identifier<inst_clp_bag_t> MEM_WRITE_BAG_BB("otawa::oslice::LIVING_MEM_WRITE_BAG");
// for manager
static Identifier<dfa::MemorySet::t*> MEM_INST("otawa::oslice::MEM_INST");
static Identifier<BitVector> REG_INST("otawa::oslice::REG_INST");
static Identifier<int> MEM_ACCESS_IDENTIFIED("otawa::oslice::MEM_ACCESS_IDENTIFIED", 0);
}}


#define CLP_THRESHOLD 0xCAFE

namespace otawa { namespace oslice {

Identifier<int> LIVENESS_DEBUG_LEVEL("otawa::oslice::LIVENESS_DEBUG_LEVEL", 0);
p::feature LIVENESS_FEATURE("otawa::oslice::LIVENESS_FEATURE", new Maker<LivenessChecker>());

p::declare LivenessChecker::reg = p::init("otawa::oslice::LivenessChecker", Version(2, 0, 0))
       .maker<LivenessChecker>()
	   .use(COLLECTED_CFG_FEATURE)
       .use(otawa::clp::CLP_ANALYSIS_FEATURE)
       .provide(LIVENESS_FEATURE);

elm::BitVector LivenessChecker::_defaultRegisters = 1;
t::uint32 LivenessChecker::_debugLevel = 0;
clp::Manager *LivenessChecker::clpManager = 0;


LivenessChecker::LivenessChecker(AbstractRegistration& _reg) : otawa::Processor(_reg) {
}

void LivenessChecker::configure(const PropList &props) {
	Processor::configure(props);
	_debugLevel = LIVENESS_DEBUG_LEVEL(props);
	if (workspace() != nullptr) {
		LIVENESS_DEBUG_LEVEL(workspace()) = _debugLevel;
	}
}

/**
 * @class LivenessChecker
 * The LivenessChecker provides the status of the liveness of the registers and memory addresses
 * @ingroup oslice
 */

/**
 * Processing the workspace.
 * @param fw		Current workspace.
 */
void LivenessChecker::processWorkSpace(WorkSpace *fw) {
	_defaultRegisters = BitVector(workspace()->platform()->regCount(), false);
	const CFGCollection& coll = **otawa::INVOLVED_CFGS(workspace());

	// the main CFG
	CFG *cfg = coll[0];
	// now lets find the program exit block
	Block* programExit = cfg->exit();

#ifdef ORIG
	// we have a list of blocks which will go to the end of the program ?
	interested_instructions_t *interestedInstructions = new interested_instructions_t();
	for (Block::EdgeIter edge = programExit->ins(); edge; edge++) {
		if(!edge->source()->isBasic())
			continue;
		InterestedInstruction* interestedInstruction = new InterestedInstruction(edge->source()->toBasic()->last(), edge->source()->toBasic());
		interestedInstructions->add(interestedInstruction);
	}
	// DBG: list a list of BBs
	if(_debugLevel & DISPLAY_LIVENESS_STAGES)
		for(interested_instructions_t::Iterator iter(*interestedInstructions); iter; ++iter)
			elm::cerr << __SOURCE_INFO__ << "Block to start: BB " << iter->getBB()->index() << " @ " << iter->getBB()->address() << ", ends with instruction " << iter->getInst() << " @ " << iter->getBB()->address() << io::endl;
#endif

	// we need to get the CLP information in order to do address access
	clpManager = new clp::Manager(workspace());

#ifdef ORIG
	buildReverseSynthLink(coll);
	initIdentifiersForEachBB(coll);
	if (interestedInstructions) {
		if(_debugLevel & DISPLAY_LIVENESS_STAGES)
			elm::cerr << __SOURCE_INFO__<< "The list of interested instructions: " << io::endl;

		// now we look into each of these instructions
		for(interested_instructions_t::Iterator currentII(*interestedInstructions); currentII; currentII++) {
			if(_debugLevel & DISPLAY_LIVENESS_STAGES) {
				elm::cerr << __SOURCE_INFO__ << "    " << currentII->getInst() << " @ " << currentII->getInst()->address() << " from BB " << currentII->getBB()->index() << io::endl;
				elm::cerr << __SOURCE_INFO__ << "Popping interested instruction " << currentII->getInst() << " @ " << currentII->getInst()->address() << io::endl;
			}
			Inst* currentInst = currentII->getInst();
			BasicBlock* currentBB = currentII->getBB();

			elm::BitVector workingRegs(workspace()->platform()->regCount(), false);
			provideRegisters(currentInst, workingRegs, 0);

			// obtain the working register
			if(_debugLevel & DISPLAY_LIVENESS_STAGES) {
				elm::cerr << __SOURCE_INFO__ << "Creating the initial Regs from " << currentInst << " @ " << currentInst->address() << io::endl;
				elm::cerr << __SOURCE_INFO__ << __TAB__ << "Working regs = " << workingRegs << io::endl;
			}

			// obtain the working memory
			// first identify all the access of the address of the current BB
			identifyAddrs(currentBB);
			// then find out if the instruction access the memory
			//clp_value_set_t workingMems;
			dfa::MemorySet::t workingMems(0);
			int currentReadMemIndex = 0;
			LivenessChecker::getMems(currentBB, currentInst, currentReadMemIndex, workingMems, 0);

			// workingMem is going to be different for each working element, therefore we allocate the memory
			if(_debugLevel & DISPLAY_LIVENESS_STAGES) {
				elm::cerr << __SOURCE_INFO__ << __TAB__ << "Working mems = "; displayAddrs(elm::cerr, workingMems); elm::cerr << io::endl;
			}

			// define the working list of BB
			Vector<WorkingElement*> workingList;

			// first we put the current BB in to the list
			workingList.add(new WorkingElement(currentBB, currentInst, workingRegs, workingMems));

			// process the working list
			processWorkingList(workingList);
		}
	} // end if (interestedInstructions)

	// finished using interestedInstructions, free it
	for(interested_instructions_t::Iterator iter(*interestedInstructions); iter; ++iter)
		delete (*iter);
	interestedInstructions->clear();
	delete interestedInstructions;
#else
	// get the working register
	elm::BitVector workingRegs(workspace()->platform()->regCount(), false);
	// at the end of the program, no memory address will be used
	dfa::MemorySet::t workingMems(0);
	// define the working list of BB
	Vector<WorkingElement*> workingList;
	// the first element of the list is the exit block
	workingList.add(new WorkingElement(programExit, 0, workingRegs, workingMems));
	// process the working list
	processWorkingList(workingList);
#endif
	// end of using manager
	delete clpManager;
} // processWorkSpace

void LivenessChecker::initIdentifiersForEachBB(const CFGCollection& coll) {
	// for each CFG
	for (int i = 0; i < coll.count(); i++) {
		CFG *cfg = coll[i]; // current CFG
		// for each BB in the CFG
		for (CFG::BlockIter v = cfg->blocks(); v(); v++) {
			if(!v->isBasic())
				continue;
			REG_BB_END_IN(*v) = BitVector(workspace()->platform()->regCount(), false);
			REG_BB_BEGIN_OUT(*v) = BitVector(workspace()->platform()->regCount(), false);
			MEM_BB_END_IN(*v) = new dfa::MemorySet::t(0);
			MEM_BB_BEGIN_OUT(*v) = new dfa::MemorySet::t(0);
		} // end for (CFG::BlockIter v = cfg->blocks(); v; v++) {
	} // end for (int i = 0; i < coll.count(); i++) {
}

void LivenessChecker::provideRegisters(Inst* inst, elm::BitVector& regsToModify, int readOrWrite) {
	// find out the registers to read/write
	Array<hard::Register *> regTable;
	if(readOrWrite == 0)
		regTable = inst->readRegs();
	else
		regTable = inst->writtenRegs();

	for(Array<hard::Register *>::Iter currReg(regTable); currReg(); ++currReg)
		regsToModify.set(currReg->platformNumber());
}

void LivenessChecker::processWorkingList(Vector<WorkingElement*>& workingList) {
	// while the list is not empty
	while(workingList.count())
	{
		// pop the first element to process
		WorkingElement* we = workingList.pop();
		Block* currentBB_wl = we->_bb;
		elm::BitVector currentRegs_wl = we->_workingRegs;
		dfa::MemorySet::t currentMems_wl(we->_workingMems); // load the memory access set
		Inst* currentInst_wl = we->_inst;
		if(_debugLevel & DISPLAY_LIVENESS_STAGES) {
			elm::cerr << __SOURCE_INFO__ << __RED__ << "Popping new working element out: CFG " << currentBB_wl->cfg()->index() << ", " <<  currentBB_wl << __RESET__ << io::endl;
			elm::cerr << __SOURCE_INFO__ << __TAB__ << "with working Regs  = " << currentRegs_wl << io::endl;
			elm::cerr << __SOURCE_INFO__ << __TAB__ << "with working MEM   = "; displayAddrs(elm::cerr, currentMems_wl); elm::cerr << io::endl;
		}
		delete we;

		int currentReadMemIndex = 0;
		int currentWriteMemIndex = 0;
		bool reachFirstInstCurrentBB_wl = true;

		if(currentBB_wl->isBasic()) {
			reachFirstInstCurrentBB_wl = false;
			// here we fill the memory access information for each instruction
			identifyAddrs(currentBB_wl->toBasic());
		}

		while(!reachFirstInstCurrentBB_wl) // only comes in when the block fetched in is a Basic Block with instructions
		{
			if(_debugLevel & DISPLAY_LIVENESS_STAGES)
				elm::cerr << __SOURCE_INFO__ << __CYAN__ << "Processing " << currentInst_wl << " @ " << currentInst_wl->address() << __RESET__ << io::endl;
			elm::BitVector currentRegsDef(workspace()->platform()->regCount(), false);
			elm::BitVector currentRegsUse(workspace()->platform()->regCount(), false);
			provideRegisters(currentInst_wl, currentRegsUse, 0);
			provideRegisters(currentInst_wl, currentRegsDef, 1);

			// for memory access
			dfa::MemorySet::t addressInstRead(0), addressInstWrite(0);
			LivenessChecker::getMems(currentBB_wl->toBasic(), currentInst_wl, currentReadMemIndex, addressInstRead, 0);
			LivenessChecker::getMems(currentBB_wl->toBasic(), currentInst_wl, currentWriteMemIndex, addressInstWrite, 1);

			if(_debugLevel & DISPLAY_LIVENESS_STAGES) {
				elm::cerr << __SOURCE_INFO__ << __TAB__ << "Reg Def       = " << currentRegsDef << io::endl;
				elm::cerr << __SOURCE_INFO__ << __TAB__ << "Reg Use       = " << currentRegsUse << io::endl;
				elm::cerr << __SOURCE_INFO__ << __TAB__ << "Mem Def       = "; displayAddrs(elm::cerr, addressInstWrite); elm::cerr << io::endl;
				elm::cerr << __SOURCE_INFO__ << __TAB__ << "Mem Use       = "; displayAddrs(elm::cerr, addressInstRead); elm::cerr << io::endl;
				elm::cerr << __SOURCE_INFO__ << __TAB__ << "RegSet Before = " << currentRegs_wl << io::endl;
				elm::cerr << __SOURCE_INFO__ << __TAB__ << "MemSet Before = "; displayAddrs(elm::cerr, currentMems_wl); elm::cerr << io::endl;
			}

			// check if the def address is over lap with the working addrs
			bool memInterested = interestingAddrs(currentMems_wl, addressInstWrite);

			// if working Regs & def Regs is not zero, that means this instruction provides
			// the registers that we are interested
			bool regInsterested = interestingRegs(currentRegs_wl, currentRegsDef); // !(currentRegs_wl & currentRegsDef).isEmpty();

			if(memInterested | regInsterested) {
				// update the working Regs
				currentRegs_wl = (currentRegs_wl - currentRegsDef) | currentRegsUse;
				// update the current working memory
				updateAddrsFromInstruction(currentMems_wl, addressInstRead, addressInstWrite, _debugLevel);
			}

			if(_debugLevel & DISPLAY_LIVENESS_STAGES) {
				elm::cerr << __SOURCE_INFO__ << __TAB__ << "RegSet After  = " << currentRegs_wl << io::endl;
				elm::cerr << __SOURCE_INFO__ << __TAB__ << "MemSet After  = "; displayAddrs(elm::cerr, currentMems_wl); elm::cerr << io::endl;
			}

			if(currentInst_wl == currentBB_wl->toBasic()->first())
				reachFirstInstCurrentBB_wl = true;
			else
				currentInst_wl = currentInst_wl->prevInst();
		} // reaches the beginning of the BB

		// merge current working state with the previous state at the beginning of the BB
		elm::BitVector t1 = REG_BB_BEGIN_OUT(currentBB_wl);
		if(!t1.size()) // if t1 is empty that means there is no info about the register
			REG_BB_BEGIN_OUT(currentBB_wl) = currentRegs_wl;
		else
			REG_BB_BEGIN_OUT(currentBB_wl) = t1 | currentRegs_wl;

		dfa::MemorySet::t *temp = MEM_BB_BEGIN_OUT(currentBB_wl);
		if(!temp) {
			temp = new dfa::MemorySet::t(0);
			MEM_BB_BEGIN_OUT(currentBB_wl) = temp;
		}
		*temp = dfa::MemorySet().join(*temp, currentMems_wl);

		// here reaches the beginning of the BB, now we need to list the list of incoming edges
		// so we can keep trace back the previous BB
		// first we find the predecessors of the BB to process
		Vector<Block *> predecessors;

		for (Block::EdgeIter e = currentBB_wl->ins(); e(); e++) {
			Block* b = e->source(); // find the source of the edge, the predecessor of current BB
			if (b->isEntry()) {
				// then get the set of the callers
				for(auto caller: b->cfg()->callers()) {
					if(_debugLevel & DISPLAY_LIVENESS_STAGES)
						elm::cerr << __SOURCE_INFO__ << "Found a caller @ CFG " << caller->cfg()->index() << ", BB " << caller->index() << io::endl;

					for(Block::EdgeIter bei = caller->ins(); bei(); bei++) {
						if(_debugLevel & DISPLAY_LIVENESS_STAGES)
							elm::cerr << __SOURCE_INFO__ << __TAB__ << __GREEN__ << "Adding the block @ CFG " << bei->source()->cfg()->index() << ", " << bei->source() << __RESET__ << io::endl;
						predecessors.add(bei->source());
					}
				}
			} // end of handling the entry block
			else if (b->isBasic()) {
				// we put the BB here into the working list with given
				if(_debugLevel & DISPLAY_LIVENESS_STAGES)
					elm::cerr << __SOURCE_INFO__ << "Found predecessor BB @ " << b->toBasic()->address() << io::endl;
				predecessors.add(b->toBasic());
			} // end of handling the basic block
			else if (b->isSynth()) {
				// this means that we reach current BB from the returning of a function
				// we obtain the exit block of the function that it returns from

				if(_debugLevel & DISPLAY_LIVENESS_STAGES) {
					elm::cerr << __SOURCE_INFO__ << "Found a Synth block with the callee to " << b->toSynth()->callee()->label() << io::endl;
				}

				// find the BB which is the last block before the exit block of the callee
				if(!b->toSynth()->callee())
					continue;
				Block* end = b->toSynth()->callee()->exit();
				// each edge to the exit block is a possible BB which will goes to the current block
				for (Block::EdgeIter EdgeToExit = end->ins(); EdgeToExit(); EdgeToExit++) {
					Block* BB_BeforeReturn = EdgeToExit->source();
					if(_debugLevel & DISPLAY_LIVENESS_STAGES)
						elm::cerr << __SOURCE_INFO__ << __GREEN__ << "Adding block CFG " << BB_BeforeReturn->cfg()->index() << ", " << BB_BeforeReturn << __RESET__ << io::endl;
					predecessors.add(BB_BeforeReturn);
				}

			} // end of handling the synth block
			else {
				if (b->isEntry())
					elm::cerr << "ENTRY" << io::endl;
				else if (b->isExit())
					elm::cerr << "EXIT" << io::endl;
				else if (b->isUnknown())
					elm::cerr << "unknown" << io::endl;
				ASSERTP(false, "Encounter an unexpected block");
			}
		} // end of finding predecessors of the current BB

		if(predecessors.count() == 0)
			if(_debugLevel & DISPLAY_LIVENESS_STAGES)
				elm::cerr << __SOURCE_INFO__ << __RED__ << "No predecessor for BB @ " << currentBB_wl->address() << __RESET__ << io::endl;

		// process the collected BBs
		// now we need to see if the input (register and memory uses) feed from the successor matches totally or a subset of the pred BB
		for(Vector<Block *>::Iter predecessor(predecessors); predecessor(); ++predecessor) {
			BitVector bv = REG_BB_END_IN(*predecessor);
			if(!bv.size()) {
				bv = BitVector(workspace()->platform()->regCount(), false);
				REG_BB_END_IN(*predecessor) = bv;
			}
			dfa::MemorySet::t* memIn = MEM_BB_END_IN(*predecessor);
			if(!memIn) {
				memIn = new dfa::MemorySet::t(0);
				MEM_BB_END_IN(*predecessor) = memIn;
			}

			bool notContainsAllRegs = !bv.includes(currentRegs_wl);
			bool notContainsAllMems = !containsAllAddrs(*memIn, currentMems_wl);

			if(_debugLevel & DISPLAY_LIVENESS_STAGES) {
				elm::cerr << __SOURCE_INFO__ << __TAB__ << "for BB @ " << predecessor->address() << io::endl;
				elm::cerr << __SOURCE_INFO__ << __TAB__ << __TAB__ << "Register used: " << bv << " contains all of " << currentRegs_wl << " ? "<< !notContainsAllRegs << io::endl;
				elm::cerr << __SOURCE_INFO__ << __TAB__ << __TAB__ << "Mem      used: "; displayAddrs(elm::cerr, *memIn); elm::cerr << " contains all of "; displayAddrs(elm::cerr, currentMems_wl); elm::cerr << "? " << !notContainsAllMems << io::endl;
			}

			if(notContainsAllRegs | notContainsAllMems | !currentBB_wl->isBasic()) {
				if(_debugLevel & DISPLAY_LIVENESS_STAGES)
					elm::cerr << __SOURCE_INFO__ << __GREEN__ << "Adding BB @ CFG " << predecessor->cfg()->index() << ", " << *predecessor << " to the working list." << __RESET__ << io::endl;
				bv = bv | currentRegs_wl;
				REG_BB_END_IN(*predecessor) = bv;
				//memIn->addAll(currentMems_wl);
				*memIn = dfa::MemorySet().join(*memIn, currentMems_wl);
				MEM_BB_END_IN(*predecessor) = memIn;
				Inst* lastInstruction = 0;
				if(predecessor->isBasic())
					lastInstruction = predecessor->toBasic()->last();
				WorkingElement *we = new WorkingElement(*predecessor, lastInstruction, currentRegs_wl, currentMems_wl);
				workingList.add(we);
			}
			else
				if(_debugLevel & DISPLAY_LIVENESS_STAGES)
					elm::cerr << __SOURCE_INFO__ << __RED__ << "NOT adding BB @ " << predecessor->address() << " to the working list." << __RESET__ << io::endl;
		}
	} // end of the current working list
}

void LivenessChecker::clearAddrs(BasicBlock* bb) {
}

/*
 * It uses clp manager to determine the addresses accessed by the semantic instructions
 */
void LivenessChecker::identifyAddrs(BasicBlock* bb) {
	int identified = MEM_ACCESS_IDENTIFIED(bb);
	if(identified == 1) {
		if(_debugLevel & IDENTIFY_MEM_ACCESS)
			elm::cerr << __SOURCE_INFO__ << "The memory access for BB " << bb->index() << " @ " << bb->address() << " has been identified." << io::endl;
		return;
	}
	else
		MEM_ACCESS_IDENTIFIED(bb) = 1;

	clp_vector_t memReadVector;
	clp_vector_t memWriteVector;
	clp::Manager::step_t step = clpManager->start(bb);
	if(_debugLevel & IDENTIFY_MEM_ACCESS)
		elm::cerr << __SOURCE_INFO__<< __TAB__ << __YELLOW__ << "In BB " << bb->index() << " @ " << bb->address() << ", the following memory accesses are identified:" << __RESET__ << io::endl;

	// the current instruction
	while(step) {
		int mode = -1;
	    switch(clpManager->sem().op) {
	    case sem::LOAD:
	    	mode = 0;
	    	break;
	    case sem::STORE:
	    	mode = 1;
	    	break;
	    }

		if (mode != -1) {
			clp::Value v = clpManager->state()->get(clp::Value(clp::REG, clpManager->sem().addr()));
			if(_debugLevel & IDENTIFY_MEM_ACCESS)
				elm::cerr << __SOURCE_INFO__<< __TAB__ << __TAB__ << __YELLOW__ << clpManager->sem() << " of " << clpManager->inst() << " @ " << clpManager->inst()->address() << ((mode == 0)?" loads ":" stores ") << v << __RESET__<< io::endl;

			sem::inst si = clpManager->sem();
			int dataSize = 0;
			switch(si.type()) {
				case sem::INT8: dataSize = 1; break;
				case sem::INT16: dataSize = 2; break;
				case sem::INT32: dataSize = 4; break;
				case sem::UINT8: dataSize = 1; break;
				case sem::UINT16: dataSize = 2; break;
				case sem::UINT32: dataSize = 4; break;
			default: dataSize = 0; break;
			}


			if(mode == 0)
				memReadVector.addFirst(MemoryAccessInformation(clpManager->inst(), v, dataSize)); // add the instruction* and the clp value into the vector for reading mems
			else
				memWriteVector.addFirst(MemoryAccessInformation(clpManager->inst(), v, dataSize)); // add the instruction* and the clp value into the vector for writing mems
		}
		step = clpManager->next();
	} // end of the steps

	MEM_READ_BAG_BB(bb) = inst_clp_bag_t(memReadVector);
	MEM_WRITE_BAG_BB(bb) = inst_clp_bag_t(memWriteVector);
}

//void LivenessChecker::updateAddrsFromInstruction(clp_value_set_t & workingMem, clp_value_set_t & readMem, clp_value_set_t & writeMem, t::uint32 debugLevel) {
void LivenessChecker::updateAddrsFromInstruction(otawa::dfa::MemorySet::t & workingMem, otawa::dfa::MemorySet::t & readMem, otawa::dfa::MemorySet::t & writeMem, t::uint32 debugLevel) {
	dfa::MemorySet ms;
	dfa::MemorySet::t intersection = ms.meet(workingMem, writeMem);
	for(dfa::MemorySet::Iter msi = intersection.areas(); msi(); msi++) {
		if(debugLevel & DISPLAY_LIVENESS_STAGES) {
			elm::cerr << __SOURCE_INFO__ << __TAB__ << __BLUE__ << "Removing " << *msi << " from the workingMems" << __RESET__ << io::endl;
		}
		workingMem = ms.remove(workingMem, *msi);
	}
	workingMem = ms.join(workingMem, readMem);
}

//bool LivenessChecker::containsAllAddrs(clp_value_set_t & a, clp_value_set_t & b) {
bool LivenessChecker::containsAllAddrs(otawa::dfa::MemorySet::t & a, otawa::dfa::MemorySet::t & b) {
	dfa::MemorySet ms;
	for(dfa::MemorySet::Iter msi = b.areas(); msi(); msi++)
		if(!ms.contains(a, *msi))
			return false;
	return true;
}

//bool LivenessChecker::interestingAddrs(clp_value_set_t const & a, clp_value_set_t const & b) {
bool LivenessChecker::interestingAddrs(dfa::MemorySet::t const & a, dfa::MemorySet::t const & b) {
	return true;
}
bool LivenessChecker::interestingRegs(elm::BitVector const & a, elm::BitVector const & b) {
	return true;
}


/**
 * Processing the workspace.
 * @param bb		The current processing basic block
 */
void LivenessChecker::getMems(BasicBlock* bb, Inst* inst, int & currentIndex, otawa::dfa::MemorySet::t & memorySet, int readOrWrite) {
	inst_clp_bag_t memBag;
	if(readOrWrite == 0) // read
		memBag = MEM_READ_BAG_BB(bb);
	else
		memBag = MEM_WRITE_BAG_BB(bb);

	while((currentIndex < memBag.count()) && (memBag[currentIndex].inst == inst)) {
		clp::Value temp = memBag[currentIndex].clpValue;

		if(_debugLevel & DISPLAY_WIP)
			elm::cerr << __SOURCE_INFO__ << "temp = " << temp << io::endl;

		if(temp.kind() == clp::ALL) {
			dfa::MemorySet ms;
			memorySet = ms.add(memorySet, MemArea(0, 0xFFFFFFFF));
		}
		else if(temp.delta()) {
			if(_debugLevel & DISPLAY_WIP)
				elm::cerr << __SOURCE_INFO__ << "size = " << memBag[currentIndex].size << io::endl;
			//if(temp.mtimes() == (unsigned)(-1)) {
			if(temp.mtimes() > CLP_THRESHOLD) {
				dfa::MemorySet ms;
				memorySet = ms.add(memorySet, MemArea(Address(0), Address(0xFFFFFFFF)));
			}
			else {
				//assert(0); // FIXME
				for(elm::t::uint32 i = 0; i <= temp.mtimes(); i++) {
					dfa::MemorySet ms;
					memorySet = ms.add(memorySet, MemArea(temp.lower() + i* temp.delta(), memBag[currentIndex].size));
				}
			}
		}
		else {
			dfa::MemorySet ms;
			memorySet = ms.add(memorySet, MemArea(temp.lower(), memBag[currentIndex].size));
		}

		currentIndex++;
	}
}

void LivenessChecker::buildReverseSynthLink(const CFGCollection& coll) {
	// for each CFG
	for (int i = 0; i < coll.count(); i++) {
		CFG *cfg = coll[i]; // current CFG
		// for each BB in the CFG
		for(CFG::BlockIter v = cfg->blocks(); v(); v++) {
			if (v->isSynth()) {
				if(!v->toSynth()->callee()) {
					elm::cerr << __SOURCE_INFO__ << "Block " << v->cfg()->index() << ":" << v->index() << " of CFG " << v->cfg()->name() << " has no callee" << io::endl;
					continue;
				}
				// find all the edge goes into the synth block, which are the callers of the block
				for (Block::EdgeIter e = v->ins(); e(); e++) {
					// the source of an edge is one caller
					BasicBlock* caller = e->source()->toBasic();
					// obtain the set of the callers from the entry block of the synth block
					BBSet* setOfCallers = SetOfCallers(v->toSynth()->callee()->entry());
					// create the set if not existed yet
					if(!setOfCallers)
						setOfCallers = new BBSet();
					// add the caller in it
					setOfCallers->add(caller);
					// put it back
					SetOfCallers(v->toSynth()->callee()->entry()) = setOfCallers;
				}
			} // end if ( v->isSynth())
		} // end for(CFG::BlockIter v = cfg->blocks(); v; v++) {
	} // end for (int i = 0; i < coll.count(); i++) {
}

Manager::Manager(WorkSpace *ws) {
	_debugLevel = LIVENESS_DEBUG_LEVEL(ws);
}

/**
 * Start the interpretation of a basic block.
 * @param bb	Basic block to interpret.
 */
Manager::step_t Manager::start(BasicBlock *bb) {
	if(bb->isEntry() || bb->isExit())
		return ENDED;

	_currBB = bb;
	_currInst = bb->first();
	_currInstIndex = 0;

	// load the state (regs + mems) information from the bb
	dfa::MemorySet::t* memBeginState = MEM_BB_BEGIN_OUT(bb);
	dfa::MemorySet::t* memEndState = MEM_BB_END_IN(bb);
	elm::BitVector regBeginState = REG_BB_BEGIN_OUT(bb);
	elm::BitVector regEndState = REG_BB_END_IN(bb);
	// obtain the working memory set of the bb
	inst_clp_bag_t mrbg = MEM_READ_BAG_BB(bb);
	inst_clp_bag_t mwbg = MEM_WRITE_BAG_BB(bb);

	// output the information of the current BB
	if(_debugLevel & DISPLAY_BB_STATE_INFO) {
		elm::cerr << __SOURCE_INFO__ << __GREEN__ << "The manager starts at the BB " << bb->index() << " @ " << bb->address() << __RESET__ << io::endl;
		elm::cerr << __SOURCE_INFO__ << __TAB__ << "memBeginState = "; otawa::oslice::displayAddrs(elm::cerr, *memBeginState); elm::cerr << io::endl;
		elm::cerr << __SOURCE_INFO__ << __TAB__ << "memEndState   = "; otawa::oslice::displayAddrs(elm::cerr, *memEndState); elm::cerr << io::endl;
		elm::cerr << __SOURCE_INFO__ << __TAB__ << "regBeginState = " << regBeginState << io::endl;
		elm::cerr << __SOURCE_INFO__ << __TAB__ << "regEndState   = " << regEndState << io::endl;
	}

	if(_debugLevel & DISPLAY_BB_MEM_ACCESS) {
		elm::cerr << __SOURCE_INFO__ << "The list of accessed memories and associated instructions:" << __RESET__ << io::endl;
		for(int i = 0; i < mrbg.count(); i++) {
			elm::cerr << __SOURCE_INFO__ << __TAB__ << mrbg[i].inst << " @ " << mrbg[i].inst->address() << " reads " << mrbg[i].clpValue << io::endl;
		}
		for(int i = 0; i < mwbg.count(); i++) {
			elm::cerr << __SOURCE_INFO__ << __TAB__ << mwbg[i].inst << " @ " << mwbg[i].inst->address() << " writes " << mwbg[i].clpValue << io::endl;
		}
	}

	// now fill in the information of the BB
	// workingMem and workingReg are used to re-backward-walking of the current bb
	//clp_value_set_t* workingMem = new clp_value_set_t(*memEndState);
	// clp_value_set_t* workingMem = new clp_value_set_t(*memEndState);
	dfa::MemorySet::t * workingMem = memEndState;
	elm::BitVector workingReg = elm::BitVector(regEndState);
	// starts from the last instruction of the bb and going to the first inst
	Inst* workingInst = bb->last();
	// the index of the Bag, within the bag, the element are sorted by the address of the instruction in the descending order
	int currentReadMemIndex = 0;
	int currentWriteMemIndex = 0;
	// keep going until the previous instruction of the first instruction of the basic block
	while(workingInst != bb->first()->prevInst()) {
		// obtain the use/def of register of the instruction
		elm::BitVector regUse = LivenessChecker::getRegisters();
		elm::BitVector regDef = LivenessChecker::getRegisters();
		LivenessChecker::provideRegisters(workingInst, regUse, 0);
		LivenessChecker::provideRegisters(workingInst, regDef, 1);
		workingReg = (workingReg - regDef) | regUse;
		REG_INST(workingInst) = workingReg;

		if(_debugLevel & DISPLAY_BB_MEM_ACCESS)
			elm::cerr << __SOURCE_INFO__ << __CYAN__ << "Processing " << workingInst << " @ " << workingInst->address() << io::endl;

		while((currentWriteMemIndex < mwbg.count()) && (mwbg[currentWriteMemIndex].inst == workingInst)) {
			clp::Value temp = mwbg[currentWriteMemIndex].clpValue;
			// because the memory is defined in this instruction, hence it has to be removed from the working mems for the previous instructions
			dfa::MemorySet ms;
			if(_debugLevel & DISPLAY_BB_MEM_ACCESS)
				elm::cerr << __SOURCE_INFO__ << "Removing " << temp << "from the workingMem" << io::endl;
			if(temp.kind() == clp::ALL) {
				dfa::MemorySet ms;
				*workingMem = dfa::MemorySet::empty;
			}
			else if(temp.delta()) {
				ASSERT(0);
			}
			else {
				dfa::MemorySet ms;
				*workingMem = ms.remove(*workingMem, MemArea(temp.lower(), mwbg[currentWriteMemIndex].size));
			}
			currentWriteMemIndex++;
		}

		while((currentReadMemIndex < mrbg.count()) && (mrbg[currentReadMemIndex].inst == workingInst)) {
			clp::Value temp = mrbg[currentReadMemIndex].clpValue;
			if(_debugLevel & DISPLAY_BB_MEM_ACCESS)
				elm::cerr << __SOURCE_INFO__ << "Adding " << temp << " to the workingMem" << io::endl;
			if(temp.kind() == clp::ALL) {
				dfa::MemorySet ms;
				*workingMem = ms.add(*workingMem, MemArea(0, 0xFFFFFFFF));
			}
			else if(temp.delta()) {
				ASSERT(0);
			}
			else {
				dfa::MemorySet ms;
				*workingMem = ms.add(*workingMem, MemArea(temp.lower(), mrbg[currentReadMemIndex].size));
			}
			currentReadMemIndex++;
		}

//		// add the current working mem into a vector then transfer that to the bag
//		Vector<otawa::clp::Value> tempCLPVector;
//		//for(clp_value_set_t::Iterator currMem(*workingMem); currMem; ++currMem)
//		for(dfa::MemorySet::Iter msi = workingMem->areas(); msi; msi++) {
//			// tempCLPVector.add(*currMem); FIXME
//			elm::cerr << "Adding " << *msi << io::endl;
//			assert(0);
//		}
//		MEM_INST(workingInst) = clp_bag_t(tempCLPVector);
		MEM_INST(workingInst) = new dfa::MemorySet::t(*workingMem);

		if(_debugLevel & DISPLAY_WORKING_SET) {
			elm::cerr << __SOURCE_INFO__ << "workingInst = " << workingInst << " @ " << workingInst->address() << io::endl;
			elm::cerr << __SOURCE_INFO__ << __TAB__ << "reg = " << workingReg << io::endl;
			elm::cerr << __SOURCE_INFO__ << __TAB__ << "mem = "; otawa::oslice::displayAddrs(elm::cerr, *workingMem); elm::cerr << io::endl;
		}

		workingInst = workingInst->prevInst();
	}

	// make sure the resulted regs is the same as the previous run
	if(workingReg != regBeginState) {
		elm::cerr << __SOURCE_INFO__ << "workingReg    = " << workingReg << io::endl;
		elm::cerr << __SOURCE_INFO__ << "regBeginState = " << regBeginState << io::endl;
		ASSERT(workingReg == regBeginState);
	}
	ASSERT(*workingMem == *memBeginState);

	// working mem must be freed here...
	// fixme workingMem->clear();
	// fixme delete workingMem;
	return NEW_INST;
}

Manager::step_t Manager::next(void) {
	if(_currInst == _currBB->last())
		return ENDED;

	// we are at the next instruction
	_currInstIndex++;
	_currInst = _currInst->nextInst();

	//displayState(elm::cerr);
	return NEW_INST;
}

void Manager::displayState(io::Output & output) {
	//clp_bag_t v =  MEM_INST(_currInst);
	dfa::MemorySet::t* v =  MEM_INST(_currInst);
	BitVector r = REG_INST(_currInst);
	elm::cerr << __SOURCE_INFO__ << __BLUE__ << "Instruction " << _currInst << " @ " << _currInst->address() << __RESET__ << io::endl;
	elm::cerr << __SOURCE_INFO__ << __TAB__ << "reg = " << r << io::endl;
	elm::cerr << __SOURCE_INFO__ << __TAB__ << "mem = ";
	otawa::oslice::displayAddrs(elm::cerr, *v);
	elm::cerr << io::endl;
}

elm::BitVector Manager::workingRegs(void) {
	BitVector r = REG_INST(_currInst);
	return r;
}
//clp_bag_t Manager::workingMems(void) {
dfa::MemorySet::t* Manager::workingMems(void) {
	dfa::MemorySet::t* v =  MEM_INST(_currInst);
	return v;
}

bool Manager::isRegAlive(int regID) {
	BitVector r = REG_INST(_currInst);
	return r[regID];
}

bool Manager::isMemAlive(otawa::Address memAddress) {
	dfa::MemorySet::t* v =  MEM_INST(_currInst);
	dfa::MemorySet ms;
	return ms.contains(*v, memAddress);
//	for(int i = 0; i < v.count(); i++) {
//		clp::Value temp = v[i];
//		temp.inter(memAddress.offset());
//		if(temp != clp::Value::none)
//			return true;
//	}
//	return false;
}

}} // otawa::oslice
