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
	Inst *_block;
	t::size _size;
protected:
	virtual ~BlockAST(void);
public:
	static Identifier<AST *> ID;
	BlockAST(Inst *block, t::size size);
	inline Inst *block(void) const { return _block; };
	inline t::size size(void) const { return _size; };

	// AST overload
	virtual Inst *first(void) { return _block; };
	virtual ast_kind_t kind(void) const { return AST_Block; };
	virtual BlockAST *toBlock(void) { return this; };
	virtual int countInstructions(void) const;
};
	
} // otawa

#endif // OTAWA_AST_BLOCK_AST_H
