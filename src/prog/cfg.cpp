/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	cfg.cpp -- control flow graph classes implementation.
 */

#include <elm/collection.h>
#include <otawa/cfg.h>

namespace otawa {

/**
 * @class BasicBlock
 * Represent a basic block in the strictess meaning: a sequence of instructions
 * containing no branch, or branch target except for the last instruction that
 * may be a branch. Unconditionnal branches are viewed as normal branch
 * in this definition.
 */

/**
 * Build a basic block from its container code and its sequence of instructions.
 * @param head	First instruction of the basic block. The basic block
 * lay from this instruction to the next basic block head or end of code.
 */
BasicBlock::BasicBlock(Inst *head) {
	_head = new Mark(this);
	head->insertBefore(_head);
}

/**
 * @fn BasicBlock *BasicBlock::getTaken(void) const;
 * Get the target basic block if the last branch instruction is taken.
 * @return Target basic block or null if the last instruction is not a branch.
 */

/**
 * @fn BasicBlock *BasicBlock::getNotTaken(void) const;
 * Get the following basic block if the last branch instruction is not taken.
 * @return Following basic block or null if the last instruction is a sub-program return.
 */

/**
 * @fn void BasicBlock::setTaken(BasicBlock *bb);
 * Set the target basic block of the branch last instruction.
 * @param bb	New target basic block.
 */

/**
 * @fn void BasicBlock::setNotTaken(BasicBlock *bb);	
 * Set the following basic block.
 * @param bb	New following basic block.
 */

/**
 * Identifier of the basic block pseudo-instruction.
 */
id_t BasicBlock::ID = Property::getID("otawa.BasicBlock");

/**
 * @fn Mark *BasicBlock::head(void) const;
 * Get the mark pseudo-instruction of the basic block. Following instruction
 * until the next mark are the content of the basic block.
 * @return	Basic block mark.
 */


/**
 * @class CFGInfo
 * This allows storing all CFG available in a workspace.
 */

/**
 * Identifier for retrieving CFG information from the workspace properties.
 */
id_t CFGInfo::ID_CFG = Property::getID("otawa.CFGInfo");

/**
 * Remove all CFG stored in this CFG information.
 */
void CFGInfo::clear(void) {
	
	/* Remove CFGs */
	while(!_cfgs.isEmpty())
		delete _cfgs[0];
		_cfgs.removeHead();
	
	/* Remove basic blocks. */
	PseudoInst *pseudo;
	for(Iterator<Code *> iter(_codes.visit()); iter; iter++)
		for(Inst *inst = (*iter)->first(); !inst->atEnd(); ) {
			if((pseudo = inst->toPseudo()) && pseudo->id() == BasicBlock::ID) {
				inst = pseudo->next();
				pseudo->remove();
				delete pseudo;
			}
			else
				inst = inst->next();
		}
}

/**
 * Find the basic block starting on the next instruction. If none exists, create
 * it.
 * @param inst		Instructtion to find the BB after.
 * @return				Found or created BB or null if end-of-code is reached.
 */
BasicBlock *CFGInfo::nextBB(Inst *inst) {
	for(Inst *node = inst->next(); !node->atEnd(); node = node->next()) {
		PseudoInst *pseudo = node->toPseudo();
		
		// Instruction found (no BB): create it
		if(!pseudo) {
			BasicBlock *bb = new BasicBlock(inst->next());
			bb->set<bool>(ID_Entry, true);
			return bb;
		}
		
		// Is the BB pseudo?
		if(pseudo->id() == BasicBlock::ID)
			return ((BasicBlock::Mark *)pseudo)->bb();
	}
	
	// End-of-code
	return 0;
}


/**
 * Get or create the basic block on the given instruction.
 * @param inst		Instruction starting the BB.
 * @return				Found or created BB.
 */
BasicBlock *CFGInfo::thisBB(Inst *inst) {
	for(Inst *node = inst->previous(); !node->atBegin(); node = node->previous()) {
		PseudoInst *pseudo = inst->toPseudo();
		
		// No pseudo, require to create the BB
		if(!pseudo)
			break;
		
		// Is it a BB pseudo?
		else if(pseudo->id() == BasicBlock::ID)
			return ((BasicBlock::Mark *)pseudo)->bb();
	}
	
	// At start, create the BB
	BasicBlock *bb = new BasicBlock(inst);
	bb->set<bool>(ID_Entry, true);
	return bb;
}


/**
 * Add the given code to the CFG list. A basic block analysis is performed
 * on the code and CFG are extracted from it.
 * @param	code	Code to add.
 */
void CFGInfo::addCode(Code *code) {
	ControlInst *ctrl;
	PseudoInst *pseudo;
	
	// Add the initial basic block
	_codes.add(code);
	BasicBlock *bb = new BasicBlock(code->first());
	data::Vector<BasicBlock *> entries;
	
	// Find the basic blocks
	for(Inst *inst = code->first(); !inst->atEnd(); inst = inst->next())
		if(ctrl = inst->toControl()) {
			
			// Found BB starting on next instruction
			BasicBlock *bb = nextBB(inst);
			if(bb)
				bb->use<bool>(ID_Entry) = false;
			
			// Found BB starting on target instruction
			Inst *target = ctrl->target();
			if(target) {
				BasicBlock *bb = thisBB(target);
				if(!target->isCall())
					bb->use<bool>(ID_Entry) &= ~false;
				else
					entries.add(bb);
			}
		}
	
	// Build the graph
	bb = 0;
	bool follow = true;
	for(Inst *inst = code->first(); !inst->atEnd(); inst = inst->next()) {
		
		// Start of block found
		if((pseudo = inst->toPseudo()) && pseudo->id() == BasicBlock::ID) {
			BasicBlock *next_bb = ((BasicBlock::Mark *)pseudo)->bb();
			if(bb && follow)
				bb->setNotTaken(next_bb);
			bb = next_bb;
			follow = true;
			if(bb->get<bool>(ID_Entry, false))
				entries.add(bb);
			bb->removeProp(ID_Entry);
		}
		
		// End of block
		else if(ctrl = inst->toControl()) {
			Inst *target = ctrl->target();
			if(target) {
				BasicBlock *target_bb = thisBB(target);
				assert(target_bb);
				bb->setTaken(target_bb);
			}
			follow = ctrl->isCall() || (ctrl->isConditional());
		}
	}
	
	// Build the CFG
	for(data::Vector<BasicBlock *>::Iterator iter = entries; iter; iter++)
		_cfgs.addTail(new CFG(code, *iter));
}


/**
 * Find the basic block containing the given instruction.
 * @param code	Code containing the basic block.
 * @param inst		Instruction to find the BB for.
 * @return				Found basic block. If the code is not already managed,
 * it is automatically added.
 */
BasicBlock *CFGInfo::findBB(Code *code, Inst *inst) {

	// Code processed ?
	if(!_codes.contains(code))
		addCode(code);
	
	// Look for the BB mark
	PseudoInst *pseudo;
	while(!inst->atBegin()) {
		if((pseudo = inst->toPseudo()) && pseudo->id() == BasicBlock::ID)
			return ((BasicBlock::Mark *)pseudo)->bb();
		inst = inst->next();
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
 * @param code	Code containing the instruction.
 * @param inst		Instruction to find the CFG starting with.
 */
CFG *CFGInfo::findCFG(Code *code, Inst *inst) {
	
	// Get the basic block
	BasicBlock *bb = findBB(code, inst);
	assert(bb);
	
	// Look for a CFG
	CFG *cfg = bb->get<CFG *>(CFG::ID, 0);
	if(cfg)
		return cfg;
	
	// Create the CFG
	cfg = new CFG(code, bb);
	_cfgs.add(cfg);
	return cfg;
}


/**
 * @fn const Collection<CFG *>& CFGInfo::cfgs(void) const;
 * Get the collection of CFG.
 * @return CFG collection.
 */

/**
 * @fn const Collection<Code *>& CFGInfo::codes(void) const;
 * Get the collection of codes.
 * @return Code collection.
 */


/**
 * @class BasicBlock::Mark
 * This pseudo-instruction is used for marking the start of a basic block.
 */

/**
 * @fn BasicBlock::Mark::Mark(BasicBlock *bb);
 * Constructor for the given basic block.
 * @param bb	Basic block marked by this pseudo-instruction.
 */

/**
 * @fn BasicBlock::Mark::~Mark(void);
 * Destructor: remove also the reference from the basic block.
 */

/**
 * @fn BasicBlock *BasicBlock::Mark::bb(void) const;
 * Get the basic block associated with this marker.
 * @return	Basic block.
 */


/**
 * @class CFG
 * Control Flow Graph representation. Its entry basic block is given and
 * the graph is built using following taken and not-taken properties of the block.
 */

/**
 * Identifier used for storing and retrieving the CFG on its entry BB.
 */
id_t CFG::ID = Property::getID("otawa.CFG");

/**
 * Constructor. Add a property to the basic block for quick retrieval of
 * the matching CFG.
 */
CFG::CFG(Code *code, BasicBlock *entry): ent(entry), _code(code) {
	ent->set<CFG *>(ID, this);
}

/**
 * @fn BasicBlock *CFG::entry(void) const;
 * Get the entry basic block of the CFG.
 * @return Entry basic block.
 */

/**
 * @fn Code *CFG::code(void) const;
 * Get the code containing the CFG.
 * @return Container code.
 */


} // namespace otawa
