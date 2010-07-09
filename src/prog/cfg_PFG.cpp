/*
 *	$Id$
 *	PFG class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2008, IRIT UPS.
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
 *	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 *	02110-1301  USA
 */

#include <otawa/cfg/features.h>
#include <otawa/cfg/PFG.h>
#include <otawa/cfg/PFGBuilder.h>

namespace otawa { namespace pfg {

/**
 * @class BB
 * A basic block in a @ref otawa::pfg::PFG .
 */


/**
 * @fn Address BB::address(void) const;
 * Get the start address of the basic block.
 * @return	Basic block address.
 */


/**
 * @fn Address BB::topAddress(void) const;
 * Get the address next after the basic block, that is, address + size.
 * @return	Basic block next after address.
 */


/**
 * @fn size_t BB::size(void) const;
 * Get the size of the basic block.
 * @return	Basic block size in bytes.
 */


/**
 * @fn Inst *BB::first(void) const;
 * Get the first instruction of the basic block.
 * @return	First instruction.
 */


/**
 * @fn int BB::count(void) const;
 * Count the number of instructions in the basic block.
 * @return	Number of instruction.
 */


/**
 * Test if the basic block is ended by a control instruction.
 * @return	True if it contains a control, false else.
 */
bool BB::isControl(void) const {
	for(PFG::OutIterator edge(this); edge; edge++)
		if(edge->kind() != Edge::SEQ)
			return true;
	return false;
}


/**
 * Test if the basic block is ended by a conditional branch.
 * @return	True if it is ended by a conditional branch, false else.
 */
bool BB::isConditional(void) const {
	for(PFG::OutIterator edge(this); edge; edge++)
		if(edge->kind() == Edge::COND_BRANCH
		|| edge->kind() == Edge::COND_CALL
		|| edge->kind() == Edge::COND_RETURN)
			return true;
	return false;
}

/**
 * Test if the basic block is ended by a simple branch instruction (neither a call, nor a return).
 * @return	True if it ends with a branch instructions, false else.
 */
bool BB::isBranch(void) const {
	for(PFG::OutIterator edge(this); edge; edge++)
		if(edge->kind() == Edge::BRANCH
		|| edge->kind() == Edge::COND_BRANCH)
			return true;
	return false;
}


/**
 * Test if the basic block ends with a call instruction.
 * @return	True if it ends with a call instruction, false else.
 */
bool BB::isCall(void) const {
	for(PFG::OutIterator edge(this); edge; edge++)
		if(edge->kind() == Edge::CALL
		|| edge->kind() == Edge::COND_CALL)
			return true;
	return false;
}


/**
 * Test if the basic block ends with a return instruction.
 * @return	True if it ends with a return instruction, false else.
 */
bool BB::isReturn(void) const {
	for(PFG::OutIterator edge(this); edge; edge++)
		if(edge->kind() == Edge::RETURN
		|| edge->kind() == Edge::COND_RETURN)
			return true;
	return false;
}


/**
 * Test if the basic block ends with a multi-target branch instruction (either branch, or call).
 * @return	True if it ends with a multi-target branch instruction, false else.
 */
bool BB::isMultiTarget(void) const {
	int cnt = 0;
	for(PFG::OutIterator edge(this); edge; edge++)
		if(edge->kind() == Edge::BRANCH
		|| edge->kind() == Edge::COND_BRANCH
		|| edge->kind() == Edge::CALL
		|| edge->kind() == Edge::COND_CALL)
			cnt++;
	return cnt > 1;
}


/**
 * @class BB::InstIter
 * Iterator on the instructions of a basic block.
 */


/**
 * @fn BB:BB::InstIter(const BB *bb);
 * Build the iterator on the given BB.
 * @param bb	Basic block to iterate instructions on.
 */


/**
 * @fn BB::BB::InstIter(const InstIter& iter);
 * Construction by cloning an existing iterator.
 * @param iter	Iterator to clone.
 */


/**
 * @class Edge
 * This class represents the control transfer between basic block (@ref BB).
 * An edge is defined by its source basic block, its sink basic block and its type:
 * @li @ref Edge::NONE -- null edge kind (must not be found in @ref PFG),
 * @li @ref Edge::SEQ -- sequential control transfer between two adjacent basic blocks,
 * @li @ref Edge::BRANCH -- transfer by a taken branch instruction,
 * @li @ref Edge::COND_BRANCH -- transfer by a taken conditional branch instruction,
 * @li @ref Edge::CALL -- transfer by a taken sub-program call instruction,
 * @li @ref Edge::COND_CALL -- transfer by a taken conditional sub-program call
 * 		instruction,
 * @li @ref Edge::RETURN -- transfer a taken sub-program return instruction
 * 		(notice that the sink node is ever @ref PFG::ret).
 * @li @ref Edge::COND_RETURN -- transfer a taken conditional sub-program return
 * 		instruction (notice that the sink node is ever @ref PFG::ret)..
 * 
 * @par
 * 
 * Some rules applies on the outing edges of a basic block:
 * @li It must be at most one @ref Edge::SEQ,  @ref Edge::RETURN
 * 		or @ref Edge::COND_RETURN.
 * @li The basic block must not exit in the same time with @ref Edge::BRANCH,
 * 		@ref Edge::CALL or @ref Edge::RETURN.
 * @li If a block has a @ref Edge::COND_BRANCH, @ref Edge::COND_CALL,
 * 		@ref Edge::CALL or @ref Edge::COND_RETURN, an edge @ref Edge::SEQ
 * 		must also be available.
 * 
 * @par
 * Notice also that basic block that branch to unknown address have an edge with
 * @ref PFG::unknown as sink basic block.
 */


/**
 * @typedef enum kind_t Edge::kind_t;
 * Type of edges. Look at @ref Edge for more details.
 */


/**
 * @fn kind_t Edge::kind(void) const;
 * Get the kind of an edge.
 * @return	Kind of edge.
 */


/**
 * @class PFG
 * A PFG (Program Flow Graph) is built from the binary form of the program to represent
 * all possible pathes of the program from its entry point. Building a PFG is the first
 * step to understand the control flow of the program. Usually, it helps to
 * analyze the sub-program and processed CFG (Control Flow Graphs) are extracted from it.
 * @ingroup cfg
 */


/**
 * @var BB PFG::ret;
 * This special basic block is used as the sink vertex of edges representing sub-program return.
 */


/**
 * @var BB PFG::unknown;
 * This special basic block is used as the sink vertex of edges representing branch or call
 * to an unknown target.
 */


/**
 * PFG constructor.
 */
PFG::PFG(void) {
	graph_t::add(&ret);
	graph_t::add(&unknown);
}


/**
 * Add a basic block to a PFG.
 * @param bb	Basic block to add.
 * @warning		Linking with edges basic block from different PFG results in an error.
 */
void PFG::add(BB *bb) {
	graph_t::add(bb);
	bbs.put(bb->address(), bb);
}


/**
 * Get a basic block from its address.
 * @param addr	Address of start of the the basic block to get.
 * @return		Found basic block at the starting address, null else.
 */
BB *PFG::get(Address addr) {
	return bbs.get(addr, 0);
}


static SilentFeature::Maker<PFGBuilder> maker;

}	// pfg

/**
 * Feature providing the PFG of the application.
 * <b>default processor</b>: @ref PFGBuilder
 * 
 * @par Provided properties
 * @li @ref PFG -- PFG of the application.
 * @li @ref PFG_BB -- link of a pfg::BB on the first instruction.
 * 
 * @ingroup cfg
 */
SilentFeature PFG_FEATURE("otawa::PFG_FEATURE", pfg::maker);


/**
 * @ref PFG of the program.
 * <b>Default value</b>: null
 * 
 * @par Hooks
 * @li @ref WorkSpace
 * 
 * @par Provider
 * @li @ref PFG_FEATURE
 * 
 * @ingroup cfg
 */
Identifier<pfg::PFG *> PFG("otawa::PFG", 0);


/**
 * Link of @ref pfg::BB on an in instruction.
 * 
 * @par Hooks
 * @li @ref otawa::Inst
 * 
 * @ingroup cfg
 */
Identifier<pfg::BB *> PFG_BB("otawa::PFG_BB", 0);

} // otawa
