/*
 *	$Id$
 *	Copyright (c) 2003, Institut de Recherche en Informatique de Toulouse.
 *
 *	otaw/ast/WhileAST.h -- interface for WhileAST class.
 */
#ifndef OTAWA_AST_WHILE_AST_H
#define OTAWA_AST_WHILE_AST_H

#include <otawa/ast/AST.h>

namespace otawa {

// WhileAST class
class WhileAST: public AST {
	AST *cnd, *bod;
protected:
	~WhileAST(void);
public:
	WhileAST(AST *condition, AST *body);
	inline AST *condition(void) const { return cnd; };
	inline AST *body(void) const { return bod; };
	
	// AST overload
	virtual ast_kind_t kind(void) const { return AST_While; };
	virtual WhileAST *toWhile(void) { return this; };
};
	
} // otawa

#endif // OTAWA_AST_WHILE_AST_H
