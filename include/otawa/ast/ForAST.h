/*
 *	$Id$
 *	Copyright (c) 2003, Institut de Recherche en Informatique de Toulouse.
 *
 *	otaw/ast/ForAST.h -- interface for ForAST class.
 */
#ifndef OTAWA_AST_FOR_AST_H
#define OTAWA_AST_FOR_AST_H

#include <otawa/ast/AST.h>

namespace otawa {

// ForAST class
class ForAST: public AST {
	AutoPtr<AST> bod, cnd, init, inc;
public:
	ForAST(AutoPtr<AST> initialization, AutoPtr<AST> condition,
		AutoPtr<AST> incrementation, AutoPtr<AST> body);
	inline AutoPtr<AST> initialization(void) const { return init; };
	inline AutoPtr<AST> condition(void) const { return cnd; };
	inline AutoPtr<AST> incrementation(void) const { return inc; };
	inline AutoPtr<AST> body(void) const { return bod; };
	
	// AST overload
	virtual Inst *first(void);
	virtual ast_kind_t kind(void) const { return AST_For; };
	virtual AutoPtr<ForAST> toFor(void) { return this; };
};
	
} // otawa

#endif // OTAWA_AST_FOR_H
