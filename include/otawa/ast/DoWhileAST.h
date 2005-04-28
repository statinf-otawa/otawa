/*
 *	$Id$
 *	Copyright (c) 2003, Institut de Recherche en Informatique de Toulouse.
 *
 *	otaw/ast/DoWhileAST.h -- interface for DoWhileAST class.
 */
#ifndef OTAWA_AST_DO_WHILE_AST_H
#define OTAWA_AST_DO_WHILE_AST_H

#include <otawa/ast/AST.h>

namespace otawa {

// DoWhileAST class
class DoWhileAST: public AST {
	AST *bod, *cnd;
public:
	virtual ~DoWhileAST(void);
public:
	DoWhileAST(AST *body, AST *condition);
	inline AST *condition(void) const { return cnd; };
	inline AST *body(void) const { return bod; };
	
	// AST overload
	virtual Inst *first(void);
	virtual ast_kind_t kind(void) const { return AST_DoWhile; };
	virtual DoWhileAST *toDoWhile(void) { return this; };
	virtual int countInstructions(void) const;
};
	
} // otawa

#endif // OTAWA_AST_DO_WHILE_AST_H
