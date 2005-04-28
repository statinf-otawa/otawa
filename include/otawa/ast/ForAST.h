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
	AST *bod, *cnd, *init, *inc;
protected:
	virtual ~ForAST(void);
public:
	ForAST(AST *initialization, AST *condition,
		AST *incrementation, AST *body);
	inline AST *initialization(void) const { return init; };
	inline AST *condition(void) const { return cnd; };
	inline AST *incrementation(void) const { return inc; };
	inline AST *body(void) const { return bod; };
	
	// AST overload
	virtual Inst *first(void);
	virtual ast_kind_t kind(void) const { return AST_For; };
	virtual ForAST *toFor(void) { return this; };
	virtual int countInstructions(void) const;
};
	
} // otawa

#endif // OTAWA_AST_FOR_H
