/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	src/prog/BasicBlock.cpp -- implementation of BasicBlock class.
 */

#include <otawa/cfg/BasicBlock.h>

namespace otawa {

/*operator BasicBlock::Iterator::IteratorInst<Inst *> *(void);
{ return new Iterator(this); };*/

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
	if(!inst->atEnd()) {
		Option<String> label = inst->get<String>(File::ID_Label);
		if(label)
			set<String>(File::ID_Label, *label);
	}
}


/**
 * @fn elm::AutoPtr<BasicBlock> BasicBlock::getTaken(void) const;
 * Get the target basic block if the last branch instruction is taken.
 * @return Target basic block or null if the last instruction is not a branch.
 */


/**
 * @fn elm::AutoPtr<BasicBlock> BasicBlock::getNotTaken(void) const;
 * Get the following basic block if the last branch instruction is not taken.
 * @return Following basic block or null if the last instruction is a sub-program return.
 */


/**
 * @fn void BasicBlock::setTaken(elm::AutoPtr<BasicBlock> bb);
 * Set the target basic block of the branch last instruction.
 * @param bb	New target basic block.
 */


/**
 * @fn void BasicBlock::setNotTaken(elm::AutoPtr<BasicBlock> bb);	
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
 *  @fn bool BasicBlock::isCall(void) const;
 *  Test if the basuc block is ended by a call to a sub-program.
 *  @return True if the basic block is call, false else.
 */


/**
 *  @fn bool BasicBlock::isReturn(void) const;
 *  Test if the basic block is ended by a sub-program return.
 *  @return True if it is a sub-program return, false else.
 */


/**
 * @fn bool BasicBlock::isTargetUnknown(void) const;
 * Test if the target of the branch instruction of this basic block is known or not. Unknown branch
 * target is usually to a non-constant branch address. Remark that sub-program return is not viewed
 * as an unknown branch target.
 * @return True if the branch target is unknown, false else.
 */


/**
 *	@fn address_t BasicBlock::address(void) const;
 *	Get the address of the first instruction of the basic block.
 *	@return	First instruction address.
 */


/**
 * @class BasicBlock::Mark
 * This pseudo-instruction is used for marking the start of a basic block.
 */

/**
 * @fn BasicBlock::Mark::Mark(AutoPtr<BasicBlock> bb);
 * Constructor for the given basic block.
 * @param bb	Basic block marked by this pseudo-instruction.
 */

/**
 * @fn BasicBlock::Mark::~Mark(void);
 * Destructor: remove also the reference from the basic block.
 */


/**
 * @fn AutoPtr<BasicBlock> BasicBlock::Mark::bb(void) const;
 * Get the basic block associated with this marker.
 * @return	Basic block.
 */


/**
 * Release the basic blocks links whatever the netring links.
 * This method is usually called by CFGInfo for removing all CFG.
 */
void BasicBlock::release(void) {
	
	// Remove mark
	_head->remove();
	delete _head;
	
	// Remove output edges
	tkn = 0;
	ntkn = 0;
}


/**
 * Compute the size of the basic block.
 * @return Size of basic block.
 */
size_t BasicBlock::getBlockSize(void) const {
	assert(_head);
	Inst *inst;
	PseudoInst *pseudo;
	
	// Find the next BB marker
	for(inst = _head->next(); !inst->atEnd(); inst = inst->next())
		if((pseudo = inst->toPseudo()) && pseudo->id() == ID)
			return inst->address() - address();

	// Else this is the last block
	return 0;
}


/**
 * @class BasicBlock::Iterator
 * Iterator for instructions in the basic block.
 */


// IteratorInst<T> overload
bool BasicBlock::Iterator::ended(void) const {
	PseudoInst *pseudo;
	return inst->atEnd()
		|| ((pseudo = inst->toPseudo()) && pseudo->id() == ID);
}


// IteratorInst<T> overload			
Inst *BasicBlock::Iterator::item(void) const {
	return inst;
}


// IteratorInst<T> overload
void BasicBlock::Iterator::next(void) {
	inst = inst->next();
}

} // otawa
