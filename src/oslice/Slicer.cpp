/*
 *	Slicer class implementation
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
#include <elm/sys/System.h>

#include <otawa/oslice/Slicer.h>
#include <otawa/program.h>
#include "../../include/otawa/display/CFGDecorator.h"

namespace otawa { namespace oslice {


static Identifier<otawa::dfa::MemorySet::t* > SLICER_MEM_BB_END("otawa::oslice::SLICER_MEM_BB_END", 0);
static Identifier<BitVector> SLICER_REG_BB_END("otawa::oslice::SLICER_REG_BB_END");

static Identifier<bool> TO_REMOVE("", false);
typedef Pair<Block*, t::uint32> predecessor_t;
typedef Vector<predecessor_t> predecessor_list_t;
static Identifier<Vector<Pair<Block*, t::uint32 /* flag of the edge */ > >*> ARTIFICIAL_PREDECESSORS("");
static Identifier<Vector<Block*>*> ARTIFICIAL_SUCCESSORS("");


p::declare Slicer::reg = p::init("otawa::oslice::Slicer", Version(16, 5, 3116))
       .maker<Slicer>()
	   .use(otawa::clp::CLP_ANALYSIS_FEATURE)
	   .use(INSTRUCTION_COLLECTOR_FEATURE)
	   .require(COLLECTED_CFG_FEATURE)
	   .invalidate(COLLECTED_CFG_FEATURE)
	   .provide(COLLECTED_CFG_FEATURE)
       .provide(SLICER_FEATURE);
p::feature SLICER_FEATURE("otawa::oslice::SLICER_FEATURE", new Maker<Slicer>());

p::declare LightSlicer::reg = p::init("otawa::oslice::LightSlicer", Version(16, 5, 3116))
       .maker<LightSlicer>()
	   .use(INSTRUCTION_COLLECTOR_FEATURE)
	   .require(COLLECTED_CFG_FEATURE)
	   .invalidate(COLLECTED_CFG_FEATURE)
	   .provide(COLLECTED_CFG_FEATURE)
       .provide(SLICER_FEATURE);
p::feature LIGHT_SLICER_FEATURE("otawa::oslice::LIGHT_SLICER_FEATURE", new Maker<LightSlicer>());


class SlicingDecorator: public display::CFGDecorator {
public:
	SlicingDecorator(WorkSpace *ws): display::CFGDecorator(ws), showSlicing(0) { }
	SlicingDecorator(WorkSpace *ws, int _sl): display::CFGDecorator(ws), showSlicing(_sl) { }
protected:
	virtual void displaySynthBlock(CFG *g, SynthBlock *b, display::Text& content, display::VertexStyle& style) const {
		display::CFGDecorator::displaySynthBlock(g, b, content, style);
		if(b->callee()) {
			if(!b->callee()->index())
				content.setURL("index.dot");
			else
				content.setURL(_ << b->callee()->index() << "_" << b->callee()->name() << ".dot");
		}
	}

	virtual void displayEndBlock(CFG *graph, Block *block, display::Text& content, display::VertexStyle& style) const {
		CFGDecorator::displayEndBlock(graph, block, content, style);
		content.setURL("");
	}

	virtual void displayBasicBlock(CFG *graph, BasicBlock *block, display::Text& content, display::VertexStyle& style) const {
		CFGDecorator::displayBasicBlock(graph, block, content, style);
		content.setURL("");
	}

	virtual void displayAssembly(CFG *graph, BasicBlock *block, display::Text& content) const {
		cstring file;
		int line = 0;

		InstSet* setInst = SET_OF_REMAINED_INSTRUCTIONS(block);

		for(BasicBlock::InstIter i = block->insts(); i(); i++) {
			// display source line
			if(display_source_line) {
				Option<Pair<cstring, int> > src = workspace()->process()->getSourceLine(i->address());
				if(src && ((*src).fst != file || (*src).snd != line)) {
					file = (*src).fst;
					line = (*src).snd;
					content << display::begin(source_color) << display::begin(display::ITALIC) << file << ":" << line << display::end(display::ITALIC) << display::end(source_color)
							<< display::left;
				}
			}
			// display labels
			for(Identifier<Symbol *>::Getter l(*i, SYMBOL); l(); l++)
				content << display::begin(label_color) << l->name() << ":" << display::end(label_color)
						<< display::left;
			// adding the color indicating the instruction is sliced away
			if(showSlicing && !setInst->contains(*i))
				content << display::begin(display::Color(255,0,0));
			// display instruction
			content << ot::address(i->address()) << "  " << *i;
			// end adding the color indicating the instruction is sliced away
			if(showSlicing && !setInst->contains(*i))
				content << display::end(display::Color(255,0,255));

			content << display::left;
		} // end for each instructions
	}

	virtual void displayProps(CFG *g, BasicBlock *b, display::Text& content) const {
		for(PropList::Iter p(b); p(); p++)
			if(p->id()->name())
				content << display::begin(display::BOLD) << p->id()->name() << display::begin(display::BOLD)
						<< "\t!!!!!!" << *p;
	}
private:
	int showSlicing;
};

/*
 * The Cleaner class for COLLECTED_CFG_FEATURE
 */
#if 0
class CollectedCFGCleaner: public Cleaner {
public:
	CollectedCFGCleaner(WorkSpace *_ws): ws(_ws) { }

protected:
	virtual void clean(void) {
		const CFGCollection* cfgc = INVOLVED_CFGS(ws);
		ASSERT(cfgc);
		List<CFG*> cfgsToDelete;
		// collects the things to delete
		for(CFGCollection::Iter cfgi(cfgc); cfgi; cfgi++) {
			cfgsToDelete.add(*cfgi); // collect the CFGs
		}
		// the removed of Blocks and Edges are handled by ~CFG()
		for(List<CFG*>::Iter slli(cfgsToDelete); slli; slli++)
			delete *slli;
		// clean the Collection
		delete INVOLVED_CFGS(ws);
		INVOLVED_CFGS(ws).remove();
	}
private:
	WorkSpace* ws;
};
#endif


/**
 */
Slicer::Slicer(AbstractRegistration& _reg)
	:	otawa::Processor(_reg),
		sliced_coll(nullptr),
		_debugLevel(0),
	    _outputCFG( false),
	    _lightSlicing(false)
{ }


/**
 */
void Slicer::configure(const PropList &props) {
	Processor::configure(props);
	_slicingCFGOutputPath = SLICING_CFG_OUTPUT_PATH(props);
	_slicedCFGOutputPath = SLICED_CFG_OUTPUT_PATH(props);
	_debugLevel = SLICE_DEBUG_LEVEL(props);
	_outputCFG = CFG_OUTPUT(props);
	//_lightSlicing = ENABLE_LIGHT_SLICING(props);
}


///
void *Slicer::interfaceFor(const AbstractFeature& f) {
	return sliced_coll;
}


void Slicer::processWorkSpace(WorkSpace *fw) {
	// obtain the collected CFG from the program, provided by the COLLECTED_CFG_FEATURE
	const CFGCollection& coll = **otawa::INVOLVED_CFGS(workspace());

	// compute how many instructions before slicing
	{
		int sum = 0;
		int sumCFG = 0;
		int sumB = 0;
		const CFGCollection* cfgc = INVOLVED_CFGS(workspace());
		for(CFGCollection::Iter cfg(cfgc); cfg(); cfg++) {
			sumCFG++;
			for(CFG::BlockIter bi = cfg->blocks(); bi(); bi++) {
				sumB++;
				if(bi->isBasic())
					sum = sum + bi->toBasic()->count();
				else
					continue;
			} // for each BB
		} // for each CFG
		warn(String(" Before slicing: ") << sumCFG << " CFGs, " << sumB << " Blocks, " << sum << " instructions");
	}

	// start measuring the time taken by the slicer
//	clock_t clockWorkSpace;
//	clockWorkSpace = clock();
//	system::StopWatch watchWorkSpace;
//	watchWorkSpace.start();

	if(!_lightSlicing && !LivenessChecker::clpManager)
		LivenessChecker::clpManager = new clp::Manager(workspace());

	initIdentifiersForEachBB(coll);

	//LivenessChecker::buildReverseSynthLink(coll);

	// get a list of interested instruction
	interested_instructions_t *interestedInstructions = INTERESTED_INSTRUCTIONS(fw);
	ASSERT(interestedInstructions);
	if (interestedInstructions) {
		warn(String(" ") << interestedInstructions->count() << " instructions to resolve");
		if(_debugLevel & DISPLAY_SLICING_STAGES) {
			elm::cerr << __SOURCE_INFO__<< "The list of interested instructions: " << io::endl;
			for(interested_instructions_t::Iter currentII(*interestedInstructions); currentII(); currentII++) {
				elm::cerr << __SOURCE_INFO__ << "    " << currentII->getInst() << " @ " << currentII->getInst()->address() << " from BB " << currentII->getBB()->index() << io::endl;
			}
		}

		// now we look into each of these instructions
		for(interested_instructions_t::Iter currentII(*interestedInstructions); currentII(); currentII++) {
			if(_debugLevel & DISPLAY_SLICING_STAGES) {
				elm::cerr << __SOURCE_INFO__ << "Popping interested instruction " << currentII->getInst() << " @ " << currentII->getInst()->address() << io::endl;
			}
			Inst* currentInst = currentII->getInst();
			BasicBlock* currentBB = currentII->getBB();
			SET_OF_REMAINED_INSTRUCTIONS(currentBB)->add(currentInst);

			// we know the BB, we know the instruction, then we can obtain its state from the oslice manager
			elm::BitVector workingRegs(workspace()->platform()->regCount(), false);
			LivenessChecker::provideRegisters(currentInst, workingRegs, 0);

			otawa::dfa::MemorySet::t workingMems = otawa::dfa::MemorySet::empty;
			if(!_lightSlicing) {
				LivenessChecker::identifyAddrs(currentBB);
				int currentReadMemIndex = 0;
				LivenessChecker::getMems(currentBB, currentInst, currentReadMemIndex, workingMems, 0);
			}

			if(_debugLevel & DISPLAY_SLICING_STAGES) {
				elm::cerr << __SOURCE_INFO__ << "Creating the initial Regs from " << currentInst << " @ " << currentInst->address() << io::endl;
				elm::cerr << __SOURCE_INFO__ << __TAB__ << "Working regs = " << workingRegs << io::endl;
				if(!_lightSlicing) {
					elm::cerr << __SOURCE_INFO__ << __TAB__ << "Working mems = ";
					otawa::oslice::displayAddrs(elm::cerr, workingMems);
					elm::cerr << io::endl;
				}
			}

			// define the working list of BB
			elm::Vector<WorkingElement*> workingList;
			// first we put the current BB in to the list
			workingList.add(new WorkingElement(currentBB, currentInst, workingRegs, workingMems));
			processWorkingList(workingList);
		}
	}

	// now try to dump the CFG here
	if(_outputCFG) {
		for(CFGCollection::Iter cfg(coll); cfg(); cfg++) {
			// full program will use different abstract graph
			// DotDisplayer(workspace(), slicingCFGOutputPath, 1).display(coll);
			//display::DisplayedCFG ag(**cfg);
			SlicingDecorator d(workspace(), 1);
			display::Displayer *disp = display::Provider::display(*cfg, d, display::OUTPUT_RAW_DOT);
			// set up the path
			Path dir;
			if(_slicingCFGOutputPath.length() == 0)
				dir = Path("slicing");
			else
				dir = Path(_slicingCFGOutputPath);

			if(!dir.exists())
				sys::System::makeDir(dir);

			if(cfg->index() == 0)
				disp->setPath(dir / "index.dot");
			else
				disp->setPath(dir / string(_ << cfg->index() << "_" << cfg->name() << ".dot"));
			disp->process();
			delete disp;
		}
	}

//	clock_t clockWorkCFGReconstruction;
//	clockWorkCFGReconstruction = clock();
//	system::StopWatch watchWorkCFGReconstruction;
//	watchWorkCFGReconstruction.start();

	slicing();

//	clockWorkCFGReconstruction = clock() - clockWorkCFGReconstruction;
//	elm::cerr << "CFG SLI takes " << clockWorkCFGReconstruction << " micro-seconds" << io::endl;

	if(_outputCFG) {
		for(CFGCollection::Iter cfg(sliced_coll); cfg(); cfg++) {
			//display::DisplayedCFG ag(**cfg);
			SlicingDecorator d(workspace(), 0);
			display::Displayer *disp = display::Provider::display(*cfg, d, display::OUTPUT_RAW_DOT);
			// set up the path
			Path dir;
			if(_slicedCFGOutputPath.length() == 0)
				dir = Path("sliced");
			else
				dir = Path(_slicedCFGOutputPath);

			if(!dir.exists())
				sys::System::makeDir(dir);

			if(cfg->index() == 0)
				disp->setPath(dir / "index.dot");
			else
				disp->setPath(dir / string(_ << cfg->index() << "_" << cfg->name() << ".dot"));
			disp->process();
			delete disp;
		}
	}


//	clockWorkSpace = clock() - clockWorkSpace;
//	watchWorkCFGReconstruction.stop();
//	otawa::ot::time t2 = watchWorkCFGReconstruction.delay();
//	elm::cerr << "OSlicer takes " << clockWorkSpace << " micro-seconds" << io::endl;

	// compute how many instructions after slicing
	{
		int sum = 0;
		int sumCFG = 0;
		int sumB = 0;
		for(CFGCollection::Iter cfg(sliced_coll); cfg(); cfg++) {
			sumCFG++;
			for(CFG::BlockIter bi = cfg->blocks(); bi(); bi++) {
				sumB++;
				if(bi->isBasic())
					sum = sum + bi->toBasic()->count();
				else
					continue;
			} // for each BB
		} // for each CFG
		warn(String(" After slicing: ") << sumCFG << " CFGs, " << sumB << " Blocks, " << sum << " instructions");
	}

} // end of function Slicer::work

/**
 * Build the virtual CFG.
 * @param cfg          CFG to inline into.
 */
void Slicer::make(CFG *cfg, CFGMaker& maker) {
	ASSERT(cfg);
	HashMap<Block *, Block *> bmap;
	Vector<Block*> workingList;
	Vector<Block*> blocksToRemove;

	// add initial blocks
	bmap.put(cfg->entry(), maker.entry());
	bmap.put(cfg->exit(), maker.exit());
	if(cfg->unknown())
		 bmap.put(cfg->unknown(), maker.unknown());

	// add other blocks
	for(CFG::BlockIter v = cfg->blocks(); v(); v++) {
		 if(v->isEnd())
			  continue;
		 else if(v->isBasic()) {
			  BasicBlock *bb = v->toBasic();
			  if(TO_REMOVE(bb))
				  continue;

			  InstSet* setInst = SET_OF_REMAINED_INSTRUCTIONS(bb);
			  Vector<Inst *> insts(setInst->count());
			  
			  // only add the non-sliced instruction to the vector insts
			  for(BasicBlock::InstIter i = bb->insts(); i(); i++) {
				   if(setInst->contains(*i))
					    insts.add(*i);
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
	for(CFG::BlockIter v = cfg->blocks(); v(); v++) {
		if (_debugLevel & DISPLAY_CFG_CREATION)
			elm::cerr << __SOURCE_INFO__<< "looking at the BB " << v->index() << " of CFG " << cfg->index() << io::endl;

		if(TO_REMOVE(*v))
		{
			if(_debugLevel & DISPLAY_CFG_CREATION)
			elm::cerr << __SOURCE_INFO__ << __TAB__ << "this node is removed, ignored" << io::endl;
			TO_REMOVE(*v).remove();
			Vector<Block* > *edgeTargets = ARTIFICIAL_SUCCESSORS(*v);
			if(edgeTargets)
			delete edgeTargets;
			ARTIFICIAL_SUCCESSORS(*v).remove();
			predecessor_list_t *edgeSources = ARTIFICIAL_PREDECESSORS(*v);
			if(edgeSources)
			delete edgeSources;
			ARTIFICIAL_PREDECESSORS(*v).remove();
			continue;
		}

		for(BasicBlock::EdgeIter e = v->outs(); e(); e++) {
			if(_debugLevel & DISPLAY_CFG_CREATION)
			elm::cerr << __SOURCE_INFO__ << __TAB__ << e->source()->index() << " to " << e->sink()->index() << io::endl;
			if(TO_REMOVE(*e)) {
				if(_debugLevel & DISPLAY_CFG_CREATION)
					elm::cerr << __SOURCE_INFO__ << __TAB__ << __TAB__ << "this edge is removed, ignored" << io::endl;
				TO_REMOVE(*v).remove();
				continue;
			}
			Block* source = bmap.get(e->source());
			Block* target = bmap.get(e->sink());
			maker.add(source, target, new Edge(e->flags()));
		} // end for(BasicBlock::EdgeIter e = v->outs(); e; e++) {

		 // linking the artificial edge
		predecessor_list_t *edgeSources = ARTIFICIAL_PREDECESSORS(*v);
		if(edgeSources) { // for each edge
			for(predecessor_list_t::Iter plti(*edgeSources); plti(); plti++) {
				if(_debugLevel & DISPLAY_CFG_CREATION) {
					elm::cerr << __SOURCE_INFO__ << __TAB__ << "In CFG " << v->cfg() << ", " << (*plti).fst->index() << " to " << v->index() << io::endl;
					elm::cerr << __SOURCE_INFO__ << __TAB__ << __TAB__ << "this edge is created due to BB removal" << io::endl;
				}
				Block* source = bmap.get((*plti).fst);
				Block* target = bmap.get(*v);
				maker.add(source, target, new Edge((*plti).snd));
			}
			delete edgeSources;
		}
		ARTIFICIAL_PREDECESSORS(*v).remove();
		Vector<Block* > *edgeTargets = ARTIFICIAL_SUCCESSORS(*v);
		if(edgeTargets)
			delete edgeTargets;
		ARTIFICIAL_SUCCESSORS(*v).remove();

		// removed because for artificial edges we use incoming edges instead of the out-going ones.
		/*
		Vector<Block* > *edgeTargets = ARTIFICIAL_SUCCESSORS(*v);
		if(edgeTargets) {
			for(Vector<Block* >::Iterator i(*edgeTargets); i; i++) {
				if(_debugLevel & DISPLAY_CFG_CREATION) {
					elm::cerr << __SOURCE_INFO__ << __TAB__ << v->index() << " to " << i->index() << io::endl;
					elm::cerr << __SOURCE_INFO__ << __TAB__ << __TAB__ << "this edge is created due to BB removal" << io::endl;
				}
				maker.add(bmap.get(*v), bmap.get(*i), new Edge());
			}
			delete edgeTargets;
		}
		ARTIFICIAL_SUCCESSORS(*v).remove();
		predecessor_list_t *edgeSources = ARTIFICIAL_PREDECESSORS(*v);
		if(edgeSources)
			delete edgeSources;
		ARTIFICIAL_PREDECESSORS(*v).remove();
		*/
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
void Slicer::commit(WorkSpace *ws) {
	ASSERT(sliced_coll);
	ENTRY_CFG(ws) = (*sliced_coll)[0];
	INVOLVED_CFGS(ws) = sliced_coll;
}


///
void Slicer::destroy(WorkSpace *ws) {
	if(sliced_coll != nullptr) {

		List<CFG *> cfgsToDelete;
		for(auto g: *sliced_coll)
			delete g;

		ENTRY_CFG(ws).remove();
		INVOLVED_CFGS(ws).remove();
		delete sliced_coll;
		sliced_coll = nullptr;

	}
}



void Slicer::initIdentifiersForEachBB(const CFGCollection& coll) {
	// for each CFG
	for (int i = 0; i < coll.count(); i++) {
		for (CFG::BlockIter v = coll[i]->blocks(); v(); v++) {
			if(!v->isBasic())
				continue;
			SET_OF_REMAINED_INSTRUCTIONS(*v) = new InstSet();
			SLICER_REG_BB_END(*v) = BitVector(workspace()->platform()->regCount(), false);
			SLICER_MEM_BB_END(*v) = new otawa::dfa::MemorySet::t(0);
		} // end for (CFG::BlockIter v = cfg->blocks(); v; v++) {
	} // end for (int i = 0; i < coll.count(); i++) {
}

void Slicer::processWorkingList(elm::Vector<WorkingElement*>& workingList) {
	// while the list is not empty
	while(workingList.count())
	{
		// pop the first element to process
		WorkingElement* we = workingList.pop();
		Block* currentBB_wl = we->_bb;
		elm::BitVector currentRegs_wl = we->_workingRegs;
		// load the memory access set
		otawa::dfa::MemorySet::t currentMems_wl(we->_workingMems);
		Inst* currentInst_wl = we->_inst;

		if(_debugLevel & DISPLAY_SLICING_STAGES) {
			elm::cerr << __SOURCE_INFO__ << __RED__ << "Popping new working element out: BB " << currentBB_wl->index() << " @ " <<  currentBB_wl->address() << __RESET__ << io::endl;
			elm::cerr << __SOURCE_INFO__ << __TAB__ << "Starting witt " << currentInst_wl << " @ " << currentInst_wl->address() << io::endl;
			elm::cerr << __SOURCE_INFO__ << __TAB__ << "with working Regs  = " << currentRegs_wl << io::endl;
			if(!_lightSlicing) {
				elm::cerr << __SOURCE_INFO__ << __TAB__ << "with working MEM   = ";
				otawa::oslice::displayAddrs(elm::cerr, currentMems_wl);
				elm::cerr << io::endl;
			}
		}
		delete we;

		int currentReadMemIndex = 0;
		int currentWriteMemIndex = 0;
		bool reachFirstInstCurrentBB_wl = true;

		if(currentBB_wl->isBasic()) {
			reachFirstInstCurrentBB_wl = false;
			// here we fill the memory access information for each instruction
			if(!_lightSlicing)
				LivenessChecker::identifyAddrs(currentBB_wl->toBasic());
		}

		while(!reachFirstInstCurrentBB_wl)
		{
			if(_debugLevel & DISPLAY_SLICING_STAGES)
				elm::cerr << __SOURCE_INFO__ << __YELLOW__ << "Processing " << currentInst_wl << " @ " << currentInst_wl->address() << __RESET__ << io::endl;
			elm::BitVector currentRegsDef(workspace()->platform()->regCount(), false);
			elm::BitVector currentRegsUse(workspace()->platform()->regCount(), false);
			LivenessChecker::provideRegisters(currentInst_wl, currentRegsUse, 0);
			LivenessChecker::provideRegisters(currentInst_wl, currentRegsDef, 1);

			// for memory access
			otawa::dfa::MemorySet::t addressInstRead(0), addressInstWrite(0);
			if(!_lightSlicing) {
				LivenessChecker::getMems(currentBB_wl->toBasic(), currentInst_wl, currentReadMemIndex, addressInstRead, 0);
				LivenessChecker::getMems(currentBB_wl->toBasic(), currentInst_wl, currentWriteMemIndex, addressInstWrite, 1);
			}

			if(_debugLevel & DISPLAY_SLICING_STAGES) {
				elm::cerr << __SOURCE_INFO__ << __TAB__ << "Reg Def       = " << currentRegsDef << io::endl;
				elm::cerr << __SOURCE_INFO__ << __TAB__ << "Reg Use       = " << currentRegsUse << io::endl;
				if(!_lightSlicing) {
					elm::cerr << __SOURCE_INFO__ << __TAB__ << "Mem Def       = "; otawa::oslice::displayAddrs(elm::cerr, addressInstWrite); elm::cerr << io::endl;
					elm::cerr << __SOURCE_INFO__ << __TAB__ << "Mem Use       = "; otawa::oslice::displayAddrs(elm::cerr, addressInstRead); elm::cerr << io::endl;
				}
				elm::cerr << __SOURCE_INFO__ << __TAB__ << "RegSet Before = " << currentRegs_wl << io::endl;
				if(!_lightSlicing) {
					elm::cerr << __SOURCE_INFO__ << __TAB__ << "MemSet Before = ";  otawa::oslice::displayAddrs(elm::cerr, currentMems_wl);  elm::cerr << io::endl;
				}
			}

			// check if the def address is over lap with the working addrs
			bool memInterested = false;
			if(!_lightSlicing)
				memInterested = interestingAddrs(currentMems_wl, addressInstWrite);
			else
				memInterested = currentInst_wl->isStore();

			// if working Regs & def Regs is not zero, that means this instruction provides
			// the registers that we are interested
			bool regInsterested = interestingRegs(currentRegs_wl, currentRegsDef); // !(currentRegs_wl & currentRegsDef).isEmpty();

			if(memInterested | regInsterested) {

				if(_debugLevel & DISPLAY_SLICING_STAGES)
					elm::cerr << __SOURCE_INFO__ << __GREEN__ << __TAB__ << "We are interested in instruction " << currentInst_wl << __RESET__ << io::endl;

				// update the working Regs
				currentRegs_wl = (currentRegs_wl - currentRegsDef) | currentRegsUse;
				// update the current working memory
				if(!_lightSlicing)
					LivenessChecker::updateAddrsFromInstruction(currentMems_wl, addressInstRead, addressInstWrite, _debugLevel);
				// do something with the instruction
				SET_OF_REMAINED_INSTRUCTIONS(currentBB_wl)->add(currentInst_wl);
			}
			else
				if(_debugLevel & DISPLAY_SLICING_STAGES)
					elm::cerr << __SOURCE_INFO__ << __RED__ << __TAB__ << "We slice away instruction " << currentInst_wl << __RESET__ << io::endl;

			if(_debugLevel & DISPLAY_SLICING_STAGES) {
				elm::cerr << __SOURCE_INFO__ << __TAB__ << "RegSet After  = " << currentRegs_wl << io::endl;
				if(!_lightSlicing) {
					elm::cerr << __SOURCE_INFO__ << __TAB__ << "MemSet After  = "; otawa::oslice::displayAddrs(elm::cerr, currentMems_wl); elm::cerr << io::endl;
				}
			}

			if(currentInst_wl == currentBB_wl->toBasic()->first())
				reachFirstInstCurrentBB_wl = true;
			else
				currentInst_wl = currentInst_wl->prevInst();
		} // reaches the beginning of the BB

		// here reaches the beginning of the BB, now we need to list the list of incoming edges
		// so we can keep trace back the previous BB
		// first we find the predecessors of the BB to process
		elm::Vector<Block *> predecessors;

		for (Block::EdgeIter e = currentBB_wl->ins(); e(); e++) {
			Block* b = e->source(); // find the source of the edge, the predecessor of current BB

			if (b->isEntry()) {
				// then get the set of the callers
				for(auto caller: b->cfg()->callers()) {
					if(_debugLevel & DISPLAY_SLICING_STAGES)
						elm::cerr << __SOURCE_INFO__ << "Found a caller @ CFG " << caller->cfg()->index() << ", BB " << caller->index() << io::endl;

					for(Block::EdgeIter bei = caller->ins(); bei(); bei++) {
						if(_debugLevel & DISPLAY_SLICING_STAGES)
							elm::cerr << __SOURCE_INFO__ << __TAB__ << __GREEN__ << "Adding the block @ CFG " << bei->source()->cfg()->index() << ", " << bei->source() << __RESET__ << io::endl;
						predecessors.add(bei->source());
					}
				}
			} // end of handling the entry block
			else if (b->isBasic()) {
				// we put the BB here into the working list with given
				if(_debugLevel & DISPLAY_SLICING_STAGES)
					elm::cerr << __SOURCE_INFO__ << "Found predecessor BB @ " << b->toBasic()->address() << io::endl;
				predecessors.add(b->toBasic());
			} // end of handling the basic block
			else if (b->isSynth()) {
				if(!b->toSynth()->callee())
					continue;
				// this means that we reach current BB from the returning of a function
				// we obtain the exit block of the function that it returns from
				if(_debugLevel & DISPLAY_SLICING_STAGES) {
					elm::cerr << __SOURCE_INFO__ << "Found a Synth block with the callee to " << b->toSynth()->callee()->label() << io::endl;
				}
				Block* end = b->toSynth()->callee()->exit();
				// each edge to the exit block is a possible BB which will goes to the current block
				for (Block::EdgeIter EdgeToExit = end->ins(); EdgeToExit(); EdgeToExit++) {
					Block* BB_BeforeReturn = EdgeToExit->source();
					if(_debugLevel & DISPLAY_SLICING_STAGES)
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
			if(_debugLevel & DISPLAY_SLICING_STAGES)
				elm::cerr << __SOURCE_INFO__ << __RED__ << "No predecessor for BB @ " << currentBB_wl->address() << __RESET__ << io::endl;

		// process the collected BBs
		// now we need to see if the input (register and memory uses) feed from the successor matches totally or a subset of the pred BB
		for(elm::Vector<Block *>::Iter predecessor(predecessors); predecessor(); ++predecessor) {
			BitVector bv = SLICER_REG_BB_END(*predecessor);
			//clp_value_set_t* memIn = SLICER_MEM_BB_END(predecessor);
			otawa::dfa::MemorySet::t *memIn = SLICER_MEM_BB_END(*predecessor);

			bool notContainsAllRegs = !bv.includes(currentRegs_wl);
			bool notContainsAllMems = false;

			if(!_lightSlicing)
				notContainsAllMems = !LivenessChecker::containsAllAddrs(*memIn, currentMems_wl);

			if(_debugLevel & DISPLAY_SLICING_STAGES) {
				elm::cerr << __SOURCE_INFO__ << __TAB__ << "for BB @ " << predecessor->address() << io::endl;
				elm::cerr << __SOURCE_INFO__ << __TAB__ << __TAB__ << "Register used: " << bv << " contains all of " << currentRegs_wl << " ? "<< !notContainsAllRegs << io::endl;
				if(!_lightSlicing) {
					elm::cerr << __SOURCE_INFO__ << __TAB__ << __TAB__ << "Mem      used: ";
					otawa::oslice::displayAddrs(elm::cerr, *memIn); elm::cerr << " contains all of ";
					otawa::oslice::displayAddrs(elm::cerr, currentMems_wl); elm::cerr << "? " << !notContainsAllMems << io::endl;
				}
			}

			if(notContainsAllRegs | notContainsAllMems | !currentBB_wl->isBasic()) {
				if(_debugLevel & DISPLAY_SLICING_STAGES)
					elm::cerr << __SOURCE_INFO__ << __RED__ << "Adding BB @ " << predecessor->address() << " to the working list." << __RESET__ << io::endl;
				bv = bv | currentRegs_wl;
				SLICER_REG_BB_END(*predecessor) = bv;
				if(!_lightSlicing) {
					*memIn = dfa::MemorySet().join(*memIn, currentMems_wl);
					SLICER_MEM_BB_END(*predecessor) = memIn;
				}
				Inst* lastInstruction = 0;
				if(predecessor->isBasic())
					lastInstruction = predecessor->toBasic()->last();
				WorkingElement *we = new WorkingElement(*predecessor, lastInstruction, currentRegs_wl, currentMems_wl);
				workingList.add(we);
			}
			else
				if(_debugLevel & DISPLAY_SLICING_STAGES)
					elm::cerr << __SOURCE_INFO__ << __RED__ << "NOT adding BB @ " << predecessor->address() << " to the working list." << __RESET__ << io::endl;
		}

	} // end of the current working list
}

// check if any element in b is in a
inline bool Slicer::interestingAddrs(otawa::dfa::MemorySet::t const & a, otawa::dfa::MemorySet::t const & b) {
	dfa::MemorySet ms;
	dfa::MemorySet::t intersection = ms.meet(a, b);
	if(intersection == dfa::MemorySet::empty)
		return false;

	return true;
}

inline bool Slicer::interestingRegs(elm::BitVector const & a, elm::BitVector const & b) {
	elm::BitVector c(a.size(), true);
	c.clear(15); // ignore PC (ARM) cover 31->27
	c.clear(14); // ignore LR (ARM) cover 31->25
//	Processing bx lr @ 00200968
//		Reg Def       = 00000000000000010
//		Reg Use       = 00000000000000100

	return !((a & b & c).isEmpty());
}

void Slicer::slicing(void) {
	const CFGCollection& coll = **otawa::INVOLVED_CFGS(workspace());

	// putting the block to remove in the working list
	Vector<Block*> blocksToRemove;
	for (CFGCollection::Iter c(coll); c(); c++) {
		for (CFG::BlockIter v = c->blocks(); v(); v++) {
			if (v->isBasic()) {
				BasicBlock *bb = v->toBasic();
				InstSet* setInst = SET_OF_REMAINED_INSTRUCTIONS(bb);
				// if all the instruction are sliced
				if (setInst->count() == 0) {
					if(_debugLevel & DISPLAY_CFG_CREATION)
						elm::cerr << __SOURCE_INFO__<< "all instructions are sliced in BB" << bb->index() << " @ " << bb->address() << io::endl;
					// mark the BB sliced
					TO_REMOVE(bb) = true;
					blocksToRemove.add(bb);
				}
			} // if the block is basic
		} // for each Block
	} // for each CFG

	while(blocksToRemove.count()) {
		Block *b = blocksToRemove.pop();
		if(_debugLevel & DISPLAY_CFG_CREATION)
			elm::cerr << __SOURCE_INFO__ << "Popping BB " << b->index() << " of CFG " << b->cfg()->index() << " from the BB-removing working list" << io::endl;

		predecessor_list_t predecessors;
		Vector<Block*> successors;
		// Collect the predecessors of the current block
		// incoming edges exited in the original CFG
		for (Block::EdgeIter in = b->ins(); in(); in++) { // just to be safe not to remove the element during iter ops.
			// if the edge is not marked as removed, then we add the source of the edge to the list of predecessor
			if(!TO_REMOVE(*in) && in->source() != b) {
				predecessors.add(pair(in->source(),in->flags()));
				// mark the incoming edge removed
				TO_REMOVE(*in) = true;
				if(_debugLevel & DISPLAY_CFG_CREATION)
					elm::cerr << __SOURCE_INFO__ << __TAB__ << "Removing an input edge " << *in << io::endl;
			}
			else {
				if(_debugLevel & DISPLAY_CFG_CREATION)
					elm::cerr << __SOURCE_INFO__ << __TAB__ << *in << " has already been removed, ignored." << io::endl;
			}
		}
		// now processing the predecessor of the current block due to the removals of the other blocks
		predecessor_list_t *edgeSources = ARTIFICIAL_PREDECESSORS(b);
		if(edgeSources) {
			for(predecessor_list_t::Iter in(*edgeSources); in(); in++) {
				if(_debugLevel & DISPLAY_CFG_CREATION)
					elm::cerr << __SOURCE_INFO__ << __TAB__ << "Removing an input edge from " << (*in).fst << " to " << b << io::endl;
				if((*in).fst != b) // if there is a self looping edge on a removed BB, we will remove this edge too, hence the predecessor is not added
					predecessors.add(*in);
				// remove the current block from the successors of the its predecessor
				Vector<Block* > *edgeTargets = ARTIFICIAL_SUCCESSORS((*in).fst);
				if(edgeTargets)
					edgeTargets->remove(b);
			}
			delete edgeSources;
			ARTIFICIAL_PREDECESSORS(b).remove();
		}

		// Collecting the successors of the current block
		// now we process the out-going edges
		for (Block::EdgeIter out = b->outs(); out(); out++) { // just to be safe not to remove the element during iter ops.
			// if the edge is not yet marked removed, then we add the sink of the edge to sucessors
			if(!TO_REMOVE(*out) && out->target() != b)
				successors.add(out->sink());
			TO_REMOVE(*out) = true;
			if(_debugLevel & DISPLAY_CFG_CREATION)
				elm::cerr << __SOURCE_INFO__ << __TAB__ << "Removing an output edge to " << *out << io::endl;
		}
		// now we process the successors of the current block due to the removals of the other blocks
		Vector<Block* > *edgeTargets = ARTIFICIAL_SUCCESSORS(b);
		if(edgeTargets) {
			for(Vector<Block*>::Iter out(*edgeTargets); out(); out++) {
				if(_debugLevel & DISPLAY_CFG_CREATION)
					elm::cerr << __SOURCE_INFO__ << __TAB__ << "Removing an output edge to BB " << out->index() << io::endl;
				if(*out != b) // if there is a self looping edge on a removed BB, we will remove this edge too, hence the predecessor is not added
					successors.add(*out);
				// remove the current block from the list of the predecessors of its successor
				predecessor_list_t *edgeSources = ARTIFICIAL_PREDECESSORS(*out);
				if(edgeSources) {
					//edgeSources->remove(b);
					for(predecessor_list_t::Iter plti(*edgeSources); plti(); plti++) { // scan through each predecessor
						if((*plti).fst == b)
							edgeSources->remove(plti);
					}
				}
			}
			delete edgeTargets;
			ARTIFICIAL_SUCCESSORS(b).remove();
		}

		// special case, only one out going edge and pointed to iself (infinite loop ... often seen in the systems with waits for the interrupts
		if(b->countOuts() == 1 && b->outs()->sink() == b) {
			if(_debugLevel & DISPLAY_CFG_CREATION)
				elm::cerr << __SOURCE_INFO__ << "BB " << b->index() << " does not have any output edges but to himself, link it with the exit node" << io::endl;
			successors.add(b->cfg()->exit()); // connect to the exit node
		}

		// ========= TO REMOVE =========
		// actually this may not be necessary
		// if the predecessor has the targets of the current block, remove the current block from the target
		for (predecessor_list_t::Iter predecessor(predecessors); predecessor(); predecessor++) {
			Vector<Block* > *edgeTargets = ARTIFICIAL_SUCCESSORS((*predecessor).fst);
			if(!edgeTargets)
				continue;
			if(edgeTargets->contains(b)) {
				if(_debugLevel & DISPLAY_CFG_CREATION)
					elm::cerr << __SOURCE_INFO__ << "predecessor BB " << (*predecessor).fst->index() << " has a edge to current BB, removing...." << io::endl;
				ASSERT(0);
				edgeTargets->remove(b);
			}
		}
		// if the successor has predecessor of the current block, remove the current block from the source
		for (Vector<Block*>::Iter successor(successors); successor(); successor++) {
			predecessor_list_t *edgeSources = ARTIFICIAL_PREDECESSORS(*successor);
			if(!edgeSources)
				continue;
			//edgeSources->remove(b);
			for(predecessor_list_t::Iter plti(*edgeSources); plti(); plti++) { // scan through each predecessor
				if((*plti).fst == b) {
					if(_debugLevel & DISPLAY_CFG_CREATION)
						elm::cerr << __SOURCE_INFO__ << "successor BB " << successor->index() << " has a edge to current BB, removing...." << io::endl;
					ASSERT(0);
					edgeSources->remove(plti);
				}
			}
		}
		// ========= TO REMOVE END =========

		// now connect the predecessor with the successor
		 // for each predecessor, need to wire the edge between the predecessor and its successor
		for (predecessor_list_t::Iter predecessor(predecessors); predecessor(); predecessor++) {
			for (Vector<Block*>::Iter successor(successors); successor(); successor++) {
				// check if the successor is already linked with the predecessor
				// first check the real link
				bool foundRealEdge = false;
				bool foundArtificialEdge = false;
				Edge* realEdge = 0;
				for(Block::EdgeIter ei = (*predecessor).fst->outs(); ei(); ei++) {
					if(ei->target() == *successor && (*TO_REMOVE(*ei)) == false) {
						foundRealEdge = true;
						realEdge = *ei;
						break;
					}
				}
				// then check the artificial edge
				Vector<Block* > *edgeTargets = ARTIFICIAL_SUCCESSORS((*predecessor).fst);
				if(edgeTargets && edgeTargets->contains(*successor)) {
					foundArtificialEdge = true;
				}

				// in this case, we need to remove the exiting edge, creating an artificial edge whose flag is a combination of
				// the existing predecessor and current predecessor
				if(foundRealEdge) {
					// remove the existing real edge
					TO_REMOVE(realEdge) = true;
					t::uint32 existingFlag = realEdge->flags(); // extract the flag

					if(_debugLevel & DISPLAY_CFG_CREATION) {
						elm::cerr << __SOURCE_INFO__ << __TAB__ << "Already existing a real edge between CFG " << (*predecessor).fst->cfg()->index() << ", BB " << (*predecessor).fst->index() << " to BB " << successor->index() << ", need to remove" << io::endl;
						elm::cout << __SOURCE_INFO__ << __TAB__ << __TAB__ << "exiting edge flag = " << existingFlag << io::endl;
						elm::cout << __SOURCE_INFO__ << __TAB__ << __TAB__ << "new edge flag     = " << (*predecessor).snd << io::endl;
					}

					// add the successor edge from the predecessor as the replacement
					if(!edgeTargets) {
						edgeTargets = new Vector<Block* >();
						ARTIFICIAL_SUCCESSORS((*predecessor).fst) = edgeTargets;
					}
					edgeTargets->add(*successor);

					// similarly we add the predecessor with the desired exiting flag
					predecessor_list_t *edgeSources = ARTIFICIAL_PREDECESSORS(*successor);
					if(!edgeSources) {
						edgeSources = new predecessor_list_t();
						ARTIFICIAL_PREDECESSORS(*successor) = edgeSources;
					}
					existingFlag = existingFlag | (*predecessor).snd;
					edgeSources->add(pair((*predecessor).fst, existingFlag));
				}
				else if(foundArtificialEdge) {
					t::uint32 existingFlag = 0;
					// we need to find the predecessor!
					predecessor_list_t *edgeSources = ARTIFICIAL_PREDECESSORS(*successor);
					for (predecessor_list_t::Iter edgeSource(*edgeSources); edgeSource(); edgeSource++) {
						if((*edgeSource).fst == (*predecessor).fst) {
							if(_debugLevel & DISPLAY_CFG_CREATION) {
								elm::cerr << __SOURCE_INFO__ <<__TAB__ << "Already existing a artificial edge between CFG " << (*predecessor).fst->cfg()->index() << ", BB " << (*predecessor).fst->index() << " to BB " << successor->index() << ", need to remove" << io::endl;
								elm::cout << __SOURCE_INFO__ <<__TAB__ << __TAB__ << "exiting edge flag = " << (*edgeSource).snd << io::endl;
								elm::cout << __SOURCE_INFO__ <<__TAB__ << __TAB__ << "new edge flag     = " << (*predecessor).snd << io::endl;
							}
							existingFlag = (*edgeSource).snd | (*predecessor).snd;
							edgeSources->remove(edgeSource);
							break;
						}
					}
					edgeSources->add(pair((*predecessor).fst, existingFlag));
				}
				else {
					// make the wiring
					if(_debugLevel & DISPLAY_CFG_CREATION)
						elm::cerr << __SOURCE_INFO__ << __TAB__ << "Adding edge between BB " << (*predecessor).fst->index() << " to BB " << successor->index() << io::endl;
					// connecting the predecessor with all of the successors
					Vector<Block* > *edgeTargets = ARTIFICIAL_SUCCESSORS((*predecessor).fst);
					// in case the EDGE_TARGET is not initialized
					if(!edgeTargets) {
						edgeTargets = new Vector<Block* >();
						ARTIFICIAL_SUCCESSORS((*predecessor).fst) = edgeTargets;
					}
					edgeTargets->add(*successor);
					predecessor_list_t *edgeSources = ARTIFICIAL_PREDECESSORS(*successor);
					if(!edgeSources) {
						edgeSources = new predecessor_list_t();
						ARTIFICIAL_PREDECESSORS(*successor) = edgeSources;
					}
					edgeSources->add(*predecessor);
				}
			}
		} // finish linking the predecessors and successors of the removal BB
	} // end of the working list

	for(CFGCollection::Iter c(coll); c(); c++) {
		makeCFG(*c);
	}

	sliced_coll = new CFGCollection();
	for(FragTable<CFGMaker *>::Iter m(makers); m(); m++) {
	        sliced_coll->add(m->build());
	        delete *m;
	}

}

} }
