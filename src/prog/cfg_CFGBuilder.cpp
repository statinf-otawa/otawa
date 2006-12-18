/*
 *	$Id$
 *	Copyright (c) 2005, IRIT UPS.
 *
 *	prog/cfg_CFGBuilder.cpp -- CFGBuilder class implementation.
 */

#include <otawa/cfg/CFGBuilder.h>
#include <otawa/cfg.h>

namespace otawa {

/**
 */
Identifier CFGBuilder::ID_Entry("otawa::ID_Entry");

	

/**
 * @class CFGBuilder
 * This processor is used for building the CFG from the binary program
 * representation. Found CFG are linked the framework to the framework thanks
 * a CFGInfo object stored in a property.
 * 
 * @Provided Features
 * @li @ref CFG_INFO_FEATURE
 */


/**
 * CFG builder constructor.
 */
CFGBuilder::CFGBuilder(void)
: Processor("otawa::CFGBuilder", Version(1, 0, 0)) {
	provide(CFG_INFO_FEATURE);
}


/**
 */	
void CFGBuilder::processFrameWork(FrameWork *fw) {
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
	for(Inst *node = inst->next(); !node->atEnd(); node = node->next()) {
		PseudoInst *pseudo = node->toPseudo();
		
		// Instruction found (no BB): create it
		if(!pseudo) {
			BasicBlock *bb = new CodeBasicBlock(node);
			assert(bb);
			bb->set<bool>(ID_Entry, false);
			return bb;
		}
		
		// Is the BB pseudo?
		if(pseudo->id() == &CodeBasicBlock::ID) {
			return ((CodeBasicBlock::Mark *)pseudo)->bb();
		}
	}
	
	// End-of-code
	BasicBlock *bb = new CodeBasicBlock(inst->next());
	bb->set<bool>(ID_Entry, false);
	return bb;
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
	for(Inst *node = inst->previous(); !node->atBegin(); node = node->previous()) {
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
	bb->set<bool>(ID_Entry, false);
	return bb;
}


/**
 * Record the given instruction as the startup of a sub-program.
 * @param inst	Subprogram startup.
 */
void CFGBuilder::addSubProgram(Inst *inst) {
	assert(inst);
	BasicBlock *bb = thisBB(inst);
	bb->set<bool>(ID_Entry, true);
}


/**
 * Build the CFG for the given code.
 * @param code	Code to build the CFG for.
 */
void CFGBuilder::buildCFG(CodeItem *code) {
	assert(code);
	PseudoInst *pseudo;
	
	// Add the initial basic block
	BasicBlock *bb = thisBB(code->first());
	
	// Find the basic blocks
	for(Inst *inst = code->first(); !inst->atEnd(); inst = inst->next())
		if(inst->isControl()) {
			
			// Found BB starting on target instruction			
			Inst *target = inst->target();
			if(target) {
				assert(!target->isPseudo());
				BasicBlock *bb = thisBB(target);
				assert(bb);
				if(inst->isCall())
					bb->set<bool>(ID_Entry, true);
			}

			// Found BB starting on next instruction
			BasicBlock *bb = nextBB(inst);
			assert(bb);
		}
	
	// Build the graph
	genstruct::Vector<BasicBlock *> entries;
	bb = 0;
	bool follow = true;
	for(Inst *inst = code->first(); !inst->atEnd(); inst = inst->next()) {
		
		// Start of block found
		if((pseudo = inst->toPseudo()) && pseudo->id() == &CodeBasicBlock::ID) {
			
			// Record not-taken edge
			BasicBlock *next_bb = ((CodeBasicBlock::Mark *)pseudo)->bb();
			if(bb && follow)
				new Edge(bb, next_bb, EDGE_NotTaken);
			
			// Initialize new BB
			bb = next_bb;
			follow = true;
			if(bb->get<bool>(ID_Entry, false)) {
				assert(!entries.contains(bb));
				entries.add(bb);
			}
			bb->removeProp(&ID_Entry);
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
	
	// Recording the CFG
	for(int i = 0; i < entries.length(); i++)
		_cfgs.add(new CFG(code, entries[i]));
}


/**
 * Add a file to the builder.
 * @param file	Added file.
 */
void CFGBuilder::addFile(File *file) {
	assert(file);
	
	// Scan file symbols
	for(Iterator<Symbol *> sym(file->symbols()); sym; sym++)
		if(sym->kind() == Symbol::FUNCTION) {
			Inst *inst = sym->findInst();
			if(inst)
				addSubProgram(inst);
	}
	
	// Scan file segments
	for(Iterator<Segment *> seg(file->segments()); seg; seg++)
		for(Iterator<ProgItem *> item(seg->items()); item; item++)
			if(seg->flags() & Segment::EXECUTABLE)
				buildCFG((CodeItem *)*item);
}


/**
 * Build the CFG of the program.
 * @param fw	Current framework.
 */
void CFGBuilder::buildAll(FrameWork *fw) {
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
	fw->addDeletable(CFGInfo::ID, new CFGInfo(fw, _cfgs));
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
