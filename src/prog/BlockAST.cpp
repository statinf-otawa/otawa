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
 * @param block Entry basic block.
 */
BlockAST::BlockAST(Inst *block): blk(block) {
	assert(block);
	blk->set<BlockAST *>(ID, this);
}


/**
 * Destroy the block AST, cleaning properties put on basic block and basic block head.
 */
BlockAST::~BlockAST(void) {
	assert(blk);
	blk->removeProp(ID);
}

/**
 * @fn Inst *BlockAST::block(void) const;
 * Get the entry basic block of the AST block.
 */

/**
 * The property matching the given identifier is put on the first instruction
 * of the AST. The property contains a pointer to the block AST.
 */
const id_t BlockAST::ID = Property::getID("otawa.ast.block");


}	// otawa
