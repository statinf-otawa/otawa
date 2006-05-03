/*
 *	$Id$
 *	Copyright (c) 2003, Institut de Recherche en Informatique de Toulouse.
 *
 *	otaw/ast/AST.h -- interface for AST class.
 */
#ifndef OTAWA_AST_AST_H
#define OTAWA_AST_AST_H

#include <otawa/instruction.h>
#include <otawa/program.h>

namespace otawa {

// Defined AST
class BlockAST;
class CallAST;
class SeqAST;
class IfAST;
class WhileAST;
class DoWhileAST;
class ForAST;
	
// AST Kind
typedef enum ast_kind_t {
	AST_Undef,
	AST_Nop,
	AST_Block,
	AST_Call,
	AST_Seq,
	AST_If,
	AST_While,
	AST_DoWhile,
	AST_For
} ast_kind_t;

// AST Class
class AST: public elm::Lock, public ProgObject {
	friend class FunAST;
protected:
	virtual ~AST(void) { };
public:
	static AST& NOP;
	static AST& UNDEF;
	virtual ast_kind_t kind(void) const = 0;
	virtual Inst *first(void) = 0;
	virtual void release(void);
	
	virtual bool isNOP(void) { return false; };
	virtual bool isUndef(void) { return false; };
	virtual BlockAST *toBlock(void) { return 0; };
	virtual CallAST *toCall(void) { return 0; };
	virtual SeqAST *toSeq(void) { return 0; };
	virtual IfAST *toIf(void) { return 0; };
	virtual WhileAST *toWhile(void) { return 0; };
	virtual DoWhileAST *toDoWhile(void) { return 0; };
	virtual ForAST *toFor(void) { return 0; };
	virtual int countInstructions(void) const = 0;
};


	
}	// otawa

#endif	// OTAWA_AST_AST_H
