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
	AutoPtr<AST> cnd, bod;
public:
	WhileAST(AutoPtr<AST> condition, AutoPtr<AST> body);
	inline AutoPtr<AST> condition(void) const { return cnd; };
	inline AutoPtr<AST> body(void) const { return bod; };
	
	// AST overload
	virtual Inst *first(void);
	virtual ast_kind_t kind(void) const { return AST_While; };
	virtual AutoPtr<WhileAST> toWhile(void) { return this; };
};
	
} // otawa

#endif // OTAWA_AST_WHILE_AST_H
