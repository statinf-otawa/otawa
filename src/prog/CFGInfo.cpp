/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	src/prog/CFGInfo.cpp -- implementation of CFGInfo class.
 */

#include <otawa/instruction.h>
#include <otawa/manager.h>
#include <otawa/cfg.h>

// Trace
//#	define TRACE_CFG_INFO
#if defined(NDEBUG) || !defined(TRACE_CFG_INFO)
#	define TRACE(str)
#else
#		define TRACE(str) \
			{ \
				cerr << __FILE__ << ':' << __LINE__ << ": " << str << '\n'; \
				cerr.flush(); \
			}
#endif

namespace otawa {

/**
 * @class CFGInfo
 * This allows storing all CFG available in a workspace.
 */


/**
 * Identifier for retrieving CFG information from the workspace properties.
 * 
 * @par Hooks
 * @li @ref FrameWork
 */
GenericIdentifier<CFGInfo *> CFGInfo::ID("cfg_info_id", 0, OTAWA_NS);


/**
 * Build a new CFGInfo.
 * @param fw	Framework that the CFG information applies to.
 */
CFGInfo::CFGInfo(FrameWork *_fw, elm::Collection <CFG *>& cfgs)
: fw(_fw) {
	TRACE(this << ".CFGInfo::CFGInfo(" << _fw << ")");
	_cfgs.addAll(&cfgs);
}


/**
 * Delete the CFG information contained in this program.
 */
CFGInfo::~CFGInfo(void) {
	TRACE(this << ".CFGInfo::~CFGInfo()");	
	clear();
}


/**
 * Remove all CFG stored in this CFG information.
 */
void CFGInfo::clear(void) {
	PseudoInst *pseudo;
	
	// Remove CFGs
	for(int i = 0; i < _cfgs.count(); i++)
		delete _cfgs[i];
	_cfgs.clear();
	
	// Release basic block
	for(Iterator<File *> file(*fw->files()); file; file++)
		for(Iterator<Segment *> seg(file->segments()); seg; seg++)
			for(Iterator<ProgItem *> item(seg->items()); item; item++)
				if(seg->flags() & Segment::EXECUTABLE) {
					CodeItem *code = (CodeItem *)*item;
						for(Inst *inst = code->first(); !inst->atEnd();) {
							PseudoInst *pseudo = inst->toPseudo();
							inst = inst->next();
							if(pseudo && pseudo->id() == &BasicBlock::ID)
								delete ((BasicBlock::Mark *)pseudo)->bb();
						}
				}
}


/**
 * Find the basic block containing the given instruction.
 * @param inst		Instruction to find the BB for.
 * @return				Found basic block. If the code is not already managed,
 * it is automatically added.
 */
BasicBlock *CFGInfo::findBB(Inst *inst) {
	PseudoInst *pseudo;
	while(!inst->atBegin()) {
		if((pseudo = inst->toPseudo()) && pseudo->id() == &CodeBasicBlock::ID)
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
	return ENTRY(bb);
}


/**
 * Get the collection of CFG.
 * @return CFG collection.
 */
elm::Collection<CFG *>& CFGInfo::cfgs(void) {
	return _cfgs;
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
