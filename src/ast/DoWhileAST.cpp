/*
 *	DoWhileAST class implementation
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
#include <otawa/ast/DoWhileAST.h>

namespace otawa { namespace ast {

/**
 * @class DoWhileAST
 * Representation of C do{ ... } while(...); control structure.
 *
 * @ingroup ast
  */


/**
 * Build a new DO-WHILE AST.
 * @param body		Loop body.
 * @param condition	Loop condition.
 */
DoWhileAST::DoWhileAST(AST *body, AST *condition)
: bod(body), cnd(condition) {
	ASSERT(body && condition);
}


/**
 */
DoWhileAST::~DoWhileAST(void) {
	bod->release();
	cnd->release();
}


/**
 * @fn AST *DoWhileAST::condition(void) const;
 * Get the condition of the loop.
 * @return Loop condition.
 */


/**
 * @fn AST *DoWhileAST::body(void) const;
 * Get the body of the loop.
 * @return loop body.
 */


/**
 */
Inst *DoWhileAST::first(void) {
	Inst *result = bod->first();
	if(!result)
		result = cnd->first();
	return result;
}


/**
 */
int DoWhileAST::countInstructions(void) const {
	return bod->countInstructions() + cnd->countInstructions();
}

} }		// otawa::ast
