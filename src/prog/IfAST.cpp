/*
 *	$Id$
 *	Copyright (c) 2003, Institut de Recherche en Informatique de Toulouse.
 *
 *	src/prog/IfAST.cpp -- implementation for IfAST class.
 */

#include <otawa/ast/IfAST.h>

namespace otawa {

/**
 * @class IfAST
 * AST for representing selections.
 */

/**
 * Build a selection without else part.
 * @param condition	Selection condition.
 * @param then_part	Then part.
 */
IfAST::IfAST(AST *condition, AST *then_part)
: cond(condition), tpart(then_part), epart(&NOP) {
	ASSERT(condition && then_part);
}


/**
 * Full selection building.
 * @param condition	Selection condition.
 * @param then_part	Then part.
 * @param else_part	Else part.
 */
IfAST::IfAST(AST *condition, AST *then_part,
AST *else_part) : cond(condition), tpart(then_part), epart(else_part) {
	ASSERT(condition && then_part && else_part);
}


/**
 */
IfAST::~IfAST(void) {
	cond->release();
	tpart->release();
	epart->release();
}


/**
 * @fn AST *IfAST::condition(void) const;
 * Get the condition of the selection.
 * @return Selection condition.
 */


/**
 * @fn AST *IfAST::thenPart(void) const;
 * Get the "then" part of the selection.
 * @return "then" part.
 */


/**
 * @fn AST *IfAST::elsePart(void) const;
 * Get the "else" part of the selection.
 * @return "else" part.
 */


/**
 */
Inst *IfAST::first(void) {
	return cond->first();
}


/**
 */
int IfAST::countInstructions(void) const {
	return	cond->countInstructions()
		+	tpart->countInstructions()
		+	epart->countInstructions();
}

} // otawa
