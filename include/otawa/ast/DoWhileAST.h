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
	AutoPtr<AST> bod, cnd;
public:
	DoWhileAST(AutoPtr<AST> body, AutoPtr<AST> condition);
	inline AutoPtr<AST> condition(void) const { return cnd; };
	inline AutoPtr<AST> body(void) const { return bod; };
	
	// AST overload
	virtual Inst *first(void);
	virtual ast_kind_t kind(void) const { return AST_DoWhile; };
	virtual AutoPtr<DoWhileAST> toDoWhile(void) { return this; };

};
	
} // otawa

#endif // OTAWA_AST_DO_WHILE_AST_H
