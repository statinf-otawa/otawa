/*
 *	$Id$
 *	Copyright (c) 2003, Institut de Recherche en Informatique de Toulouse.
 *
 *	otaw/ast/IfAST.h -- interface for IfAST class.
 */
#ifndef OTAWA_AST_IF_AST_H
#define OTAWA_AST_IF_AST_H

#include <otawa/ast/AST.h>

namespace otawa {

// IfAST class
class IfAST: public AST {
	AST *cond, *tpart, *epart;
protected:
	virtual ~IfAST(void);
public:
	IfAST(AST *condition, AST *then_part);
	IfAST(AST *condition, AST *then_part, AST *else_part);
	inline AST *condition(void) const { return cond; };
	inline AST *thenPart(void) const { return tpart; };
	inline AST *elsePart(void) const { return epart; };
	
	// AST overload
	virtual Inst *first(void);
	virtual ast_kind_t kind(void) const { return AST_If; };
	virtual IfAST *toIf(void) { return this; };
};
	
} // otawa

#endif // OTAWA_AST_IF_AST_H
