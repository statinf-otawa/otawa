/*
 *	$Id$
 *	BasicBlock class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2003-08, IRIT UPS.
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

#include <otawa/cfg.h>
#include <otawa/prog/WorkSpace.h>

namespace otawa {

// Null basic block
class NullBasicBlock: public BasicBlock {
};
static NullBasicBlock null_bb_inst;


/**
 */
BasicBlock& BasicBlock::null_bb = null_bb_inst;


/**
 * @class BasicBlock
 * This is the minimal definition of a basic block.
 *
 * @par Implemented concepts
 * @li @ref otawa::concept::InstBlock
 */


/**
 * Identifier of the basic block pseudo-instruction.
 */
Identifier<BasicBlock *> BasicBlock::ID("ptawa::BasicBlock::ID", 0);


/**
 * Build an empty basic block.
 */
BasicBlock::BasicBlock(void): first(&Inst::null), _size(0), flags(0), _cfg(0) {
}


/**
 * Get the target basic block if the last branch instruction is taken.
 * @return Target basic block or null if the last instruction is not a branch.
 * @deprecated{Use @ref Edge class instead.}
 */
BasicBlock *BasicBlock::getTaken(void) {
	for(BasicBlock::OutIterator edge(this); edge; edge++)
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
	for(BasicBlock::OutIterator edge(this); edge; edge++)
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
	ASSERT(bb);
	new Edge(this, bb, EDGE_Taken);
}



/**
 * Set the following basic block.
 * @param bb	New following basic block.
 * @deprecated{Use @ref Edge class instead.}
 */
void BasicBlock::setNotTaken(BasicBlock *bb) {
	ASSERT(bb);
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
 * @fn bool BasicBlock::isEntry(void) const;
 * Test if the basic block is the CFG entry.
 * @return True if the current BB is the CFG entry.
 */


/**
 * @fn bool BasicBlock::isExit(void) const;
 * Test if the basic block is the CFG exit.
 * @return True if the current BB is the CFG exit.
 */


/**
 * @fn bool BasicBlock::isEnd(void) const;
 * Test if the basic block is the CFG end, entry or exit.
 * @return True if the current BB is the CFG end.
 */


/**
 * @fn size_t BasicBlock::size(void) const;
 * Get the size of the basic block.
 * @return Size of basic block.
 */


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
int BasicBlock::countInsts(void) const {
	int cnt = 0;
	for(InstIter inst(this); inst; inst++)
		cnt++;
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
 * Get an iterator on the entering edges.
 * @return	Entering edge iterator.
 * @deprecated	Use InIterator instead.
 */
/*IteratorInst<Edge *> *BasicBlock::inEdges(void) {
	InIterator iter(this);
	return new IteratorObject<InIterator, Edge *>(iter);
}*/


/**
 * Get an iterator on the leaving edges.
 * @return	Leaving edge iterator.
 * @deprecated	Use OutIterator instead.
 */
/*IteratorInst<Edge *> *BasicBlock::outEdges(void) {
	OutIterator iter(this);
	return new IteratorObject<OutIterator, Edge *>(iter);
}*/


/**
 * @fn int BasicBlock::number(void);
 * Get the number hooked on this basic block, that is, value of ID_Index
 * property.
 * @return Number of the basic block or -1 if there is no number hooked.
 */


/**
 * Print the reference for a basic block.
 * @param	out		Output stream.
 */
void BasicBlock::print(io::Output& out) const {
	if(isEntry())
		out << "ENTRY";
	else if(isExit())
		out << "EXIT";
	else
		out << "BB " << number() << " (" << address() << ")";
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
 * @fn bool BasicBlock::isConditional(void) const;
 * Test if the BB is ended by a conditional branch.
 * @return	True if the BB is ended by a conditional branch, false else.
 */


/**
 * @class BasicBlock::InstIter
 * Iterator for instructions in the basic block.
 */


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
  CodeBasicBlock::CodeBasicBlock(Inst *inst, t::size size) {
	ASSERT(inst);

	// initialize
	set(inst, size);
	flags = 0;

	// copy label if any
	string label = LABEL(inst);
	if(label)
		LABEL(this) = label;
}


/**
 * Get the last instruction of a basic block.
 * @return		Last instruction (or NULL if the basic block is an end).
 */
Inst *BasicBlock::lastInst(void) const {
	Inst *last = 0;
	for(BasicBlock::InstIter inst(this); inst; inst++)
		last = inst;
	return last;
}


/**
 * Find the last control instruction of the basic block.
 * This is usually the last one but (1) delayed-branch architecture insert instructions after the control
 * and (2) OTAWA CFG configuration system may make merge different blocks or ignore some control instructions.
 * If no control is found, return the last instruction.
 * @return	Last control instruction.
 */
Inst *BasicBlock::controlInst(void) const {
	Inst *control = 0, *last = 0;
	for(BasicBlock::InstIter inst(this); inst; inst++) {
		last = inst;
		if(inst->isControl())
			control = inst;
	}
	if(!control)
		control = last;
	return control;
}

} // otawa
