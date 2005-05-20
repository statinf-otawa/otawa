/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	src/prog/BasicBlock.cpp -- implementation of BasicBlock class.
 */

#include <otawa/cfg.h>
#include <otawa/prog/FrameWork.h>

namespace otawa {

// Null basic block
class NullBasicBlock: public BasicBlock {
	inhstruct::DLList list;
	Mark null_mark;
public:
	inline NullBasicBlock(void): null_mark(this) {
		list.addFirst(&null_mark);		
		_head = &null_mark;
	};
};
static NullBasicBlock null_bb_inst;


/**
 */
BasicBlock& BasicBlock::null_bb = null_bb_inst;


/**
 * @class BasicBlock
 * This is the minimal definition of a basic block.
 */


/**
 * Identifier of the basic block pseudo-instruction.
 */
id_t BasicBlock::ID = Property::getID("otawa.bb");


/**
 * @fn Mark *BasicBlock::head(void) const;
 * Get the mark pseudo-instruction of the basic block. Following instruction
 * until the next mark are the content of the basic block.
 * @return	Basic block mark.
 */


/**
 * Get the target basic block if the last branch instruction is taken.
 * @return Target basic block or null if the last instruction is not a branch.
 * @deprecated{Use @ref Edge class instead.}
 */
BasicBlock *BasicBlock::getTaken(void) {
	for(elm::Iterator<Edge *> edge(outEdges()); edge; edge++)
		if(edge->kind() == EDGE_Taken || edge->kind() == EDGE_Call)
			return edge->target();
	return 0;
}


/**
 * Get the following basic block if the last branch instruction is not taken.
 * @return Following basic block or null if the last instruction is a sub-program return.
 * @deprecated{Use @ref Edge class instead.}
 */
BasicBlock *BasicBlock::getNotTaken(void) {
	for(elm::Iterator<Edge *> edge(outEdges()); edge; edge++)
		if(edge->kind() == EDGE_NotTaken)
			return edge->target();
	return 0;
}


/**
 * Set the target basic block of the branch last instruction.
 * @param bb	New target basic block.
 * @deprecated{Use @ref Edge class instead.}
 */
void BasicBlock::setTaken(BasicBlock *bb) {
	assert(bb);
	new Edge(this, bb, EDGE_Taken);
}



/**
 * Set the following basic block.
 * @param bb	New following basic block.
 * @deprecated{Use @ref Edge class instead.}
 */
void BasicBlock::setNotTaken(BasicBlock *bb) {
	assert(bb);
	new Edge(this, bb, EDGE_NotTaken);
}


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
 * Compute the size of the basic block.
 * @return Size of basic block.
 */
size_t BasicBlock::size(void) const {
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
 * @fn size_t BasicBlock::getBlockSize(void) const;
 * Compute the size of the basic block.
 * @return Size of basic block.
 * @deprecated See @ref BasicBlock::size().
 */


/**
 * Count the number of instructions in the basic block.
 * @return	Number of instruction in the basic block.
 */
int BasicBlock::countInstructions(void) const {
	assert(_head);
	Inst *inst;
	int cnt = 0;
	PseudoInst *pseudo;

	for(inst = _head->next(); !inst->atEnd(); inst = inst->next()) {
		pseudo = inst->toPseudo();
		if(!pseudo)
			cnt++;
		else if(pseudo->id() == ID)
			break;
	}
	return cnt;
}


/**
 */
BasicBlock::~BasicBlock(void) {
}


/**
 * @fn void BasicBlock::addInEdge(Edge *edge);
 * Add an input edge to the basic block input list.
 * @param edge	Edge to add.
 */


/**
 * @fn void BasicBlock::addOutEdge(Edge *edge);
 * Add an output edge to the basic block output list.
 * @param edge	Edge to add.
 */


/**
 * @fn void BasicBlock::removeInEdge(Edge *edge);
 * Remove an input edge from the basic block input list.
 * @param edge	Edge to remove.
 */


/**
 * @fn void BasicBlock::removeOutEdge(Edge *edge);
 * Remove an output edge from the basic block output list.
 * @param edge	Edge to remove.
 */


/**
 * @fn IteratorInst<Edge *> *BasicBlock::inEdges(void);
 * Get an iterator on the input edges.
 * @return	Iterator on output edges.
 */


/**
 * @fn IteratorInst<Edge *> *BasicBlock::outEdges(void);
 * Get an iterator on the output edges.
 * @return	Iterator on output edges.
 */


/**
 * @fn IteratorInst<Inst *> *BasicBlock::visit(void);
 * Get an iterator on the instructions of the baic block.
 * @return	Iterator on instructions.
 */


/**
 * @fn BasicBlock::operator IteratorInst<Inst *> *(void);
 * Same as @ref visit() but allows passing basic block in @ref Iterator class.
 */


/**
 * Find the basic block at the given address if it exists.
 * @param fw	Framework to look in.
 * @param addr	Address of basic block or null if it cannot be found.
 */
BasicBlock *BasicBlock::findBBAt(FrameWork *fw, address_t addr) {
	Inst *inst = fw->findInstAt(addr);
	PseudoInst *pseudo;
	while(!inst->atBegin()
	&& (!(pseudo = inst->toPseudo())
		|| pseudo->id() != CodeBasicBlock::ID))
		inst = inst->previous();
	if(inst->atBegin())
		return 0;
	else
		return ((CodeBasicBlock::Mark *)pseudo)->bb();
}


/*
 * Iterator for edges
 */
bool BasicBlock::EdgeIterator::ended(void) const {
	return iter.ended();
}

Edge *BasicBlock::EdgeIterator::item(void) const {
	return iter.item();
}

void BasicBlock::EdgeIterator::next(void) {
	iter.next();
}


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
 * @class BasicBlock::Iterator
 * Iterator for instructions in the basic block.
 */


/**
 */
bool BasicBlock::Iterator::ended(void) const {
	PseudoInst *pseudo;
	return inst->atEnd()
		|| ((pseudo = inst->toPseudo()) && pseudo->id() == ID);
}


/**
 */
Inst *CodeBasicBlock::Iterator::item(void) const {
	return inst;
}


/**
 */
void CodeBasicBlock::Iterator::next(void) {
	inst = inst->next();
}


/**
 * @class CodeBasicBlock
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
CodeBasicBlock::CodeBasicBlock(Inst *inst) {
	assert(inst);
	assert(inst->atEnd() || !inst->isPseudo());
	
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
	
	// Set flags
	flags = 0;
}


/**
 * Delete the basic block.
 */
CodeBasicBlock::~CodeBasicBlock(void) {
	if(_head)
		delete _head;
};

} // otawa
