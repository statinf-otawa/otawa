#include <otawa/oslice/Slicer.h>

namespace otawa { namespace oslice {

Identifier<otawa::oslice::BBSet*> SetOfCallers("", 0);
//static Identifier<otawa::oslice::clp_value_set_t* > MEM_BB_END_IN("", NULL);
static Identifier<otawa::dfa::MemorySet::t* > MEM_BB_END_IN("", 0);
static Identifier<BitVector> REG_BB_END_IN("");

static Identifier<bool> TO_REMOVE("", false);
static Identifier<Vector<Block*>*> EDGE_TARGETS("");
static Identifier<Vector<Block*>*> EDGE_SOURCES("");

p::feature SLICER_FEATURE("otawa::oslice::SLICER_FEATURE", new Maker<Slicer>());

p::declare Slicer::reg = p::init("otawa::oslice::Slicer", Version(2, 0, 0))
       .maker<Slicer>()
       .require(LIVENESS_FEATURE)
	   .require(COLLECTED_CFG_FEATURE)
	   .invalidate(COLLECTED_CFG_FEATURE)
	   .provide(COLLECTED_CFG_FEATURE)
       .provide(SLICER_FEATURE)
	   ;

/*
 * The Cleaner class for COLLECTED_CFG_FEATURE
 */
class CollectedCFGCleaner: public Cleaner {
public:
	CollectedCFGCleaner(WorkSpace *_ws): ws(_ws) { }

protected:
	virtual void clean(void) {
		const CFGCollection* cfgc = INVOLVED_CFGS(ws);
		assert(cfgc);
		SLList<CFG*> cfgsToDelete;
		// collects the things to delete
		for(CFGCollection::Iterator cfgi(cfgc); cfgi; cfgi++) {
			cfgsToDelete.add(*cfgi); // collect the CFGs
		}
		// the removed of Blocks and Edges are handled by ~CFG()
		for(SLList<CFG*>::Iterator slli(cfgsToDelete); slli; slli++)
			delete *slli;
		// clean the Collection
		delete INVOLVED_CFGS(ws);
		INVOLVED_CFGS(ws).remove();
	}
private:
	WorkSpace* ws;
};


/**
 */
Slicer::Slicer(AbstractRegistration& _reg) : otawa::Processor(_reg) {
//	elm::log::Debug::setSourcePathLength(60);
//	elm::log::Debug::setDebugFlag(true);
//	elm::log::Debug::setSourceInfoFlag(true);
//#ifndef NO_COLOR
//	elm::log::Debug::setColorFlag(true);
//#else
//	elm::log::Debug::setColorFlag(false);
//#endif
//	elm::log::Debug::setPrefixColor(elm::color::IWhi);
	//_debugLevel |= DISPLAY_SLICING_STAGES;
}

/**
 */
void Slicer::configure(const PropList &props) {
	Processor::configure(props);
	slicingCFGOutputPath = SLICING_CFG_OUTPUT_PATH(props);
	slicedCFGOutputPath = SLICED_CFG_OUTPUT_PATH(props);
	_debugLevel = DEBUG_LEVEL(props);

}


void Slicer::processWorkSpace(WorkSpace *fw) {
	// obtain the collected CFG from the program, provided by the COLLECTED_CFG_FEATURE
	const CFGCollection& coll = **otawa::INVOLVED_CFGS(workspace());
	initIdentifiersForEachBB(coll);

	// will make use of osliceManager
	oslice::Manager osliceManager(workspace());

	// get a list of interested instruction
	interested_instructions_t *interestedInstructions = INTERESTED_INSTRUCTIONS(fw);
	if (interestedInstructions) {
		if(_debugLevel & DISPLAY_SLICING_STAGES)
			elm::cout << __SOURCE_INFO__<< "The list of interested instructions: " << io::endl;

		// now we look into each of these instructions
		for(interested_instructions_t::Iterator currentII(*interestedInstructions); currentII; currentII++) {
			if(_debugLevel & DISPLAY_SLICING_STAGES) {
				elm::cout << __SOURCE_INFO__ << "    " << currentII->getInst() << " @ " << currentII->getInst()->address() << " from BB " << currentII->getBB()->index() << io::endl;
				elm::cout << __SOURCE_INFO__ << "Popping interested instruction " << currentII->getInst() << " @ " << currentII->getInst()->address() << io::endl;
			}
			Inst* currentInst = currentII->getInst();
			BasicBlock* currentBB = currentII->getBB();
			SET_OF_REMAINED_INSTRUCTIONS(currentBB)->add(currentInst);

			// we know the BB, we know the instruction, then we can obtain its state from the oslice manager
			elm::BitVector workingRegs(workspace()->platform()->regCount(), false);
			LivenessChecker::provideRegisters(currentInst, workingRegs, 0);
			//clp_value_set_t workingMems;
			otawa::dfa::MemorySet::t workingMems = otawa::dfa::MemorySet::empty;
			int currentReadMemIndex = 0;
			LivenessChecker::getMems(currentBB, currentInst, currentReadMemIndex, workingMems, 0);

			if(_debugLevel & DISPLAY_SLICING_STAGES) {
				elm::cout << __SOURCE_INFO__ << "Creating the initial Regs from " << currentInst << " @ " << currentInst->address() << io::endl;
				elm::cout << __SOURCE_INFO__ << __TAB__ << "Working regs = " << workingRegs << io::endl;
				elm::cout << __SOURCE_INFO__ << __TAB__ << "Working mems = "; LivenessChecker::displayAddrs(elm::cout, workingMems); elm::cout << io::endl;
			}

			// define the working list of BB
			elm::genstruct::Vector<WorkingElement*> workingList;
			// first we put the current BB in to the list
			workingList.add(new WorkingElement(currentBB, currentInst, workingRegs, workingMems));
			processWorkingList(workingList);
		}
	}

	// now try to dump the CFG here
	DotDisplayer(workspace(), slicingCFGOutputPath, 1).display(coll);

	Vector<Block*> workingList;
	for (CFGCollection::Iterator c(coll); c; c++) {
		for (CFG::BlockIter v = c->blocks(); v; v++) {
			if (v->isBasic()) {
				BasicBlock *bb = v->toBasic();
				InstSet* setInst = SET_OF_REMAINED_INSTRUCTIONS(bb);
				// if all the instruction are sliced
				if (setInst->count() == 0) {
					if(_debugLevel & DISPLAY_CFG_CREATION)
						elm::cout << __SOURCE_INFO__<< "all instructions are sliced in BB" << bb->index() << " @ " << bb->address() << io::endl;
					// mark the BB sliced
					TO_REMOVE(bb) = true;
					workingList.add(bb);
				}
			}
		}
	} // finish putting the block to remove in the working list

	while(workingList.count()) {
		Block *b = workingList.pop();
		if(_debugLevel & DISPLAY_CFG_CREATION)
			elm::cout << __SOURCE_INFO__ << "Popping BB " << b->index() << " of CFG " << b->cfg()->index() << " from the BB-removing working list" << io::endl;

		Vector<Block*> predecessors;
		Vector<Block*> successors;
		for (Block::EdgeIter in = b->ins(); in; in++) { // just to be safe not to remove the element during iter ops.
			if(!TO_REMOVE(*in)) // only include the non-removed predecessor
				predecessors.add(in->source());
			TO_REMOVE(*in) = true;
			if(_debugLevel & DISPLAY_CFG_CREATION)
				elm::cout << __SOURCE_INFO__ << __TAB__ << "Removing an input edge " << *in << io::endl;
		}
		Vector<Block* > *edgeSources = EDGE_SOURCES(b);
		if(edgeSources) {
			for(Vector<Block*>::Iterator in(*edgeSources); in; in++) {
				if(_debugLevel & DISPLAY_CFG_CREATION)
					elm::cout << __SOURCE_INFO__ << __TAB__ << "Removing an input edge from BB " << in->index() << io::endl;
				predecessors.add(*in);
				Vector<Block* > *edgeTargets = EDGE_TARGETS(*in);
				if(edgeTargets)
					edgeTargets->remove(b);
			}
			delete edgeSources;
			EDGE_SOURCES(b).remove();
		}

		for (Block::EdgeIter out = b->outs(); out; out++) { // just to be safe not to remove the element during iter ops.
			if(!TO_REMOVE(*out))
				successors.add(out->sink());
			TO_REMOVE(*out) = true;
			if(_debugLevel & DISPLAY_CFG_CREATION)
				elm::cout << __SOURCE_INFO__ << __TAB__ << "Removing an output edge " << *out << io::endl;
		}
		Vector<Block* > *edgeTargets = EDGE_TARGETS(b);
		if(edgeTargets) {
			for(Vector<Block*>::Iterator out(*edgeTargets); out; out++) {
				if(_debugLevel & DISPLAY_CFG_CREATION)
					elm::cout << __SOURCE_INFO__ << __TAB__ << "Removing an output edge to BB " << out->index() << io::endl;
				successors.add(*out);
				Vector<Block* > *edgeSources = EDGE_SOURCES(*out);
				if(edgeSources)
					edgeSources->remove(b);
			}
			delete edgeTargets;
			EDGE_TARGETS(b).remove();
		}


		// removing all the artificial edges from its predecessor
		for (Vector<Block*>::Iterator predecessor(predecessors); predecessor; predecessor++) {
			Vector<Block* > *edgeTargets = EDGE_TARGETS(predecessor);
			if(!edgeTargets)
				continue;
			if(edgeTargets->contains(b)) {
				if(_debugLevel & DISPLAY_CFG_CREATION)
					elm::cout << __SOURCE_INFO__ << "predecessor BB " << predecessor->index() << " has a edge to current BB, removing...." << io::endl;
				edgeTargets->remove(b);
			}
		}
		// removing all the artificial edges from its successors
		for (Vector<Block*>::Iterator successor(successors); successor; successor++) {
			Vector<Block* > *edgeSources = EDGE_SOURCES(successor);
			if(!edgeSources)
				continue;
			if(edgeSources->contains(b)) {
				if(_debugLevel & DISPLAY_CFG_CREATION)
					elm::cout << __SOURCE_INFO__ << "successor BB " << successor->index() << " has a edge to current BB, removing...." << io::endl;
				edgeSources->remove(b);
			}
		}

		 // for each predecessor, need to wire the edge between the predecessor and its successor
		for (Vector<Block*>::Iterator predecessor(predecessors); predecessor; predecessor++) {
			for (Vector<Block*>::Iterator successor(successors); successor; successor++) {
				// check if the successor is already linked with the predecessor
				// first check the real link
				bool found = false;
				for(Block::EdgeIter ei = predecessor->outs(); ei; ei++) {
					if(ei->target() == *successor) {
						found = true;
						break;
					}
				}
				Vector<Block* > *edgeTargets = EDGE_TARGETS(predecessor);
				if(edgeTargets && edgeTargets->contains(*successor))
					found = true;
				if(found) {
					if(_debugLevel & DISPLAY_CFG_CREATION)
						elm::cout << __SOURCE_INFO__ <<__TAB__ << "Already existing an edge between BB " << predecessor->index() << " to BB " << successor->index() << io::endl;
				}
				else {
					if(_debugLevel & DISPLAY_CFG_CREATION)
						elm::cout << __SOURCE_INFO__ << __TAB__ << "Adding edge between BB " << predecessor->index() << " to BB " << successor->index() << io::endl;
					Vector<Block* > *edgeTargets = EDGE_TARGETS(predecessor);
					if(!edgeTargets) {
						edgeTargets = new Vector<Block* >();
						EDGE_TARGETS(predecessor) = edgeTargets;
					}
					edgeTargets->add(successor);
					Vector<Block* > *edgeSources = EDGE_SOURCES(successor);
					if(!edgeSources) {
						edgeSources = new Vector<Block* >();
						EDGE_SOURCES(successor) = edgeSources;
					}
					edgeSources->add(predecessor);
				}
			}
		} // finish linking the predecessors and successors of the removal BB
	} // end of the working list


	for(CFGCollection::Iterator c(coll); c; c++) {
		makeCFG(c);
	}

	sliced_coll = new CFGCollection();
	for(genstruct::FragTable<CFGMaker *>::Iterator m(makers); m; m++) {
	        sliced_coll->add(m->build());
	        delete *m;
	}

	DotDisplayer(workspace(), slicedCFGOutputPath, 0).display(*sliced_coll);

} // end of function Slicer::work

/**
 * Build the virtual CFG.
 * @param cfg          CFG to inline into.
 */
void Slicer::make(CFG *cfg, CFGMaker& maker) {
	ASSERT(cfg);
	genstruct::HashTable<Block *, Block *> bmap;
	Vector<Block*> workingList;
	Vector<Block*> blocksToRemove;

	// add initial blocks
	bmap.put(cfg->entry(), maker.entry());
	bmap.put(cfg->exit(), maker.exit());
	if(cfg->unknown())
		 bmap.put(cfg->unknown(), maker.unknown());

	// add other blocks
	for(CFG::BlockIter v = cfg->blocks(); v; v++) {
		 if(v->isEnd())
			  continue;
		 else if(v->isBasic()) {
			  BasicBlock *bb = v->toBasic();
			  if(TO_REMOVE(bb))
				  continue;

			  InstSet* setInst = SET_OF_REMAINED_INSTRUCTIONS(bb);
			  genstruct::Vector<Inst *> insts(setInst->count());

			  for(BasicBlock::InstIter i = bb->insts(); i; i++) {
				   if(setInst->contains(i))
					    insts.add(i);
			  }

			  BasicBlock *nv = new BasicBlock(insts.detach());
			  maker.add(nv);
			  bmap.add(*v, nv); // * operator on iterator gives the pointer to the item, which is a pointer to Block
		 }
		 // process synthetic block
		 else {
			  // build synth block
			  SynthBlock *sb = v->toSynth();
			  SynthBlock *nsb = new SynthBlock();
			  bmap.put(sb, nsb);

			  // link with callee
			  if(!sb->callee())
				   maker.add(nsb);
			  else {
				   CFGMaker& callee = makerOf(sb->callee());
				   maker.call(nsb, callee);
			  }
		 }
	}

	// add edges
	for(CFG::BlockIter v = cfg->blocks(); v; v++) {
		if (_debugLevel & DISPLAY_CFG_CREATION)
			elm::cout << __SOURCE_INFO__<< "looking at the BB " << v->index() << " of CFG " << cfg->index() << io::endl;

		if(TO_REMOVE(*v))
		{
			if(_debugLevel & DISPLAY_CFG_CREATION)
			elm::cout << __SOURCE_INFO__ << __TAB__ << "this node is removed, ignored" << io::endl;
			TO_REMOVE(*v).remove();
			Vector<Block* > *edgeTargets = EDGE_TARGETS(*v);
			if(edgeTargets)
			delete edgeTargets;
			EDGE_TARGETS(*v).remove();
			Vector<Block* > *edgeSources = EDGE_SOURCES(*v);
			if(edgeSources)
			delete edgeSources;
			EDGE_SOURCES(*v).remove();
			continue;
		}

		for(BasicBlock::EdgeIter e = v->outs(); e; e++) {
			if(_debugLevel & DISPLAY_CFG_CREATION)
			elm::cout << __SOURCE_INFO__ << __TAB__ << e->source()->index() << " to " << e->sink()->index() << io::endl;
			if(TO_REMOVE(*e)) {
				if(_debugLevel & DISPLAY_CFG_CREATION)
					elm::cout << __SOURCE_INFO__ << __TAB__ << __TAB__ << "this edge is removed, ignored" << io::endl;
				TO_REMOVE(*v).remove();
				continue;
			}
			maker.add(bmap.get(e->source()), bmap.get(e->sink()), new Edge());
		} // end for(BasicBlock::EdgeIter e = v->outs(); e; e++) {

		 // linking the artificial edge
		Vector<Block* > *edgeTargets = EDGE_TARGETS(*v);
		if(edgeTargets) {
			for(Vector<Block* >::Iterator i(*edgeTargets); i; i++) {
				if(_debugLevel & DISPLAY_CFG_CREATION) {
					elm::cout << __SOURCE_INFO__ << __TAB__ << v->index() << " to " << i->index() << io::endl;
					elm::cout << __SOURCE_INFO__ << __TAB__ << __TAB__ << "this edge is created due to BB removal" << io::endl;
				}
				maker.add(bmap.get(*v), bmap.get(*i), new Edge());
			}
			delete edgeTargets;
		}
		EDGE_TARGETS(*v).remove();
		Vector<Block* > *edgeSources = EDGE_SOURCES(*v);
		if(edgeSources)
		delete edgeSources;
		EDGE_SOURCES(*v).remove();
		continue;
	}
}

/**
 * Virtualize a CFG and add it to the cfg map.
 * @param call Call string.
 * @param cfg  CFG to virtualize.
 */
void Slicer::makeCFG(CFG *cfg) {
	// now we need to insert the content of our new cfg (created by makerOf(cfg))
	make(cfg, makerOf(cfg));
}

/**
 * Obtain the maker for a particular CFG.
 * @param cfg  CFG to look a maker for.
 * @return	      Associated CFG maker.
 */
CFGMaker& Slicer::makerOf(CFG *cfg) {
	CFGMaker *r = map.get(cfg, 0);
	if(!r) {
		 r = &newMaker(cfg->first());
		 map.put(cfg, r);
	}
	return *r;
}

/**
 * Build a new maker for a CFG (an inlined CFG).
 * @param      First instruction of CFG.
 * @return     Built maker.
 */
CFGMaker& Slicer::newMaker(Inst *first) {
	CFGMaker *m = new CFGMaker(first);
	makers.add(m);
	return *m;
}

// this function is called at the end of the processor
// the main idea is to replace the INVOLVED_CFGS of the workspace
// by the new one which is the CFG_original \ sliced_instructions
void Slicer::cleanup(WorkSpace *ws) {
	assert(sliced_coll);
	ENTRY_CFG(ws) = (*sliced_coll)[0];
	INVOLVED_CFGS(ws) = sliced_coll;
	addCleaner(COLLECTED_CFG_FEATURE, new CollectedCFGCleaner(ws));
	//addCleaner(SLICER_FEATURE, new CollectedCFGCleaner(ws));
	//SLICER_FEATURE
}

void Slicer::initIdentifiersForEachBB(const CFGCollection& coll) {
	// for each CFG
	for (int i = 0; i < coll.count(); i++) {
		CFG *cfg = coll[i]; // current CFG
		// for each BB in the CFG
		for (CFG::BlockIter v = cfg->blocks(); v; v++) {
			if(!v->isBasic())
				continue;
			SET_OF_REMAINED_INSTRUCTIONS(*v) = new InstSet();
			REG_BB_END_IN(*v) = BitVector(workspace()->platform()->regCount(), false);
			// MEM_BB_END_IN(*v) = new clp_value_set_t();
			MEM_BB_END_IN(*v) = new otawa::dfa::MemorySet::t(0);
		} // end for (CFG::BlockIter v = cfg->blocks(); v; v++) {
	} // end for (int i = 0; i < coll.count(); i++) {
}

void Slicer::processWorkingList(elm::genstruct::Vector<WorkingElement*>& workingList) {
	// while the list is not empty
	while(workingList.count())
	{
		// pop the first element to process
		WorkingElement* we = workingList.pop();
		BasicBlock* currentBB_wl = we->_bb;
		elm::BitVector currentRegs_wl = we->_workingRegs;
		// load the memory access set
		//clp_value_set_t currentMems_wl(we->_workingMems);
		otawa::dfa::MemorySet::t currentMems_wl(we->_workingMems);
		Inst* currentInst_wl = currentBB_wl->last();
		if(we->_inst != currentBB_wl->last())
			currentInst_wl = we->_inst;

		if(_debugLevel & DISPLAY_SLICING_STAGES) {
			elm::cout << __SOURCE_INFO__ << __RED__ << "Popping new working element out: BB " << currentBB_wl->index() << " @ " <<  currentBB_wl->address() << __RESET__ << io::endl;
			elm::cout << __SOURCE_INFO__ << __TAB__ << "Starting witt " << currentInst_wl << " @ " << currentInst_wl->address() << io::endl;
			elm::cout << __SOURCE_INFO__ << __TAB__ << "with working Regs  = " << currentRegs_wl << io::endl;
			elm::cout << __SOURCE_INFO__ << __TAB__ << "with working MEM   = "; LivenessChecker::displayAddrs(elm::cout, currentMems_wl); elm::cout << io::endl;
		}
		delete we;

		int currentReadMemIndex = 0;
		int currentWriteMemIndex = 0;

		bool beginingOfCurrentBB_wl = false;
		while(!beginingOfCurrentBB_wl)
		{
			if(_debugLevel & DISPLAY_SLICING_STAGES)
				elm::cout << __SOURCE_INFO__ << __YELLOW__ << "Processing " << currentInst_wl << " @ " << currentInst_wl->address() << __RESET__ << io::endl;
			elm::BitVector currentRegsDef(workspace()->platform()->regCount(), false);
			elm::BitVector currentRegsUse(workspace()->platform()->regCount(), false);
			LivenessChecker::provideRegisters(currentInst_wl, currentRegsUse, 0);
			LivenessChecker::provideRegisters(currentInst_wl, currentRegsDef, 1);

			// for memory access
			//clp_value_set_t addressInstRead, addressInstWrite;
			otawa::dfa::MemorySet::t addressInstRead(0), addressInstWrite(0);
			LivenessChecker::getMems(currentBB_wl, currentInst_wl, currentReadMemIndex, addressInstRead, 0);
			LivenessChecker::getMems(currentBB_wl, currentInst_wl, currentWriteMemIndex, addressInstWrite, 1);

			if(_debugLevel & DISPLAY_SLICING_STAGES) {
				elm::cout << __SOURCE_INFO__ << __TAB__ << "Reg Def       = " << currentRegsDef << io::endl;
				elm::cout << __SOURCE_INFO__ << __TAB__ << "Reg Use       = " << currentRegsUse << io::endl;
				elm::cout << __SOURCE_INFO__ << __TAB__ << "Mem Def       = "; LivenessChecker::displayAddrs(elm::cout, addressInstWrite); elm::cout << io::endl;
				elm::cout << __SOURCE_INFO__ << __TAB__ << "Mem Use       = "; LivenessChecker::displayAddrs(elm::cout, addressInstRead); elm::cout << io::endl;
				elm::cout << __SOURCE_INFO__ << __TAB__ << "RegSet Before = " << currentRegs_wl << io::endl;
				elm::cout << __SOURCE_INFO__ << __TAB__ << "MemSet Before = "; LivenessChecker::displayAddrs(elm::cout, currentMems_wl); elm::cout << io::endl;
			}

			// check if the def address is over lap with the working addrs
			bool memInterested = interestingAddrs(currentMems_wl, addressInstWrite);

			// if working Regs & def Regs is not zero, that means this instruction provides
			// the registers that we are interested
			bool regInsterested = interestingRegs(currentRegs_wl, currentRegsDef); // !(currentRegs_wl & currentRegsDef).isEmpty();

			if(memInterested | regInsterested) {

				if(_debugLevel & DISPLAY_SLICING_STAGES)
					elm::cout << __SOURCE_INFO__ << __GREEN__ << __TAB__ << "We are interested in instruction " << currentInst_wl << __RESET__ << io::endl;

				// update the working Regs
				currentRegs_wl = (currentRegs_wl - currentRegsDef) | currentRegsUse;
				// update the current working memory
				LivenessChecker::updateAddrsFromInstruction(currentMems_wl, addressInstRead, addressInstWrite, _debugLevel);
				// do something with the instruction
				SET_OF_REMAINED_INSTRUCTIONS(currentBB_wl)->add(currentInst_wl);
			}
			else
				if(_debugLevel & DISPLAY_SLICING_STAGES)
					elm::cout << __SOURCE_INFO__ << __RED__ << __TAB__ << "We slice away instruction " << currentInst_wl << __RESET__ << io::endl;

			if(_debugLevel & DISPLAY_SLICING_STAGES) {
				elm::cout << __SOURCE_INFO__ << __TAB__ << "RegSet After  = " << currentRegs_wl << io::endl;
				elm::cout << __SOURCE_INFO__ << __TAB__ << "MemSet After  = "; LivenessChecker::displayAddrs(elm::cout, currentMems_wl); elm::cout << io::endl;
			}

			if(currentInst_wl == currentBB_wl->first())
				beginingOfCurrentBB_wl = true;
			else
				currentInst_wl = currentInst_wl->prevInst();
		} // reaches the beginning of the BB

		// here reaches the beginning of the BB, now we need to list the list of incoming edges
		// so we can keep trace back the previous BB
		// first we find the predecessors of the BB to process
		elm::genstruct::Vector<BasicBlock *> predecessors;

		for (Block::EdgeIter e = currentBB_wl->ins(); e; e++) {
			Block* b = e->source(); // find the source of the edge, the predecessor of current BB
			if (b->isEntry()) {
				BBSet* callers = SetOfCallers(b);
				if(!callers)
					continue; // means we reach the program entry

				for(BBSet::Iterator caller(*callers); caller ; ++caller) {
					if(_debugLevel & DISPLAY_SLICING_STAGES)
						elm::cout << __SOURCE_INFO__ << "Found a caller @ " << caller->address() << io::endl;
					// we now add the caller BB
					predecessors.add(caller);
				}
			} // end of handling the entry block
			else if (b->isBasic()) {
				// we put the BB here into the working list with given
				if(_debugLevel & DISPLAY_SLICING_STAGES)
					elm::cout << __SOURCE_INFO__ << "Found predecessor BB @ " << b->toBasic()->address() << io::endl;
				predecessors.add(b->toBasic());
			} // end of handling the basic block
			else if (b->isSynth()) {
				// this means that we reach current BB from the returning of a function
				// we obtain the exit block of the function that it returns from
				if(_debugLevel & DISPLAY_SLICING_STAGES) {
					elm::cout << "Caller of the current Synth Block = " << b->toSynth()->caller()->label() << io::endl;
					elm::cout << "Callee of the current Synth Block = " << b->toSynth()->callee()->label() << io::endl;
				}

				Block* end = b->toSynth()->callee()->exit();
				// each edge to the exit block is a possible BB which will goes to the current block
				for (Block::EdgeIter EdgeToExit = end->ins(); EdgeToExit; EdgeToExit++) {
					BasicBlock* BB_BeforeReturn = EdgeToExit->source()->toBasic();
					if(_debugLevel & DISPLAY_SLICING_STAGES)
						elm::cout << __SOURCE_INFO__ << "Found a callee @ " << BB_BeforeReturn->address() << io::endl;
					predecessors.add(BB_BeforeReturn);
				}

			} // end of handling the synth block
			else {
				if (b->isEntry())
					elm::cout << "ENTRY" << io::endl;
				else if (b->isExit())
					elm::cout << "EXIT" << io::endl;
				else if (b->isUnknown())
					elm::cout << "unknown" << io::endl;
				ASSERTP(false, "Encounter an unexpected block");
			}
		} // end of finding predecessors of the current BB







		if(predecessors.count() == 0)
			if(_debugLevel & DISPLAY_SLICING_STAGES)
				elm::cout << __SOURCE_INFO__ << __RED__ << "No predecessor for BB @ " << currentBB_wl->address() << __RESET__ << io::endl;

		// process the collected BBs
		// now we need to see if the input (register and memory uses) feed from the successor matches totally or a subset of the pred BB
		for(elm::genstruct::Vector<BasicBlock *>::Iterator predecessor(predecessors); predecessor; ++predecessor) {
			BitVector bv = REG_BB_END_IN(predecessor);
			//clp_value_set_t* memIn = MEM_BB_END_IN(predecessor);
			otawa::dfa::MemorySet::t *memIn = MEM_BB_END_IN(predecessor);

			bool notContainsAllRegs = !bv.includes(currentRegs_wl);
			bool notContainsAllMems = !LivenessChecker::containsAllAddrs(*memIn, currentMems_wl);

			if(_debugLevel & DISPLAY_SLICING_STAGES) {
				elm::cout << __SOURCE_INFO__ << __TAB__ << "for BB @ " << predecessor->address() << io::endl;
				elm::cout << __SOURCE_INFO__ << __TAB__ << __TAB__ << "Register used: " << bv << " contains all of " << currentRegs_wl << " ? "<< !notContainsAllRegs << io::endl;
				elm::cout << __SOURCE_INFO__ << __TAB__ << __TAB__ << "Mem      used: "; LivenessChecker::displayAddrs(elm::cout, *memIn); elm::cout << " contains all of "; LivenessChecker::displayAddrs(elm::cout, currentMems_wl); elm::cout << "? " << !notContainsAllMems << io::endl;
			}

			if(notContainsAllRegs | notContainsAllMems) {
				if(_debugLevel & DISPLAY_SLICING_STAGES)
					elm::cout << __SOURCE_INFO__ << __RED__ << "Adding BB @ " << predecessor->address() << " to the working list." << __RESET__ << io::endl;
				bv = bv | currentRegs_wl;
				REG_BB_END_IN(predecessor) = bv;
				//memIn->addAll(currentMems_wl);
				*memIn = dfa::MemorySet().join(*memIn, currentMems_wl);
				MEM_BB_END_IN(predecessor) = memIn;

				WorkingElement *we = new WorkingElement(predecessor, predecessor->last(), currentRegs_wl, currentMems_wl);
				workingList.add(we);
			}
			else
				if(_debugLevel & DISPLAY_SLICING_STAGES)
					elm::cout << __SOURCE_INFO__ << __RED__ << "NOT adding BB @ " << predecessor->address() << " to the working list." << __RESET__ << io::endl;
		}

	} // end of the current working list
}

// check if any element in b is in a
//inline bool Slicer::interestingAddrs(clp_value_set_t const & a, clp_value_set_t const & b) {
inline bool Slicer::interestingAddrs(otawa::dfa::MemorySet::t const & a, otawa::dfa::MemorySet::t const & b) {
	dfa::MemorySet ms;
//	for(clp_value_set_t::Iterator addr(b); addr; ++addr)
//		if(a.contains(*addr))
//			return true;
//
//	return false;
//	for(dfa::MemorySet::Iter msi = b.areas(); msi; msi++) {
//
//		elm::cout << "a = " << a << ", msi = " << *msi << io::endl;
//
//		if(ms.contains(a, msi)) {
//			return true;
//		}
//	}
//
//	for(dfa::MemorySet::Iter msi = a.areas(); msi; msi++) {
//
//		elm::cout << "b = " << b << ", msi = " << *msi << io::endl;
//
//		if(ms.contains(b, msi)) {
//			return true;
//		}
//	}

	dfa::MemorySet::t intersection = ms.meet(a, b);
	if(intersection == dfa::MemorySet::empty)
		return false;


	return true;
}

inline bool Slicer::interestingRegs(elm::BitVector const & a, elm::BitVector const & b) {
	return !((a & b).isEmpty());
}

} }
