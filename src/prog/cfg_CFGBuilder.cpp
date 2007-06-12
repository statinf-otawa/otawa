/*
 *	$Id$
 *	Copyright (c) 2005-07, IRIT UPS.
 *
 *	CFGBuilder class implementation
 */

#include <otawa/cfg/CFGBuilder.h>
#include <otawa/cfg.h>
#include <otawa/prog/TextDecoder.h>

namespace otawa {

/**
 */
Identifier<bool> IS_ENTRY("otawa::is_entry", false);

	

/**
 * @class CFGBuilder
 * This processor is used for building the CFG from the binary program
 * representation. Found CFG are linked the framework to the framework thanks
 * a CFGInfo object stored in a property.
 * 
 * @par Provided Features
 * @li @ref CFG_INFO_FEATURE
 * 
 * @par Required Features
 * @li @ref DECODED_TEXT
 */


/**
 * CFG builder constructor.
 */
CFGBuilder::CFGBuilder(void)
: Processor("otawa::CFGBuilder", Version(1, 0, 0)), verbose(false) {
	provide(CFG_INFO_FEATURE);
	require(DECODED_TEXT);
}


/**
 */	
void CFGBuilder::processWorkSpace(WorkSpace *fw) {
	assert(fw);
	buildAll(fw);
}


/**
 * Find the basic block starting on the next instruction. If none exists, create
 * it.
 * @param inst		Instructtion to find the BB after.
 * @return			Found or created BB or null if end-of-code is reached.
 */
BasicBlock *CFGBuilder::nextBB(Inst *inst) {
	assert(inst);
	for(Inst *node = inst->nextInst(); node; node = node->nextInst()) {
		PseudoInst *pseudo = node->toPseudo();
		
		// Instruction found (no BB): create it
		if(!pseudo) {
			BasicBlock *bb = new CodeBasicBlock(node);
			assert(bb);
			IS_ENTRY(bb) = false;
			return bb;
		}
		
		// Is the BB pseudo?
		if(pseudo->id() == &CodeBasicBlock::ID) {
			return ((CodeBasicBlock::Mark *)pseudo)->bb();
		}
	}
	
	// End-of-code
	/*BasicBlock *bb = new CodeBasicBlock(inst->nextInst());
	IS_ENTRY(bb) = false;
	return bb;*/
	return 0;
}


/**
 * Get or create the basic block on the given instruction.
 * @param inst		Instruction starting the BB.
 * @return				Found or created BB.
 */
BasicBlock *CFGBuilder::thisBB(Inst *inst) {
	assert(inst);
	
	// Straight in BB?
	PseudoInst *pseudo = inst->toPseudo();
	if(pseudo && pseudo->id() == &CodeBasicBlock::ID)
		return ((CodeBasicBlock::Mark *)pseudo)->bb();
	
	// Look backward
	for(Inst *node = inst->prevInst(); node; node = node->prevInst()) {
		pseudo = node->toPseudo();
		
		// No pseudo, require to create the BB
		if(!pseudo)
			break;
		
		// Is it a BB pseudo?
		else if(pseudo->id() == &CodeBasicBlock::ID)
			return ((CodeBasicBlock::Mark *)pseudo)->bb();
	}
	
	// At start, create the BB
	BasicBlock *bb = new CodeBasicBlock(inst);
	IS_ENTRY(bb) = false;
	return bb;
}


/**
 * Record the given instruction as the startup of a sub-program.
 * @param inst	Subprogram startup.
 */
void CFGBuilder::addSubProgram(Inst *inst) {
	assert(inst);
	BasicBlock *bb = thisBB(inst);
	IS_ENTRY(bb) = true;
}


/**
 * Build the CFG for the given code.
 * @param code	Code to build the CFG for.
 */
void CFGBuilder::buildCFG(Segment *seg) {
	assert(seg);
	PseudoInst *pseudo;
	
	/*for(Segment::ItemIter item(seg); item; item++)
		cerr << "ITEM " << item->address() << io::endl;*/
	
	// Add the initial basic block
	Segment::ItemIter item(seg);
	while(item && !item->toInst())
		item++;
	if(!item)
		return;
	BasicBlock *bb = thisBB(item->toInst());
	
	// Find the basic blocks
	for(; item; item++) {
		//cerr << "Processing " << item->address() << io::endl;
		Inst *inst = item->toInst();
		if(inst && inst->isControl()) {
			
			// Found BB starting on target instruction			
			Inst *target = inst->target();
			if(target) {
				assert(!target->isPseudo());
				BasicBlock *bb = thisBB(target);
				assert(bb);
				if(inst->isCall())
					IS_ENTRY(bb) = true;
			}
			else if(!inst->isReturn() && verbose) {
				Symbol * sym = 0; //code->closerSymbol(inst);
				warn( _ << "unresolved indirect control at 0x"
					<< inst->address().address() << " ("
					<< (sym ? &sym->name()  : "") << " + 0x"
					<< (sym ? (inst->address() - sym->address()) : 0) << ")");
				cout << '\t' << inst->address() << '\t';
				inst->dump(out);
				cout << io::endl;
			}

			// Found BB starting on next instruction
			BasicBlock *bb = nextBB(inst);
			//assert(bb);
		}
	}
	
	// Build the graph
	genstruct::Vector<BasicBlock *> entries;
	bb = 0;
	bool follow = true;
	for(Segment::ItemIter item(seg); item; item++) {
		Inst *inst = item->toInst();
		if(inst) {
			
			// Start of block found
			if((pseudo = inst->toPseudo()) && pseudo->id() == &CodeBasicBlock::ID) {
			
				// Record not-taken edge
				BasicBlock *next_bb = ((CodeBasicBlock::Mark *)pseudo)->bb();
				if(bb && follow)
					new Edge(bb, next_bb, EDGE_NotTaken);
			
				// Initialize new BB
				bb = next_bb;
				follow = true;
				if(IS_ENTRY(bb)) {
					assert(!entries.contains(bb));
					entries.add(bb);
				}
				bb->removeProp(&IS_ENTRY);
			}
		
			// End of block
			else if(inst->isControl()) {
			
				// Record the taken edge
				Inst *target = inst->target();
				if(target) {
					BasicBlock *target_bb = thisBB(target);
					assert(target_bb);
					new Edge(bb, target_bb, inst->isCall() ? EDGE_Call : EDGE_Taken);
				}
			
				// Record BB flags
				if(inst->isReturn())
					bb->flags |= BasicBlock::FLAG_Return;
				else if(inst->isCall())
					bb->flags |= BasicBlock::FLAG_Call;
				if(!target && !inst->isReturn())
					bb->flags |= BasicBlock::FLAG_Unknown;
				follow = inst->isCall() || inst->isConditional();
			}
		}
	}
	
	// Recording the CFG
	for(int i = 0; i < entries.length(); i++)
		_cfgs.add(new CFG(seg, entries[i]));
}


/**
 * Add a file to the builder.
 * @param file	Added file.
 */
void CFGBuilder::addFile(File *file) {
	assert(file);
	
	// Scan file symbols
	for(File::SymIter sym(file); sym; sym++)
		if(sym->kind() == Symbol::FUNCTION) {
			Inst *inst = sym->findInst();
			if(inst)
				addSubProgram(inst);
	}
	
	// Scan file segments
	for(File::SegIter seg(file); seg; seg++)
		if(seg->isExecutable())
			buildCFG(seg);
}


/**
 * Build the CFG of the program.
 * @param fw	Current framework.
 */
void CFGBuilder::buildAll(WorkSpace *fw) {
	assert(fw);
	_cfgs.clear();
	
	// Remove old built
	fw->removeProp(&CFGInfo::ID);
	
	// Find and mark the start
	Inst *start = fw->start();
	if(start)
		addSubProgram(start);
	
	// Compute CFG for each code piece
	for(Process::FileIter file(fw->process()); file; file++)
		addFile(file);
	
	// Build the CFGInfo
	CFGInfo *info = new CFGInfo(fw);
	for(genstruct::Vector<CFG *>::Iterator cfg(_cfgs); cfg; cfg++)
		info->add(cfg);
}


/**
 */
void CFGBuilder::configure(const PropList& props) {
	verbose = VERBOSE(props);
	Processor::configure(props);
}


/**
 * Feature asserting that the CFG has been scanned in the program. The result
 * is put the @ref CFGInfo::ID.
 * 
 * @Properties
 * @li @ref CFGInfo::ID (FrameWork)
 */
Feature<CFGBuilder> CFG_INFO_FEATURE("otawa::cfg_info");

} // otawa
