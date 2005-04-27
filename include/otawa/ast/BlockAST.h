/*
 *	$Id$
 *	Copyright (c) 2003, Institut de Recherche en Informatique de Toulouse.
 *
 *	otaw/ast/BlockAST.h -- interface for BlockAST class.
 */
#ifndef OTAWA_AST_BLOCK_AST_H
#define OTAWA_AST_BLOCK_AST_H

#include <otawa/ast/AST.h>
#include <otawa/instruction.h>

namespace otawa {

// BlockAST class
class BlockAST: public AST {
	Inst *blk;
protected:
	virtual ~BlockAST(void);
public:
	static const id_t ID;
	BlockAST(Inst *block);
	inline Inst *block(void) const { return blk; };

	// AST overload
	virtual Inst *first(void) { return blk; };
	virtual ast_kind_t kind(void) const { return AST_Block; };
	virtual BlockAST *toBlock(void) { return this; };
	
	
};
	
} // otawa

#endif // OTAWA_AST_BLOCK_AST_H
