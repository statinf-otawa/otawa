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
	AutoPtr<AST> cond, tpart, epart;
public:
	IfAST(AutoPtr<AST> condition, AutoPtr<AST> then_part);
	IfAST(AutoPtr<AST> condition, AutoPtr<AST> then_part, AutoPtr<AST> else_part);
	inline AutoPtr<AST> condition(void) const { return cond; };
	inline AutoPtr<AST> thenPart(void) const { return tpart; };
	inline AutoPtr<AST> elsePart(void) const { return epart; };
	
	// AST overload
	virtual Inst *first(void);
	virtual ast_kind_t kind(void) const { return AST_If; };
	virtual AutoPtr<IfAST> toIf(void) { return this; };
};
	
} // otawa

#endif // OTAWA_AST_IF_AST_H
