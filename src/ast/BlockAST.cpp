/*
 *	ASTLoader class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2003, IRIT UPS.
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
#include <otawa/ast/BlockAST.h>

namespace otawa { namespace ast {

/**
 * @class BlockAST
 * This class represents the leafs of the AST. They link the AST representation
 * with the instructions. The block AST range from its entry instruction
 * to the next block AST entry (here, "next" means following the control
 * flow of the program). Each AST block entry is marked by
 * an ID_Block property.
 * @p
 * Remark that an AST block may contains branches
 * due to translation of some language structure: for example, the shortcut
 * evaluation of || and && operators in C. Remark also that a block AST may
 * have many outputs.
 *
 * @ingroup ast
 */

/**
 * Build a new block AST.
 * @param block	First instruction in the block.
 * @param size	Size of the block.
 */
BlockAST::BlockAST(Inst *block, t::uint32 size): _block(block), _size(size) {
	ASSERT(block);
	ASSERT(size >= 0);
	ID(_block) = this;
}


/**
 * Destroy the block AST, cleaning properties put on basic block and basic block head.
 */
BlockAST::~BlockAST(void) {
	//blk->removeProp(ID);
}

/**
 * @fn Inst *BlockAST::block(void) const;
 * Get the entry basic block of the AST block.
 */


/**
 * @fn size_t BlockAST::uint32(void) const;
 * Get the size of the AST block.
 */


/**
 * @fn Inst *BlockAST::first();
 * Get the first instruction of the block.
 * @return	Block first instruction.
 */


/**
 * The property matching the given identifier is put on the first instruction
 * of the AST. The property contains a pointer to the block AST.
 */
Identifier<AST *> BlockAST::ID("otawa::BlockAST::id", 0);


/**
 */
int BlockAST::countInstructions(void) const {
	address_t last = _block->address() + _size;
	int count = 0;
	for(Inst *inst = _block; inst->address() < last; inst = inst->nextInst())
		count++;
	return count;
}

} }	// otawa::ast
