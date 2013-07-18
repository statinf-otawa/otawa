/*
 *	IfAST class implementation
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	OTAWA is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	OTAWA is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with OTAWA; if not, write to the Free Software
 *	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */
#include <otawa/ast/IfAST.h>

namespace otawa { namespace ast {

/**
 * @class IfAST
 * AST for representing selections.
 *
 * @ingroup ast
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

} }		// otawa::ast
