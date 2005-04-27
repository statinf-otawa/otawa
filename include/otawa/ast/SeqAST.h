/*
 *	$Id$
 *	Copyright (c) 2003, Institut de Recherche en Informatique de Toulouse.
 *
 *	otaw/ast/SeqAST.h -- interface for SeqAST class.
 */
#ifndef OTAWA_AST_SEQ_AST_H
#define OTAWA_AST_SEQ_AST_H

#include <otawa/ast/AST.h>

namespace otawa {

// SeqAST class
class SeqAST: public AST {
	AST *c1, *c2;
public:
	SeqAST(AST *child1, AST *child2);
	inline AST *child1(void) const { return c1; };
	inline AST *child2(void) const { return c2; };
	
	// AST overload
	virtual Inst *first(void);
	virtual ast_kind_t kind(void) const { return AST_Seq; };
	virtual SeqAST *toSeq(void) { return this; };
};
	
} // otawa

#endif	// OTAWA_AST_SEQ_AST_H
