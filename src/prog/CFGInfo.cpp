/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	src/prog/CFGInfo.cpp -- implementation of CFGInfo class.
 */

#include <otawa/instruction.h>
#include <otawa/manager.h>
#include <otawa/cfg/BasicBlock.h>
#include <otawa/cfg/CFGInfo.h>

// Trace
#ifndef NDEBUG
//#	define TRACE_CFG_INFO
#	ifdef TRACE_CFG_INFO
#		define TRACE(str) \
			{ \
				cerr << __FILE__ << ':' << __LINE__ << ": " << str << '\n'; \
				cerr.flush(); \
			}
#	else
#		define TRACE(str)
#	endif
#endif

namespace otawa {

/**
 * @class CFGInfo
 * This allows storing all CFG available in a workspace.
 */

/**
 * Identifier for retrieving CFG information from the workspace properties.
 */
const id_t CFGInfo::ID = Property::getID("otawa.CFGInfo");

/**
 * Identifier for marking basic blocks entries of a CFG.
 */
id_t CFGInfo::ID_Entry = Property::getID("otawa.CFGEntry");

	
/**
 * Build a new CFGInfo.
 * @param fw	Framework that the CFG information applies to.
 */
CFGInfo::CFGInfo(FrameWork *_fw): built(false), fw(_fw) {
	TRACE(this << ".CFGInfo::CFGInfo(" << _fw << ")");
	//fw->set<CFGInfo *>(ID, this);
	fw->setProp(new LockedProperty<CFGInfo *>(ID, this));
}


/**
 * Delete the CFG information contained in this program.
 */
CFGInfo::~CFGInfo(void) {
	TRACE(this << ".CFGInfo::~CFGInfo()");	
	clear();
}


/**
 * Add the given file to the files supported by this CFG information.
 * @param file	Added file.
 */
void CFGInfo::addFile(File *file) {
	// !!NOT IMPLEMENTED!!
	assert(0);
}


/**
 * Remove all CFG stored in this CFG information.
 */
void CFGInfo::clear(void) {
	PseudoInst *pseudo;
	
	/* Remove CFGs */
	for(int i = 0; i < _cfgs.count(); i++)
		delete _cfgs[i];
	_cfgs.clear();
	
	/* Release basic block */
	while(!bbs.isEmpty()) {
		BasicBlock *bb = (BasicBlock *)bbs.first();
		bbs.removeFirst();
		delete bb;
	}
}


/**
 * Find the basic block starting on the next instruction. If none exists, create
 * it.
 * @param inst		Instructtion to find the BB after.
 * @return			Found or created BB or null if end-of-code is reached.
 */
BasicBlock *CFGInfo::nextBB(Inst *inst) {
	for(Inst *node = inst->next(); !node->atEnd(); node = node->next()) {
		PseudoInst *pseudo = node->toPseudo();
		
		// Instruction found (no BB): create it
		if(!pseudo) {
			BasicBlock *bb = new CodeBasicBlock(node);
			assert(bb);
			bbs.addLast(bb);
			bb->set<bool>(ID_Entry, false);
			return bb;
		}
		
		// Is the BB pseudo?
		if(pseudo->id() == CodeBasicBlock::ID) {
			return ((CodeBasicBlock::Mark *)pseudo)->bb();
		}
	}
	
	// End-of-code
	BasicBlock *bb = new CodeBasicBlock(inst->next());
	bbs.addLast(bb);
	bb->set<bool>(ID_Entry, false);
	return bb;
}


/**
 * Get or create the basic block on the given instruction.
 * @param inst		Instruction starting the BB.
 * @return				Found or created BB.
 */
BasicBlock *CFGInfo::thisBB(Inst *inst) {
	
	// Straight in BB?
	PseudoInst *pseudo = inst->toPseudo();
	if(pseudo && pseudo->id() == CodeBasicBlock::ID)
		return ((CodeBasicBlock::Mark *)pseudo)->bb();
	
	// Look backward
	for(Inst *node = inst->previous(); !node->atBegin(); node = node->previous()) {
		pseudo = node->toPseudo();
		
		// No pseudo, require to create the BB
		if(!pseudo)
			break;
		
		// Is it a BB pseudo?
		else if(pseudo->id() == CodeBasicBlock::ID)
			return ((CodeBasicBlock::Mark *)pseudo)->bb();
	}
	
	// At start, create the BB
	BasicBlock *bb = new CodeBasicBlock(inst);
	bbs.addLast(bb);
	bb->set<bool>(ID_Entry, false);
	return bb;
}


/**
 * Add the given code to the CFG information.
 * @param	code	Code to add.
 * @param file	Owner file.
 */
void CFGInfo::addCode(Code *code, File *file) {
	
	// Add the code
	_codes.add(code);
	
	// Add the functions entries
	if(file)
		for(Iterator<Symbol *> sym(file->symbols()); sym; sym++)
			if(sym->kind() == SYMBOL_Function) {
				Inst *inst = sym->findInst();
				if(inst)
					addSubProgram(inst);
			}
}


/**
 * Find the basic block containing the given instruction.
 * @param inst		Instruction to find the BB for.
 * @return				Found basic block. If the code is not already managed,
 * it is automatically added.
 */
BasicBlock *CFGInfo::findBB(Inst *inst) {

	// Built the CFG
	if(!built)
		build();
	
	// Look for the BB mark
	PseudoInst *pseudo;
	while(!inst->atBegin()) {
		if((pseudo = inst->toPseudo()) && pseudo->id() == CodeBasicBlock::ID)
			return ((CodeBasicBlock::Mark *)pseudo)->bb();
		inst = inst->previous();
	}
	assert(0);
}


/**
 * Find the CFG starting by the basic block containing this instruction.
 * If the CFG does not exists, it is created and added.
 * @par
 * The current algorithm does not allow detecting all CFG due to the irregular
 * nature of the assembly code. Specially, function entries also used as
 * loop head or as jump target of a function without continuation.
 * Using this method, the user may inform the system about other existing CFG.
 * @param inst	Instruction to find the CFG starting with.
 * @return 	Matching CFG or null.
 */
CFG *CFGInfo::findCFG(Inst *inst) {
	
	// Get the basic block
	BasicBlock *bb = findBB(inst);
	assert(bb);
	
	// Look for a CFG
	return findCFG(bb);
}


/**
 * Find the CFG starting at the given basic block.
 * @param bb	Basic block to look at.
 * @return	Found CFG or this BB is not a CFG start.
 */
CFG *CFGInfo::findCFG(BasicBlock *bb) {
	return bb->get<CFG *>(CFG::ID, 0);
}


/**
 * Get the collection of CFG.
 * @return CFG collection.
 */
elm::Collection<CFG *>& CFGInfo::cfgs(void) {
	if(!built)
		build();
	return _cfgs;
}


/**
 * @fn const Collection<Code *>& CFGInfo::codes(void) const;
 * Get the collection of codes.
 * @return Code collection.
 */


/**
 * Build the CFG of the program.
 */
void CFGInfo::build(void) {
	
	// Already built ?
	if(built)
		return;
	built = true;
	
	// Find and mark the start
	Inst *start = fw->start();
	if(start)
		addSubProgram(start);
	
	// Compute CFG for each code piece
	for(Iterator<Code *> code(_codes); code; code++)
		buildCFG(*code);
}


/**
 * Build the CFG for the given code.
 * @param code	Code to build the CFG for.
 */
void CFGInfo::buildCFG(Code *code) {
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
					bb->use<bool>(ID_Entry) = true;
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
		if((pseudo = inst->toPseudo()) && pseudo->id() == CodeBasicBlock::ID) {
			
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
			bb->removeProp(ID_Entry);
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
 * Record the given instruction as the startup of a sub-program.
 * @param inst	Subprogram startup.
 */
void CFGInfo::addSubProgram(Inst *inst) {
	BasicBlock *bb = thisBB(inst);
	bb->set<bool>(ID_Entry, true);
}


/**
 * Find the CFG starting at the given label.
 * @param label		Label of the first instruction of the CFG.
 * @return			Matching CFG or null.
 */
CFG *CFGInfo::findCFG(String label) {
	Inst *inst = fw->Process::findInstAt(label);
	if(!inst)
		return 0;
	else
		return findCFG(inst);
}

} // otawa
