/*
 *	$Id$
 *	Copyright (c) 2003, Institut de Recherche en Informatique de Toulouse.
 *
 *	otaw/ast/BlockAST.h -- implementation for BlockAST class.
 */

#include <otawa/ast/BlockAST.h>

namespace otawa {

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
 */

/**
 * Build a new block AST.
 * @param block	First instruction in the block.
 * @param size	Size of the block.
 */
BlockAST::BlockAST(Inst *block, size_t size): _block(block), _size(size) {
	assert(block);
	assert(size >= 0);
	_block->set<BlockAST *>(ID, this);
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
 * @fn size_t BlockAST::size(void) const;
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
Identifier<AST *> BlockAST::ID("otawa.ast.block", 0, NS);


/**
 */
int BlockAST::countInstructions(void) const {
	address_t last = _block->address() + _size;
	int count = 0;
	for(Inst *inst = _block; inst->address() < last; inst = inst->next())
		count++;
	return count;
}

}	// otawa
