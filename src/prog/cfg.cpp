/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	cfg.cpp -- control flow graph classes implementation.
 */

#include <assert.h>
#include <elm/debug.h>
#include <elm/datastruct/Collection.h>
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
 * @param inst	First instruction of the basic block. The basic block
 * lay from this instruction to the next basic block head or end of code.
 */
BasicBlock::BasicBlock(Inst *inst): flags(0) {
	assert(inst && (inst->atEnd() || !inst->isPseudo()));
	
	// Create the mark
	_head = new Mark(this);
	assert(inst->previous()->atBegin() || !inst->previous()->isPseudo());
	inst->insertBefore(_head);
	
	// Look for a label
	Option<String> label = inst->get<String>(File::ID_Label);
	if(label)
		set<String>(File::ID_Label, *label);
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
id_t CFGInfo::ID = Property::getID("otawa.CFGInfo");

/**
 * Identifier for marking basic blocks entries of a CFG.
 */
id_t CFGInfo::ID_Entry = Property::getID("otawa.CFGEntry");

/**
 * Remove all CFG stored in this CFG information.
 */
void CFGInfo::clear(void) {
	
	/* Remove CFGs */
	while(!_cfgs.isEmpty())
		delete _cfgs[0];
		_cfgs.removeAt(0);
	
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
			BasicBlock *bb = new BasicBlock(node);
			bb->set<bool>(ID_Entry, true);
			assert(bb);
			return bb;
		}
		
		// Is the BB pseudo?
		if(pseudo->id() == BasicBlock::ID) {
			return ((BasicBlock::Mark *)pseudo)->bb();
		}
	}
	
	// End-of-code
	BasicBlock *bb = new BasicBlock(inst->next());
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
	if(pseudo && pseudo->id() == BasicBlock::ID)
		return ((BasicBlock::Mark *)pseudo)->bb();
	
	// Look backward
	for(Inst *node = inst->previous(); !node->atBegin(); node = node->previous()) {
		pseudo = node->toPseudo();
		
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
	PseudoInst *pseudo;
	
	// Add the initial basic block
	_codes.add(code);
	BasicBlock *bb = new BasicBlock(code->first());
	bb->set<bool>(ID_Entry, true);
	
	// Find the basic blocks
	for(Inst *inst = code->first(); !inst->atEnd(); inst = inst->next())
		if(inst->isControl()) {
			
			// Found BB starting on target instruction			
			Inst *target = inst->target();
			if(target) {
				BasicBlock *bb = thisBB(target);
				assert(bb);
				if(!inst->isCall())
					bb->use<bool>(ID_Entry) = false;
			}

			// Found BB starting on next instruction
			BasicBlock *bb = nextBB(inst);
			assert(bb);
			if(inst->isCall() || inst->isConditional())
				bb->use<bool>(ID_Entry) = false;
		}
	
	// Build the graph
	genstruct::Vector<BasicBlock *> entries;
	bb = 0;
	bool follow = true;
	for(Inst *inst = code->first(); !inst->atEnd(); inst = inst->next()) {
		
		// Start of block found
		if((pseudo = inst->toPseudo()) && pseudo->id() == BasicBlock::ID) {
			
			// Record not-taken edge
			BasicBlock *next_bb = ((BasicBlock::Mark *)pseudo)->bb();
			if(bb && follow)
				bb->setNotTaken(next_bb);
			
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
				bb->setTaken(target_bb);
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
	assert(code && entry);
	ent->set<CFG *>(ID, this);
	Option<String> label = entry->get<String>(File::ID_Label);
	if(label)
		set<String>(File::ID_Label, *label);
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
